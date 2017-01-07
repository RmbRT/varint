#include "malloc.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>

void vi_calloc(void ** ptr, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(*ptr == NULL);
	assert(typesize != 0);
	assert(count != 0);

	*ptr = calloc(count, typesize);
}
void vi_realloc(void ** ptr, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(typesize != 0);
	assert(count != 0);

	*ptr = realloc(*ptr, typesize * count);
}
void vi_free(void ** ptr)
{
	assert(ptr != NULL);
	assert(*ptr != NULL);
	free(*ptr);
	*ptr = NULL;
}
void vi_copy(void ** ptr, void const * src, size_t typesize, size_t count)
{
	assert(ptr != NULL);
	assert(*ptr == NULL);
	assert(src != NULL);
	assert(typesize != 0);
	assert(count != 0);

	vi_calloc(ptr, typesize, count);
	memcpy(*ptr, src, typesize * count);
}