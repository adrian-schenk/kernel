#include "ext4.h"
#include <kmalloc.h>
#include <stddef.h>
#include <endian.h>
#include <printf.h>
#include <memory.h>

#define EXT4_SUPERBLOCK_OFFSET 1024
#define EXT4_SUPERBLOCK_SIZE 1024
#define EXT4_CHECKSUM_OFFSET 1020
#define EXT4_UUID_OFFSET 104

static uint32_t ext4_superblock_checksum(const uint8_t *sb);
static void crc32c_init(void);

static int ext4_read_file(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t inode, uint8_t *buffer, size_t size, size_t offset);

static void ext4_parse_extent_tree(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr);
static void ext4_parse_extent_tree_internal(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr);
static void ext4_parse_extent_tree_leaf(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr);

static int ext4_traverse_root(blkdev_handle_t *blkdev, ext4_superblock_t *sb, char *path);
static int ext4_traverse_dir(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t dir_inode, uint64_t parent_inode, char *path);

static ext4_inode_t *ext4_read_inode(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t inode_num);

static uint32_t ext4_bg_get_inode_tbl_block(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t bg, uint64_t inode_num);

fs_handle_t *ext4_handle_create(blkdev_handle_t *blk_dev)
{

  ext4_fs_t *fs = kmalloc(sizeof(ext4_fs_t));

  fs_handle_t *handle = kmalloc(sizeof(fs_handle_t));
  handle->fs_ctx = fs;
  handle->blk_dev = blk_dev;
  handle->fopen = ext4_open;
  handle->fcreate = ext4_create;
  handle->fremove = ext4_remove;
  handle->fclose = ext4_close;

  handle->fstat = ext4_fstat;

  handle->mount = ext4_mount;
  handle->unmount = ext4_unmount;
  handle->mkfs = ext4_mkfs;

  return handle;
}

int ext4_mount(fs_handle_t *handle)
{
  ext4_fs_t *fs = (ext4_fs_t *)handle->fs_ctx;

  uint8_t *buf = kmalloc(1024);

  handle->blk_dev->read(handle->blk_dev->ctx, 1024, 1024, buf);

  ext4_superblock_t *sb = (ext4_superblock_t *)buf;

  if (sb->s_magic != 0xEF53)
  {
    printf("Not an ext4 filesystem! Magic: %x\n", sb->s_magic);
    return -1;
  }

  if (sb->s_feature_ro_compat & 0x400)
  {
    crc32c_init();

    uint32_t stored = *(uint32_t *)((uint8_t *)sb + EXT4_CHECKSUM_OFFSET);
    uint32_t calculated = ext4_superblock_checksum((uint8_t *)sb);

    printf("Superblock checksum: stored %08x, calculated %08x\n", stored, calculated);

    if (stored != calculated)
    {
      printf("Superblock checksum mismatch!\n");
      return -1;
    }
    else
    {
      printf("Superblock checksum %08x valid.\n", calculated);

      fs->superblock = *sb;

      printf("Mounted ext4 filesystem with %d inodes and %d blocks!\n", sb->s_inodes_count, sb->s_blocks_count_lo);

      uint64_t block_size = 1024 << sb->s_log_block_size;
      uint64_t blocks_per_group = sb->s_blocks_per_group;
      uint64_t total_blocks = sb->s_blocks_count_lo;
      uint64_t group_count = (total_blocks + blocks_per_group - 1) / blocks_per_group;
      uint64_t inode_size = sb->s_inode_size;

      uint64_t group_size = group_count * sb->s_desc_size;
      printf("Block size: %d bytes\n", block_size);
      printf("Blocks per group: %d\n", blocks_per_group);
      printf("Total blocks: %d\n", total_blocks);
      printf("Group count: %d\n", group_count);
      printf("Group descriptor size: %d bytes\n", sb->s_desc_size);
      printf("Group table size: %d bytes\n", group_size);
      printf("Inodes per Group: %d\n", sb->s_inodes_per_group);
      printf("Inode size: %d bytes\n", inode_size);
    }
  }
  return 0;
}

int ext4_unmount(fs_handle_t *handle)
{
  return 0;
}
int ext4_mkfs(fs_handle_t *handle)
{
  return 0;
}

int ext4_open(fs_handle_t *handle, char *path)
{
  ext4_fs_t *fs = (ext4_fs_t *)handle->fs_ctx;

  ext4_superblock_t *sb = &fs->superblock;

  int inode = ext4_traverse_root(handle->blk_dev, sb, path);

  if (inode < 0)
    return -1;

  ext4_inode_t *inode_ptr = ext4_read_inode(handle->blk_dev, sb, inode);
  if (inode_ptr == NULL)
    return -1;

  if (inode_ptr->i_mode & 0x4000)
  {
    kfree(inode_ptr);
    return -2;
  }

  kfree(inode_ptr);
  return inode;
}

int ext4_create(fs_handle_t *handle)
{
  return 0;
}

