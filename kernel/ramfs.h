#pragma once
#include <stdint.h>
#include "fs.h"

typedef struct ramfs {
  uint8_t* data;
  uint64_t size;
} ramfs_t;

blkdev_handle_t *ramfs_blkdev_create(uint64_t base, uint64_t size);

int ramfs_blk_read(fs_handle_t* handle, uint64_t offset, uint64_t size, void* buffer);
int ramfs_blk_write(fs_handle_t* handle, uint64_t offset, uint64_t size, void* buffer);
int ramfs_blk_open(fs_handle_t* handle);
int ramfs_blk_close(fs_handle_t* handle);