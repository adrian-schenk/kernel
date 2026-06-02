#pragma once
#include <stdint.h>
#include <stddef.h>
#include <fs.h>

/* s_feature_compat */
#define EXT4_FEATURE_COMPAT_DIR_PREALLOC 0x0001
#define EXT4_FEATURE_COMPAT_HAS_JOURNAL 0x0004
#define EXT4_FEATURE_COMPAT_DIR_INDEX 0x0020 // htree dirs
#define EXT4_FEATURE_COMPAT_SPARSE_SUPER2 0x0200

/* s_feature_incompat — must support all of these to mount r/w */
#define EXT4_FEATURE_INCOMPAT_FILETYPE 0x0002 // dir entries store file type
#define EXT4_FEATURE_INCOMPAT_RECOVER 0x0004  // journal needs recovery
#define EXT4_FEATURE_INCOMPAT_EXTENTS 0x0040  // extent tree (not block map)
#define EXT4_FEATURE_INCOMPAT_64BIT 0x0080    // >2^32 blocks
#define EXT4_FEATURE_INCOMPAT_FLEX_BG 0x0200
#define EXT4_FEATURE_INCOMPAT_ENCRYPT 0x10000

/* s_feature_ro_compat — mount read-only if any unknown flags set */
#define EXT4_FEATURE_RO_COMPAT_SPARSE_SUPER 0x0001
#define EXT4_FEATURE_RO_COMPAT_LARGE_FILE 0x0002 // files > 2GB
#define EXT4_FEATURE_RO_COMPAT_HUGE_FILE 0x0008
#define EXT4_FEATURE_RO_COMPAT_METADATA_CSUM 0x0400 // s_checksum is valid

#define EXT4_MAGIC 0xEF53

/* s_state */
#define EXT4_VALID_FS 0x0001 // cleanly unmounted
#define EXT4_ERROR_FS 0x0002 // errors detected