int ext4_remove(fs_handle_t *handle)
{
  return 0;
}

int ext4_close(fs_handle_t *handle)
{
  return 0;
}

uint64_t ext4_fstat(fs_handle_t *handle, char *path, char recurse)
{
  return 0;
}

static int ext4_read_file(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t inode, uint8_t *buffer, size_t size, size_t offset)
{
  uint64_t block_size = 1024 << sb->s_log_block_size;
  uint64_t inode_size = sb->s_inode_size;

  uint64_t inode_bg = (inode - 1) / sb->s_inodes_per_group;
  uint64_t inode_index = (inode - 1) % sb->s_inodes_per_group;

  uint32_t bg_inode_table = ext4_bg_get_inode_tbl_block(blkdev, sb, inode_bg, inode);

  uint64_t inode_block = bg_inode_table + ((inode_index * inode_size) / block_size);
  uint64_t inode_offset = (inode_index * inode_size) % block_size;

  uint8_t *buf = kmalloc(block_size);
  blkdev->read(blkdev->ctx,
               inode_block * block_size,
               block_size,
               buf);

  ext4_inode_t *inode_table = (ext4_inode_t *)buf;
  ext4_inode_t *inode_ptr =
      (ext4_inode_t *)((uint8_t *)inode_table + inode_offset);

  if (inode_ptr->i_mode & 0x1000 == EXT4_DIRECTORY)
  {
    printf("Inode %d is a directory, not a file!\n", inode);
    kfree(buf);
    return -2;
  }

  ext4_extent_arr_t extents;
  extents.count = 0;
  ext4_parse_extent_tree(blkdev, sb, (ext4_extent_header_t *)&inode_ptr->i_block, &extents);

  kfree(buf);

  for (int i = 0; i < extents.count; i++)
  {
    buf = kmalloc(extents.arr[i].ee_len * block_size);

    blkdev->read(blkdev->ctx,
                 extents.arr[i].ee_start_lo * block_size,
                 extents.arr[i].ee_len * block_size,
                 buf);

    kfree(buf);
  }

  return 0;
}

static void ext4_parse_extent_tree(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr)
{
  if (eh->eh_depth == 0)
  {
    ext4_parse_extent_tree_leaf(blkdev, sb, eh, arr);
  }
  else
  {
    ext4_parse_extent_tree_internal(blkdev, sb, eh, arr);
  }
}

static void ext4_parse_extent_tree_internal(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr)
{
  uint64_t block_size = 1024 << sb->s_log_block_size;

  int entries_per_block = block_size / sizeof(ext4_extent_idx_t);

  ext4_extent_idx_t *idx_arr = kmalloc(block_size);
  blkdev->read(blkdev->ctx, ((uint64_t)eh->eh_entries * sizeof(ext4_extent_idx_t)) / block_size, block_size, idx_arr);

  for (int i = 0; i < eh->eh_entries; i++)
  {
    ext4_extent_idx_t *idx = &idx_arr[i];
    uint64_t child_block = ((uint64_t)idx->ei_leaf_hi << 32) | idx->ei_leaf_lo;

    ext4_extent_header_t *child_eh = kmalloc(sizeof(ext4_extent_header_t));
    blkdev->read(blkdev->ctx, child_block * block_size, sizeof(ext4_extent_header_t), child_eh);

    ext4_parse_extent_tree(blkdev, sb, child_eh, arr);
  }

  kfree(idx_arr);
}

static void ext4_parse_extent_tree_leaf(blkdev_handle_t *blkdev, ext4_superblock_t *sb, ext4_extent_header_t *eh, ext4_extent_arr_t *arr)
{
  uint64_t block_size = 1024 << sb->s_log_block_size;

  for (int i = 0; i < eh->eh_entries; i++)
  {
    ext4_extent_t *extent = (ext4_extent_t *)(eh + 1) + i;
    arr->arr[arr->count++] = *extent;
  }
}

static int ext4_traverse_root(blkdev_handle_t *blkdev, ext4_superblock_t *sb, char *path)
{
  if (*path == '/' && path[1] == '\0')
    return EXT4_ROOT_INO;
  return ext4_traverse_dir(blkdev, sb, EXT4_ROOT_INO, EXT4_ROOT_INO, path);
}

