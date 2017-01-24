#ifndef __varint_heap_h_defined
#define __varint_heap_h_defined

#include <inttypes.h>
#include <stddef.h>

typedef struct Entry Entry;
typedef struct Heap Heap;
typedef struct HeapListEntry HeapListEntry;
typedef struct HeapList HeapList;

struct Entry
{
	/** The heap this entry belongs to. */
	Heap * heap;
	/** The next heap entry. */
	Entry * next;
	/** The previous heap entry. */
	Entry * previous;
	/** The size this entry has reserved. */
	size_t reserved;
};

uintptr_t vi_end_Entry(
	Entry * this);
uintptr_t vi_begin_Entry(
	Entry * this);
Entry * vi_entry_of_block(
	uintptr_t block);
size_t vi_hole_Entry(
	Entry * this);
void vi_create_Entry(
	Entry * this,
	Heap * heap,
	Entry * previous,
	Entry * next,
	size_t reserved);
void vi_destroy_Entry(
	Entry * this);

struct Heap
{
	/** The starting address. */
	uintptr_t start;
	/** The end address. */
	uintptr_t end;

	/** The first allocated entry. */
	Entry * first;
	/** The last allocated entry.*/
	Entry * last;

	/** How many blocks this heap has. */
	size_t blocks;
};

void vi_create_Heap(
	Heap * this,
	size_t capacity);

void vi_destroy_Heap(
	Heap * this);

void * vi_alloc_Heap(
	Heap * heap,
	size_t size);

HeapListEntry * vi_HeapListEntry_Heap(
	Heap * this);


struct HeapListEntry
{
	/** The heap list this entry belongs to. */
	HeapList * list;
	/** The previous heap list entry. */
	HeapListEntry * previous;
	/** The next heap list entry. */
	HeapListEntry * next;

	/** This entry's heap*/
	Heap heap;
};

void vi_create_HeapListEntry(
	HeapListEntry * this,
	HeapList * list,
	HeapListEntry * prev,
	HeapListEntry * next,
	size_t capacity);
void vi_destroy_HeapListEntry(
	HeapListEntry * this);


struct HeapList
{
	/** The first heap list entry. */
	HeapListEntry * first;
	/** The last heap list entry. */
	HeapListEntry * last;

	/** The minimal capacity a heap should have. */
	size_t min_capacity;
};

void vi_create_HeapList(
	HeapList * this);
void vi_destroy_HeapList(
	HeapList * this);

void * vi_alloc_HeapList(
	HeapList * this,
	size_t capcity);

void vi_free_block(
	void * block);

#endif