typedef struct __attribute__((packed)) ext4_superblock
{
  /* --- base ext2 fields (offsets 0x000–0x054) --- */
  uint32_t s_inodes_count;         // 0x000 total inode count
  uint32_t s_blocks_count_lo;      // 0x004 total block count (low 32 bits)
  uint32_t s_r_blocks_count_lo;    // 0x008 reserved block count (low 32 bits)
  uint32_t s_free_blocks_count_lo; // 0x00C free block count (low 32 bits)
  uint32_t s_free_inodes_count;    // 0x010 free inode count
  uint32_t s_first_data_block;     // 0x014 first data block (0 for >1KB blocks)
  uint32_t s_log_block_size;       // 0x018 block size = 1024 << s_log_block_size
  uint32_t s_log_cluster_size;     // 0x01C cluster size (bigalloc feature)
  uint32_t s_blocks_per_group;     // 0x020 blocks per block group
  uint32_t s_clusters_per_group;   // 0x024 clusters per block group
  uint32_t s_inodes_per_group;     // 0x028 inodes per block group
  uint32_t s_mtime;                // 0x02C last mount time (UNIX)
  uint32_t s_wtime;                // 0x030 last write time (UNIX)
  uint16_t s_mnt_count;            // 0x034 mount count since last fsck
  uint16_t s_max_mnt_count;        // 0x036 max mounts before fsck (0xFFFF = disabled)
  uint16_t s_magic;                // 0x038 magic: must be 0xEF53
  uint16_t s_state;                // 0x03A fs state (1=clean, 2=errors, 4=orphans)
  uint16_t s_errors;               // 0x03C error behaviour (1=continue, 2=remount-ro, 3=panic)
  uint16_t s_minor_rev_level;      // 0x03E minor revision
  uint32_t s_lastcheck;            // 0x040 last fsck time (UNIX)
  uint32_t s_checkinterval;        // 0x044 max time between fscks
  uint32_t s_creator_os;           // 0x048 creator OS (0=Linux, 3=FreeBSD…)
  uint32_t s_rev_level;            // 0x04C revision (0=original, 1=dynamic)
  uint16_t s_def_resuid;           // 0x050 default UID for reserved blocks
  uint16_t s_def_resgid;           // 0x052 default GID for reserved blocks

  /* --- ext2 dynamic rev fields (offsets 0x054–0x07F, rev >= 1) --- */
  uint32_t s_first_ino;              // 0x054 first non-reserved inode (11 for rev>=1)
  uint16_t s_inode_size;             // 0x058 inode size in bytes (128 ext2, 256 ext4)
  uint16_t s_block_group_nr;         // 0x05A block group this superblock is in
  uint32_t s_feature_compat;         // 0x05C compatible feature flags
  uint32_t s_feature_incompat;       // 0x060 incompatible feature flags
  uint32_t s_feature_ro_compat;      // 0x064 read-only compatible feature flags
  uint8_t s_uuid[16];                // 0x068 filesystem UUID
  char s_volume_name[16];            // 0x078 volume label (null-terminated)
  char s_last_mounted[64];           // 0x088 last mount path (null-terminated)
  uint32_t s_algorithm_usage_bitmap; // 0x0C8 compression algorithms (unused in ext4)

  /* --- ext3 fields (offsets 0x0CC–0x0FF) --- */
  uint8_t s_prealloc_blocks;      // 0x0CC preallocate blocks for files
  uint8_t s_prealloc_dir_blocks;  // 0x0CD preallocate blocks for dirs
  uint16_t s_reserved_gdt_blocks; // 0x0CE blocks reserved for future GDT expansion
  uint8_t s_journal_uuid[16];     // 0x0D0 journal superblock UUID
  uint32_t s_journal_inum;        // 0x0E0 journal inode number
  uint32_t s_journal_dev;         // 0x0E4 journal device number (external journal)
  uint32_t s_last_orphan;         // 0x0E8 head of orphaned inode list
  uint32_t s_hash_seed[4];        // 0x0EC htree hash seed
  uint8_t s_def_hash_version;     // 0x0FC default hash version for dir hashes
  uint8_t s_jnl_backup_type;      // 0x0FD journal backup type
  uint16_t s_desc_size;           // 0x0FE group descriptor size (32 or 64 bytes)

  /* --- ext4 fields (offsets 0x100–0x1FF) --- */
  uint32_t s_default_mount_opts; // 0x100 default mount options
  uint32_t s_first_meta_bg;      // 0x104 first meta block group
  uint32_t s_mkfs_time;          // 0x108 filesystem creation time (UNIX)
  uint32_t s_jnl_blocks[17];     // 0x10C backup of journal inode block array
  /* 64-bit feature fields (require INCOMPAT_64BIT) */
  uint32_t s_blocks_count_hi;         // 0x150 total block count (high 32 bits)
  uint32_t s_r_blocks_count_hi;       // 0x154 reserved block count (high 32 bits)
  uint32_t s_free_blocks_count_hi;    // 0x158 free block count (high 32 bits)
  uint16_t s_min_extra_isize;         // 0x15C min extra inode reserved bytes
  uint16_t s_want_extra_isize;        // 0x15E desired extra inode reserved bytes
  uint32_t s_flags;                   // 0x160 miscellaneous flags
  uint16_t s_raid_stride;             // 0x164 RAID stride in blocks
  uint16_t s_mmp_update_interval;     // 0x166 MMP check interval (seconds)
  uint64_t s_mmp_block;               // 0x168 MMP block number
  uint32_t s_raid_stripe_width;       // 0x170 RAID stripe width in blocks
  uint8_t s_log_groups_per_flex;      // 0x174 flex_bg group size = 2^this
  uint8_t s_checksum_type;            // 0x175 checksum type (1 = crc32c)
  uint16_t s_reserved_pad;            // 0x176 padding
  uint64_t s_kbytes_written;          // 0x178 total KB written over lifetime
  uint32_t s_snapshot_inum;           // 0x180 snapshot inode number
  uint32_t s_snapshot_id;             // 0x184 sequential snapshot ID
  uint64_t s_snapshot_r_blocks_count; // 0x188 reserved blocks for snapshot
  uint32_t s_snapshot_list;           // 0x190 head of on-disk snapshot list
  uint32_t s_error_count;             // 0x194 number of fs errors seen
  uint32_t s_first_error_time;        // 0x198 time of first error (UNIX)
  uint32_t s_first_error_ino;         // 0x19C inode involved in first error
  uint64_t s_first_error_block;       // 0x1A0 block involved in first error
  uint8_t s_first_error_func[32];     // 0x1A8 function where error occurred
  uint32_t s_first_error_line;        // 0x1C8 line number of first error
  uint32_t s_last_error_time;         // 0x1CC time of last error (UNIX)
  uint32_t s_last_error_ino;          // 0x1D0 inode involved in last error
  uint32_t s_last_error_line;         // 0x1D4 line number of last error
  uint64_t s_last_error_block;        // 0x1D8 block involved in last error
  uint8_t s_last_error_func[32];      // 0x1E0 function where last error occurred
  uint8_t s_mount_opts[64];           // 0x200 mount options string
  uint32_t s_usr_quota_inum;          // 0x240 inode for user quota file
  uint32_t s_grp_quota_inum;          // 0x244 inode for group quota file
  uint32_t s_overhead_clusters;       // 0x248 overhead blocks/clusters in fs
  uint32_t s_backup_bgs[2];           // 0x24C block groups with sparse_super2 SBs
  uint8_t s_encrypt_algos[4];         // 0x254 encryption algorithms in use
  uint8_t s_encrypt_pw_salt[16];      // 0x258 salt for string2key
  uint32_t s_lpf_ino;                 // 0x268 inode of lost+found
  uint32_t s_prj_quota_inum;          // 0x26C inode for project quota file
  uint32_t s_checksum_seed;           // 0x270 crc32c(uuid) for metadata checksums
  uint8_t s_wtime_hi;                 // 0x274 high byte of s_wtime
  uint8_t s_mtime_hi;                 // 0x275 high byte of s_mtime
  uint8_t s_mkfs_time_hi;             // 0x276 high byte of s_mkfs_time
  uint8_t s_lastcheck_hi;             // 0x277 high byte of s_lastcheck
  uint8_t s_first_error_time_hi;      // 0x278 high byte of s_first_error_time
  uint8_t s_last_error_time_hi;       // 0x279 high byte of s_last_error_time
  uint8_t s_pad[2];                   // 0x27A padding
  uint16_t s_encoding;                // 0x27C filename charset encoding
  uint16_t s_encoding_flags;          // 0x27E filename charset encoding flags
  uint32_t s_orphan_file_inum;        // 0x280 inode for orphan file (orphan_file feature)
  uint32_t s_reserved[94];            // 0x284 padding to 1024 bytes
  uint32_t s_checksum;                // 0x3FC crc32c of superblock (metadata_csum feature)
} ext4_superblock_t;

