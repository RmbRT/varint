#ifndef __varint_malloc_h_defined
#define __varint_malloc_h_defined


#include "defines.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void vi_set_default_heap_size(size_t capacity);


void vi_malloc(void ** ptr, size_t typesize, size_t count);
void vi_calloc(void ** ptr, size_t typesize, size_t count);
void vi_realloc(void ** ptr, size_t typesize, size_t count);
void vi_free(void ** ptr);
void vi_copy(void ** ptr, void const * src, size_t typesize, size_t count);

void vi_destroy_heap();

#ifdef __cplusplus
}
#endif

#endif