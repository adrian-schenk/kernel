#pragma once
#include <stdint.h>
#include <blkdev.h>

typedef int (*fs_mount)(struct fs_handle* handle);
typedef int (*fs_unmount)(struct fs_handle* handle);
typedef int (*fs_mkfs_func)(struct fs_handle* handle);

typedef int (*fs_open_func)(struct fs_handle* handle);
typedef int (*fs_create_func)(struct fs_handle* handle);
typedef int (*fs_remove_func)(struct fs_handle* handle);
typedef int (*fs_close_func)(struct fs_handle* handle);

typedef struct fs_handle {
  void *fs_ctx;
  blkdev_handle_t *blk_dev;
  
  fs_open_func fopen;
  fs_create_func fcreate;
  fs_remove_func fremove;
  fs_close_func fclose;

  fs_mount mount;
  fs_unmount unmount;
  fs_mkfs_func mkfs;
} fs_handle_t;