_Static_assert(sizeof(ext4_superblock_t) == 1024, "ext4 superblock must be exactly 1024 bytes");

enum ext4_reserved_inodes {
  EXT4_ROOT_INO = 2, // root directory
  EXT4_ACL_IDX_INO = 3, // ACL index (if ACLs enabled)
  EXT4_ACL_DATA_INO = 4, // ACL data (if ACLs enabled)
  EXT4_BOOT_LOADER_INO = 5, // boot loader (if bootable)
  EXT4_UNDEL_DIR_INO = 6, // undelete directory (if undelete feature enabled)
  EXT4_RESIZE_INO = 7, // reserved for resize operations
  EXT4_JOURNAL_INO = 8 // journal inode (if journaling enabled)
};

enum ext4_inode_type
{
  EXT4_FIFO = 0x1000,
  EXT4_CHAR_DEVICE = 0x2000,
  EXT4_DIRECTORY = 0x4000,
  EXT4_BLOCK_DEVICE = 0x6000,
  EXT4_REGULAR_FILE = 0x8000,
  EXT4_SYMBOLIC_LINK = 0xA000,
  EXT4_UNIX_SOCKET = 0xC000
};

typedef struct ext4_inode
{
  uint16_t i_mode;
  uint16_t i_uid;
  uint32_t i_size_lo;
  uint32_t i_atime;
  uint32_t i_ctime;
  uint32_t i_mtime;
  uint32_t i_dtime;
  uint16_t i_gid;
  uint16_t i_links_count;
  uint32_t i_blocks_lo;
  uint32_t i_flags;
  char i_osd1[4];
  uint32_t i_block[15];
  uint32_t i_generation;
  uint32_t i_file_acl_lo;
  uint32_t i_size_high;
  uint32_t i_obso_faddr;
  char i_osd2[12];
  uint16_t i_extra_isize;
  uint16_t i_checksum_hi;
  uint32_t i_ctime_extra;
  uint32_t i_mtime_extra;
  uint32_t i_atime_extra;
  uint32_t i_crtime;
  uint32_t i_crtime_extra;
  uint32_t i_version_hi;
  uint32_t i_projid;
} __attribute__((packed)) ext4_inode_t;

_Static_assert(sizeof(ext4_inode_t) == 0xa0, "ext4 inode must be exactly 128 bytes");

typedef struct ext4_extent_header {
  uint16_t eh_magic;    // 0xF30A
  uint16_t eh_entries;  // number of valid entries
  uint16_t eh_max;      // capacity of store in entries
  uint16_t eh_depth;    // depth of extent tree (0 for leaf)
  uint32_t eh_generation;
} ext4_extent_header_t;

typedef struct ext4_extent_idx {
  uint32_t ei_block;    // first logical block covered by extent
  uint32_t ei_leaf_lo;     // block number of the child node
  uint16_t ei_leaf_hi;    // high 16 bits of block number of child node
  uint16_t ei_unused;
} ext4_extent_idx_t;

typedef struct ext4_extent {
  uint32_t ee_block;    // first logical block covered by extent
  uint16_t ee_len;      // number of blocks covered by extent
  uint16_t ee_start_hi; // high 16 bits of physical block
  uint32_t ee_start_lo; // low 32 bits of physical block
} ext4_extent_t;

typedef struct ext4_extent_tail {
  uint32_t et_checksum; // crc32c(uuid+extent_header+extent_index/extent)
} ext4_extent_tail_t;

typedef struct ext4_extent_arr {
  struct ext4_extent* arr;
  size_t count;
  size_t capacity;
} ext4_extent_arr_t;

typedef struct ext4_dir_entry
{
  uint32_t inode;
  uint16_t rec_len;
  uint16_t name_len;
  char name[];
} ext4_dir_entry_t;

typedef struct ext4_dir_entry_t
{
  uint32_t inode;
  uint16_t rec_len;
  uint8_t name_len;
  uint8_t file_type;
  char name[];
} ext4_dir_entry_2_t;

typedef struct ext4_fs
{
  ext4_superblock_t superblock;
} ext4_fs_t;

fs_handle_t *ext4_handle_create(blkdev_handle_t *blk_dev);

int ext4_mount(fs_handle_t *handle);
int ext4_unmount(fs_handle_t *handle);
int ext4_mkfs(fs_handle_t *handle);

void ext4_open(fs_handle_t *handle);
void ext4_create(fs_handle_t *handle);
void ext4_remove(fs_handle_t *handle);
void ext4_close(fs_handle_t *handle);
