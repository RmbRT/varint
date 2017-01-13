#include "heap.h"
#include <assert.h>
#include <stdlib.h>

uintptr_t vi_end_Entry(
	Entry * this)
{
	assert(this != NULL);
	return (uintptr_t) this + sizeof(Entry) + this->reserved;
}

uintptr_t vi_begin_Entry(
	Entry * this)
{
	assert(this != NULL);
	return (uintptr_t) this + sizeof(Entry);
}

Entry * vi_entry_of_block(
	uintptr_t block)
{
	assert(block != 0);

	return (Entry*)(block - sizeof(Entry));
}


static inline uintptr_t align(
	uintptr_t ptr,
	size_t size)
{
	size_t rest;
	if((rest = ptr % size))
		return ptr + size - rest;
	else
		return ptr;
}

size_t vi_hole_Entry(
	Entry * this)
{
	assert(this != NULL);

	uintptr_t end = align(vi_end_Entry(this), sizeof(Entry));

	if(this->next)
		return (uintptr_t) this->next > end
			? (uintptr_t) this->next - end
			: 0;
	else
		return this->heap->end > end
			? this->heap->end - end
			: 0;
}

void vi_create_Entry(
	Entry * this,
	Heap * heap,
	Entry * previous,
	Entry * next,
	size_t reserved)
{
	assert(this != NULL);
	assert(heap != NULL);
	assert(reserved != 0);

	this->heap = heap;
	this->next = next;
	this->previous = previous;
	this->reserved = reserved;

	if(next)
		next->previous = this;
	if(previous)
		previous->next = this;
}

void vi_destroy_Entry(
	Entry * this)
{
	assert(this != NULL);

	if(this->heap->first == this)
		this->heap->first = this->next;
	if(this->heap->last == this)
		this->heap->last = this->previous;

	--this->heap->blocks;

	if(this->previous)
		this->previous->next = this->next;
	if(this->next)
		this->next->previous = this->previous;

	this->heap = NULL;
	this->next = NULL;
	this->previous = NULL;
}


void vi_create_Heap(
	Heap * this,
	size_t capacity)
{
	assert(this != NULL);

	this->start = (uintptr_t) malloc(capacity);
	assert(this->start != 0);

	this->end = this->start + capacity;
	this->first = this->last = 0;
	this->blocks = 0;
}

void vi_destroy_Heap(
	Heap * this)
{
	assert(this != NULL);
	assert(this->blocks == 0 && "tried to destroy non-empty heap.");

	for(Entry * it = this->first; it != NULL;)
	{
		Entry * next = it->next;

		vi_destroy_Entry(it);

		it = next;
	}

	this->first = this->last = NULL;

	if(this->start)
		free((void*) this->start);
	this->start = this->end = 0;
}


void * vi_alloc_Heap(
	Heap * const this,
	size_t size)
{
	assert(this != NULL);

	// check for empty heap.
	if(!this->blocks)
	{
		Entry * entry = (Entry*) align(this->start, sizeof(Entry));
		uintptr_t begin = vi_begin_Entry(entry);
		if(begin + size < this->end)
		{
			this->first = this->last = entry;
			vi_create_Entry(
				entry,
				this,
				NULL,
				NULL,
				size);
			++this->blocks;
			return (void*) begin;
		} else
		{
			return NULL;
		}
	}

	// heap is not empty.

	Entry * left = (Entry*) align(this->start, sizeof(Entry));

	// check before the first element.
	if(this->first > left)
	{
		if((uintptr_t)this->first - vi_begin_Entry(left) > size)
		{
			vi_create_Entry(
				left,
				this,
				NULL,
				this->first,
				size);

			this->first = left;
			++this->blocks;
			return (void*)vi_begin_Entry(left);
		} else
		{
			left = this->first;
		}
	}

	Entry * right;

	{
		size_t align_e_s = align(this->end - size, sizeof(Entry));
		if(this->end - align_e_s < size)
			align_e_s -= sizeof(Entry);

		right = vi_entry_of_block(align_e_s);
	}

	// can it fit after the last entry?
	if((uintptr_t)right >= vi_end_Entry(this->last))
	{
		vi_create_Entry(
			right,
			this,
			this->last,
			NULL,
			size);
		this->last = right;

		++this->blocks;
		return (void*)vi_begin_Entry(right);
	} else
	{
		right = this->last;
	}

	while(left <= right)
	{
		// check before the right element.
		if(right->previous)
		{
			if(vi_hole_Entry(right->previous) >= sizeof(Entry) + size)
			{
				Entry * insert = (Entry*) align(vi_end_Entry(right->previous), sizeof(Entry));
				vi_create_Entry(
					insert,
					this,
					right->previous,
					right,
					size);
				++this->blocks;
				return (void*)vi_begin_Entry(insert);
			} else
				right = right->previous;
		}

		// check after the left element.
		if(left->next)
		{
			if(vi_hole_Entry(left) >= sizeof(Entry) + size)
			{
				Entry * insert = (Entry*) align(vi_end_Entry(left), sizeof(Entry));
				vi_create_Entry(
					insert,
					this,
					left,
					left->next,
					size);
				++this->blocks;
				return (void*)vi_begin_Entry(insert);
			} else
				left = left->next;
		}
	}
	
	return NULL;
}

