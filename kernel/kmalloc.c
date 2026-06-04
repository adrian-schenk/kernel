/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

#include "kmalloc.h"
#include "console.h"
#include "printf.h"
#include "spinlock.h"
#include "interrupt.h"

#define ALIGNMENT 8

#define KUNIT sizeof(struct kmalloc_chunk)

#define KMALLOC_STATE_FREE 0xa1a1a1a1
#define KMALLOC_STATE_USED 0xbfbfbfbf

struct kmalloc_chunk {
	int state;
	int length;
	struct kmalloc_chunk *next;
	struct kmalloc_chunk *prev;
};

int kmalloc_lock;

static struct kmalloc_chunk *head = 0;

static inline uintptr_t align_up(uintptr_t x)
{
    return (x + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

static int align(int n)
{
    return (n + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

/*
Initialize the linked list by creating a single chunk at
a given start address and length.  The chunk is initially
free and has no next or previous chunks.
*/

void kmalloc_init(char *start, int length)
{
    uintptr_t aligned = align_up((uintptr_t)start);

    length -= (aligned - (uintptr_t)start);

    head = (struct kmalloc_chunk *)aligned;

    head->state = KMALLOC_STATE_FREE;
    head->length = length;
    head->next = 0;
    head->prev = 0;
}

/*
Split a large chunk into two, such that the current chunk
has the desired length, and the next chunk has the remainder.
*/

static void ksplit(struct kmalloc_chunk *c, int length)
{
	struct kmalloc_chunk *n = (struct kmalloc_chunk *) ((char *) c + length);

	n->state = KMALLOC_STATE_FREE;
	n->length = c->length - length;
	n->prev = c;
	n->next = c->next;

	if(c->next)
		c->next->prev = n;

	c->next = n;
	c->length = length;
}

/*
Allocate a chunk of memory of the given length.
To avoid fragmentation, round up the length to
a multiple of the chunk size.  Then, search fo
a chunk of the desired size, and split it if necessary.
*/

void *kmalloc(int length)
{
	lock(&kmalloc_lock);
	// round up length to a multiple of KUNIT
	int extra = length % KUNIT;
	if(extra)
		length += (KUNIT - extra);

	// then add one more unit to accommodate the chunk header
	length += KUNIT;

	struct kmalloc_chunk *c = head;
	while(1) {
		if(!c) {
			kprintf("kmalloc: out of memory!\n");
			return 0;
		}
		if(c->state == KMALLOC_STATE_FREE && c->length >= length)
			break;
		c = c->next;
	}

	// split the chunk if the remainder is greater than two units
	if((c->length-length) > 2 * KUNIT) {
		ksplit(c, length);
	}

	c->state = KMALLOC_STATE_USED;

	// return a pointer to the memory following the chunk header
	unlock(&kmalloc_lock);
	return (c + 1);
}

/*
Attempt to merge a chunk with its successor,
if it exists and both are in the free state.
*/

static void kmerge(struct kmalloc_chunk *c)
{
	if(!c)
		return;

	if(c->state != KMALLOC_STATE_FREE)
		return;

	if(c->next && c->next->state == KMALLOC_STATE_FREE) {
		c->length += c->next->length;
		if(c->next->next) {
			c->next->next->prev = c;
		}
		c->next = c->next->next;
	}

}

/*
Free memory by marking the chunk as de-allocated,
then attempting to merge it with the predecessor and successor.
*/

void kfree(void *ptr)
{
	struct kmalloc_chunk *c = (struct kmalloc_chunk *) ptr;
	c--;

	if(c->state != KMALLOC_STATE_USED) {
		kprintf("invalid kfree(%x)\n", ptr);
		return;
	}

	c->state = KMALLOC_STATE_FREE;

	kmerge(c);
	kmerge(c->prev);
}

void kmalloc_debug()
{
	struct kmalloc_chunk *c;

	kprintf("state ptr      prev     next     length\n");

	for(c = head; c; c = c->next) {
		if(c->state == KMALLOC_STATE_FREE) {
			kprintf("F");
		} else if(c->state == KMALLOC_STATE_USED) {
			kprintf("U");
		} else {
			kprintf("kmalloc list corrupted at %x!\n", c);
			return;
		}
		kprintf("     %x %x %x %d\n", c, c->prev, c->next, c->length);
	}
}