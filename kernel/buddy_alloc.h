#pragma once

#include <stddef.h>
#include <stdint.h>

#define MIN_ORDER 5
#define MAX_ORDER 16

#define POOL_SIZE (1 << MAX_ORDER)

typedef struct block block_t;

void buddy_init(void);

void* buddy_alloc(size_t size);

void buddy_free(void* ptr);