HeapListEntry * vi_HeapListEntry_Heap(
	Heap * this)
{
	assert(this != NULL);

	return (HeapListEntry *)((uintptr_t) this - offsetof(HeapListEntry, heap));
}


void vi_create_HeapListEntry(
	HeapListEntry * this,
	HeapList * list,
	HeapListEntry * prev,
	HeapListEntry * next,
	size_t capacity)
{
	assert(this != NULL);

	this->list = list;
	this->previous = prev;
	this->next = next;

	vi_create_Heap(&this->heap, capacity);
}

void vi_destroy_HeapListEntry(
	HeapListEntry * this)
{
	assert(this != NULL);

	if(this->list->first == this)
		this->list->first = this->next;
	if(this->list->last == this)
		this->list->last = this->previous;

	if(this->previous)
		this->previous->next = this->next;
	if(this->next)
		this->next->previous = this->previous;


	this->previous = NULL;
	this->next = NULL;
	this->list = NULL;
	vi_destroy_Heap(&this->heap);
}


void vi_create_HeapList(
	HeapList * this)
{
	assert(this != 0);

	this->first = this->last = NULL;
}

void vi_destroy_HeapList(
	HeapList * this)
{
	assert(this != NULL);

	for(HeapListEntry * it = this->first; it != NULL;)
	{
		HeapListEntry * const next = it->next;

		vi_destroy_HeapListEntry(it);
		free(it);

		it = next;
	}
}

void * vi_alloc_HeapList(
	HeapList * this,
	size_t size)
{
	assert(this != NULL);
	if(!size)
		return NULL;


	for(HeapListEntry * it = this->first; it != NULL; it = it->next)
	{
		void * ret = vi_alloc_Heap(&it->heap, size);
		if(ret)
			return ret;
	}

	// add some maneuvering room for when the malloc has a wrong alignment.
	size_t capacity = size + sizeof(Entry) * 2;

	HeapListEntry * entry = (HeapListEntry *) malloc(sizeof(HeapListEntry));
	vi_create_HeapListEntry(
		entry,
		this,
		this->last,
		NULL,
		(this->min_capacity > capacity)
			? this->min_capacity
			: capacity);

	if(!this->first)
		this->first = entry;
	this->last = entry;

	return vi_alloc_Heap(&entry->heap, size);
}

void vi_free_block(
	void * block)
{
	assert(block != NULL);

	Entry * entry = vi_entry_of_block((uintptr_t)block);
	if(entry->previous)
		assert(entry->previous->next == entry);
	if(entry->next)
		assert(entry->next->previous == entry);
	Heap * heap = entry->heap;
	vi_destroy_Entry(entry);

	if(!heap->blocks)
	{
		HeapListEntry * listentry = vi_HeapListEntry_Heap(heap);
		vi_destroy_HeapListEntry(listentry);
		free(listentry);
	}
}