#include "ramfs.h"
#include "kmalloc.h"

blkdev_handle_t *ramfs_blkdev_create(uint64_t base, uint64_t size) {
  ramfs_t* fs = kmalloc(sizeof(ramfs_t));
  fs->data = (uint8_t*) base;
  fs->size = size;

  blkdev_handle_t* handle = kmalloc(sizeof(blkdev_handle_t));
  handle->ctx = fs;
  handle->open = ramfs_blk_open;
  handle->read = ramfs_blk_read;
  handle->write = ramfs_blk_write;
  handle->close = ramfs_blk_close;

  return handle;
}

int ramfs_blk_read(fs_handle_t* handle, uint64_t offset, uint64_t size, void* buffer) {

  ramfs_t* fs = (ramfs_t*) handle->blk_dev->ctx;

  if (size == 0)
		return -1;

	if ((offset % 512) || (size % 512))
		return -1;

	uint64_t drive_size = fs->size;

	if (offset + size > drive_size)
		return -1;

  for (uint64_t i = 0; i < size; i++) {
    ((uint8_t*) buffer)[i] = fs->data[offset + i];
  }
  return 0;
}

int ramfs_blk_write(fs_handle_t* handle, uint64_t offset, uint64_t size, void* buffer) {

  ramfs_t* fs = (ramfs_t*) handle->blk_dev->ctx;

  if (size == 0)
		return -1;

	if ((offset % 512) || (size % 512))
		return -1;

	uint64_t drive_size = fs->size;

	if (offset + size > drive_size)
		return -1;

  for (uint64_t i = 0; i < size; i++) {
    fs->data[offset + i] = ((uint8_t*) buffer)[i];
  }
}

int ramfs_blk_open(fs_handle_t* handle) {
  return 0;
}

int ramfs_blk_close(fs_handle_t* handle) {
  return 0;
}