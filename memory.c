#include "memory.h"
#include "error.h"
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static
struct memory
{
	uint8_t *start;
	uint8_t *end;
	uint8_t *next;
} pool;

size_t memory_allocate(size_t nbytes)
{
	FAILURE_IF(pool.start != NULL, 0, "Memory pool already allocated.");
	FAILURE_IF(nbytes == 0, 0, "Memory pool size must be positive.");
	
	pool.start = calloc(nbytes, sizeof(uint8_t));
	FAILURE_IF(pool.start == NULL, 0, "Failed to allocate memory pool.");
	
	pool.end = pool.start + nbytes;
	pool.next = pool.start;
	
	return nbytes;
}

void * memory_take(size_t nbytes)
{
	FAILURE_IF(pool.start == NULL, NULL, "Memory pool is not allocated.");
	FAILURE_IF(pool.next >= pool.end, NULL,
		"Memory pool limit exceeded: %u out of %u bytes used.",
		memory_usage(), memory_capacity());
	uint8_t *mem = pool.next;
	FAILURE_IF(pool.next + nbytes > pool.end, NULL, "Too big chunk to allocate.");
	pool.next += nbytes;
	return mem;
}

void memory_free(void)
{
	free(pool.start);
	pool.start = NULL;
	pool.end = NULL;
	pool.next = NULL;
}

size_t memory_usage(void)
{
	return (pool.next - pool.start);
}

size_t memory_capacity(void)
{
	return (pool.end - pool.start);
}

size_t memory_available(void)
{
	return (pool.end - pool.next);
}
