#pragma once
#include <stdint.h>

typedef int (*fd_open_func)(struct fd_handle* handle, const char* path);
typedef int (*fd_write_func)(struct fd_handle* handle, uint64_t offset, uint64_t size, void* buffer);
typedef int (*fd_read_func)(struct fd_handle* handle, uint64_t offset, uint64_t size, void* buffer);
typedef int (*fd_close_func)(struct fd_handle* handle);

typedef struct fd_handle {
  void *ctx;
  
  fd_open_func fopen;
  fd_write_func fwrite;
  fd_read_func fread;
  fd_close_func fclose;
} fd_handle_t;