// TODO: implement non extent
static int ext4_traverse_dir(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t dir_inode, uint64_t parent_inode, char *path)
{
  const int block_size = 1024 << sb->s_log_block_size;
  const int inode_size = sb->s_inode_size;

  ext4_inode_t *inode = ext4_read_inode(blkdev, sb, dir_inode);

  if ((inode->i_mode & 0xF000) != EXT4_DIRECTORY)
  {
    printf("Inode %d is not a directory!\n", dir_inode);
    kfree(inode);
    return -1;
  }

  while (*path == '/')
    path++;

  if (*path == '\0')
  {
    kfree(inode);
    return dir_inode;
  }

  int path_len = 0;
  while (path[path_len] != '/' && path[path_len] != '\0')
    path_len++;

  ext4_extent_arr_t extents;
  extents.arr = kmalloc(sizeof(ext4_extent_t) * 128);
  extents.count = 0;
  ext4_parse_extent_tree(blkdev, sb, (ext4_extent_header_t *)&inode->i_block, &extents);
  kfree(inode);
  for (int k = 0; k < extents.count; k++)
  {
    uint8_t *buf = kmalloc(block_size * extents.arr[k].ee_len);

    // read directory data block
    blkdev->read(blkdev->ctx, extents.arr[k].ee_start_lo * block_size, extents.arr[k].ee_len * block_size, buf);

    ext4_dir_entry_2_t *dir_entry = (ext4_dir_entry_2_t *)buf;
    while ((uint8_t *)dir_entry < buf + block_size * extents.arr[k].ee_len)
    {
      if (dir_entry->inode != 0 && dir_entry->inode != dir_inode)
      {
        // recurse on subdirectory
        if (dir_entry->file_type == 0x2)
        {
          if (dir_entry->inode != parent_inode && strncmp(dir_entry->name, path, dir_entry->name_len) == 0)
          {
            kfree(buf);
            return ext4_traverse_dir(blkdev, sb, dir_entry->inode, dir_inode, path + path_len);
          }
        }
        else if (strncmp(dir_entry->name, path, dir_entry->name_len) == 0 && path[path_len] == '\0')
        {
          kfree(buf);
          return dir_entry->inode;
        }
      }

      if (dir_entry->rec_len == 0)
      {
        printf("Invalid directory entry with rec_len 0!\n");
        kfree(buf);
        break;
      }

      dir_entry = (ext4_dir_entry_2_t *)((uint8_t *)dir_entry + dir_entry->rec_len);
    }
    kfree(buf);
  }

  kfree(extents.arr);
  kfree(inode);

  return -1;
}

static ext4_inode_t *ext4_read_inode(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t inode_num)
{

  uint64_t block_size = 1024 << sb->s_log_block_size;
  uint64_t inodes_per_group = sb->s_inodes_per_group;
  uint64_t inode_size = sb->s_inode_size;

  uint64_t inode_bg = (inode_num - 1) / inodes_per_group;
  uint64_t inode_index = (inode_num - 1) % inodes_per_group;
  uint64_t bg_inode_table = ext4_bg_get_inode_tbl_block(blkdev, sb, inode_bg, inode_num);

  uint64_t offset = inode_index * inode_size;
  uint64_t inode_block = bg_inode_table + (offset / block_size);
  uint64_t inode_offset = offset % block_size;

  uint8_t *buf = kmalloc(block_size * 2);
  blkdev->read(blkdev->ctx,
               inode_block * block_size,
               block_size,
               buf);

  ext4_inode_t *inode_table = (ext4_inode_t *)buf;
  ext4_inode_t *inode =
      (ext4_inode_t *)((uint8_t *)inode_table + inode_offset);

  ext4_inode_t *result = kmalloc(sizeof(ext4_inode_t));
  *result = *inode;

  kfree(buf);
  return result;
}

static uint32_t ext4_bg_get_inode_tbl_block(blkdev_handle_t *blkdev, ext4_superblock_t *sb, uint64_t bg, uint64_t inode_num)
{
  uint64_t block_size = 1024 << sb->s_log_block_size;
  uint64_t inodes_per_group = sb->s_inodes_per_group;

  uint64_t group_desc_block = EXT4_SUPERBLOCK_OFFSET / block_size + 1 + bg * (sb->s_desc_size / block_size);
  uint8_t *group_desc_buf = kmalloc(block_size);
  blkdev->read(blkdev->ctx, group_desc_block * block_size, block_size, group_desc_buf);

  uint32_t bg_inode_table = *(uint32_t *)(group_desc_buf + (bg % (block_size / sb->s_desc_size)) * sb->s_desc_size + 8);
  kfree(group_desc_buf);

  return bg_inode_table;
}

static uint32_t crc32c_table[256];

static void crc32c_init(void)
{
  for (uint32_t i = 0; i < 256; i++)
  {
    uint32_t crc = i;

    for (int j = 0; j < 8; j++)
    {
      if (crc & 1)
        crc = (crc >> 1) ^ 0x82F63B78u;
      else
        crc >>= 1;
    }

    crc32c_table[i] = crc;
  }
}

static uint32_t crc32c_update(uint32_t crc,
                              const void *data,
                              size_t len)
{
  const uint8_t *p = data;

  while (len--)
  {
    crc = (crc >> 8) ^
          crc32c_table[(crc ^ *p++) & 0xff];
  }

  return crc;
}

static uint32_t ext4_superblock_checksum(const uint8_t *sb)
{
  uint32_t crc = 0xFFFFFFFF;

  crc = crc32c_update(crc, sb, EXT4_CHECKSUM_OFFSET);

  return crc;
}