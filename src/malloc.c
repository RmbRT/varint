#include "malloc.h"
#include "heap.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

static HeapList heap_list;
static int heap_list_initialised = 0;

void vi_set_default_heap_size(size_t capacity)
{
#ifdef CUSTOM_HEAP
	#pragma omp critical(heap)
	{
		if(!heap_list_initialised)
		{
			vi_create_HeapList(&heap_list);
			heap_list_initialised = 1;
		}

		heap_list.min_capacity = capacity;
	}
#endif
}

static void * _malloc(size_t typesize, size_t count)
{
	assert(typesize != 0);
	assert(count != 0);

	void * ptr = NULL;
	ptr = vi_alloc_HeapList(&heap_list, typesize * count);

	assert(ptr != NULL && "malloc failed");
	assert(vi_entry_of_block((uintptr_t)ptr)->heap != NULL);
	return ptr;
}

void vi_malloc(void ** ptr, size_t typesize, size_t count)
{
	assert(heap_list_initialised);

	assert(ptr != NULL);
	assert(*ptr == NULL);

#ifdef CUSTOM_HEAP
	#pragma omp critical (heap)
	*ptr = _malloc(typesize, count);
#else
	*ptr = malloc(typesize * count);
#endif
}

void vi_calloc(void ** ptr, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(*ptr == NULL);
	assert(typesize != 0);
	assert(count != 0);

#ifdef CUSTOM_HEAP
	vi_malloc(ptr, typesize, count);
	memset(*ptr, 0, typesize * count);
#else
	*ptr = calloc(typesize, count);
#endif
}


static void _realloc(void ** ptr, size_t typesize, size_t count)
{
	if(*ptr)
	{
		Entry * entry = vi_entry_of_block((uintptr_t)*ptr);

		// little sanity check.
		assert(entry->heap != NULL);
		assert(!entry->previous || entry->previous->next == entry);
		assert(!entry->next || entry->next->previous == entry);

		size_t cap = entry->reserved;
		if(cap >= typesize * count)
			return;

		size_t hole;
		if(entry->next)
		{
			hole = (uintptr_t)entry->next - vi_end_Entry(entry);
		} else
		{
			hole = entry->heap->end - vi_end_Entry(entry);
		}

		if(hole + cap >= count)
		{
			entry->reserved = count;
		} else
		{
			void * reloc = NULL;
			reloc = _malloc(typesize, count);
			memcpy(reloc, *ptr, cap);
			vi_free_block(ptr);
			*ptr = reloc;
		}
	} else
	{
		*ptr = _malloc(typesize, count);
	}
}

void vi_realloc(void ** ptr, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(typesize != 0);
	assert(count != 0);
#ifdef CUSTOM_HEAP
	#pragma omp critical (heap)
	_realloc(ptr, typesize, count);
#else
	*ptr = realloc(*ptr, typesize * count);
#endif
}
void vi_free(void ** ptr)
{
	assert(ptr != NULL);
	assert(*ptr != NULL);
#ifdef CUSTOM_HEAP
	#pragma omp critical(heap)
	vi_free_block(*ptr);
#else
	free(*ptr);
#endif
	*ptr = NULL;
}
void vi_copy(void ** ptr, void const * src, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(*ptr == NULL);
	assert(src != NULL);
	assert(typesize != 0);
	assert(count != 0);

	vi_malloc(ptr, typesize, count);
	memcpy(*ptr, src, typesize * count);
}

void vi_destroy_heap()
{
#ifdef CUSTOM_HEAP
	#pragma omp critical(heap)
	if(heap_list_initialised)
	{
		vi_destroy_HeapList(&heap_list);
		heap_list_initialised = 0;
	}
#endif
}