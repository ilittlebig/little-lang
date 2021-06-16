#ifndef VEC_H
#define VEC_H

#include <stddef.h>
#include <stdlib.h>

typedef struct vec_t {
	void** data;
	size_t capacity;
	size_t size;
} vec_t;

int vec_init(vec_t*, size_t);
int vec_length(vec_t*);

void* vec_get(vec_t*, int);
static void vec_resize(vec_t*, int);
void vec_push_back(vec_t*, void*);
void vec_pop(vec_t*, int);
void vec_free(vec_t*);

#endif
