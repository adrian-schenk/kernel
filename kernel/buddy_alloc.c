#include <stdint.h>
#include <stddef.h>
#include "memlayout.h"

#define MIN_ORDER 5              /* 2^5  = 32 bytes minimum block */
#define MAX_ORDER 19             /* 2^19 = 524288 bytes maximum block */

#define POOL_SIZE (1 << MAX_ORDER)
#define NUM_ORDERS (MAX_ORDER - MIN_ORDER + 1)

static uint8_t memory_pool;

typedef struct block {
    struct block* next;
    uint8_t order;
    uint8_t is_free;
} block_t;

static block_t* free_lists[NUM_ORDERS];

static inline size_t order_to_size(int order)
{
    return (size_t)1 << order;
}

static inline int size_to_order(size_t size)
{
    int order = MIN_ORDER;

    size += sizeof(block_t);

    while (((size_t)1 << order) < size)
        order++;

    return order;
}

static inline int order_index(int order)
{
    return order - MIN_ORDER;
}

static void push_block(block_t** list, block_t* block)
{
    block->next = *list;
    *list = block;
}

static block_t* pop_block(block_t** list)
{
    block_t* b = *list;

    if (b)
        *list = b->next;

    return b;
}

static void remove_block(block_t** list, block_t* target)
{
    block_t* prev = NULL;
    block_t* curr = *list;

    while (curr)
    {
        if (curr == target)
        {
            if (prev)
                prev->next = curr->next;
            else
                *list = curr->next;

            return;
        }

        prev = curr;
        curr = curr->next;
    }
}

static block_t* get_buddy(block_t* block)
{
    uintptr_t offset =
        (uintptr_t)((uint8_t*)block - memory_pool);

    uintptr_t buddy_offset =
        offset ^ order_to_size(block->order);

    return (block_t*)(memory_pool + buddy_offset);
}

void buddy_init(void)
{
    memory_pool = PHYS_TABLE_REGION;
    for (int i = 0; i < NUM_ORDERS; i++)
        free_lists[i] = NULL;

    block_t* initial = (block_t*)memory_pool;

    initial->order = MAX_ORDER;
    initial->is_free = 1;
    initial->next = NULL;

    push_block(
        &free_lists[order_index(MAX_ORDER)],
        initial
    );
}

static void split_block(block_t* block)
{
    int new_order = block->order - 1;

    size_t size = order_to_size(new_order);

    block_t* buddy =
        (block_t*)((uint8_t*)block + size);

    block->order = new_order;
    buddy->order = new_order;

    block->is_free = 1;
    buddy->is_free = 1;

    push_block(
        &free_lists[order_index(new_order)],
        buddy
    );
}

void* buddy_alloc(size_t size)
{
    int needed_order = size_to_order(size);

    if (needed_order > MAX_ORDER)
        return NULL;

    int current_order = needed_order;

    while (current_order <= MAX_ORDER)
    {
        block_t* block =
            free_lists[order_index(current_order)];

        if (block)
            break;

        current_order++;
    }

    if (current_order > MAX_ORDER)
        return NULL;

    block_t* block =
        pop_block(
            &free_lists[order_index(current_order)]
        );

    while (current_order > needed_order)
    {
        split_block(block);

        current_order--;

        push_block(
            &free_lists[order_index(current_order)],
            block
        );

        block = pop_block(
            &free_lists[order_index(current_order)]
        );
    }

    block->is_free = 0;

    return (void*)(block + 1);
}

static void merge_block(block_t* block)
{
    while (block->order < MAX_ORDER)
    {
        block_t* buddy = get_buddy(block);

        if (!buddy->is_free)
            break;

        if (buddy->order != block->order)
            break;

        remove_block(
            &free_lists[order_index(buddy->order)],
            buddy
        );

        if (buddy < block)
            block = buddy;

        block->order++;

        block->is_free = 1;
    }

    push_block(
        &free_lists[order_index(block->order)],
        block
    );
}

void buddy_free(void* ptr)
{
    if (!ptr)
        return;

    block_t* block =
        ((block_t*)ptr) - 1;

    block->is_free = 1;

    merge_block(block);
}