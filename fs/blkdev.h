#pragma once
#include <stdint.h>

typedef int (*blkdev_read_func)(struct blkdev_handle* handle, uint64_t offset, uint64_t size, void* buffer);
typedef int (*blkdev_write_func)(struct blkdev_handle* handle, uint64_t offset, uint64_t size, void* buffer);
typedef int (*blkdev_open_func)(struct blkdev_handle* handle);
typedef int (*blkdev_close_func)(struct blkdev_handle* handle);

typedef struct blkdev_handle {
  void *ctx;
  blkdev_open_func open;
  blkdev_read_func read;
  blkdev_write_func write;
  blkdev_close_func close;
} blkdev_handle_t;