#include "vec.h"

int vec_init(vec_t* vec, size_t init_capacity) {
	vec->data = malloc(init_capacity * sizeof(void*));
	if (!vec->data) return -1;

	vec->size = 0;
	vec->capacity = init_capacity;

	return 0;
}

int vec_length(vec_t* vec) {
	return vec->size;
}

void* vec_get(vec_t* vec, int index) {
	if (index >= 0 && index < vec->size) {
		return vec->data[index];
	}
	return NULL;
}

static void vec_resize(vec_t* vec, int capacity) {
	void** data = realloc(vec->data, vec->capacity * sizeof(void*));
	if (data) {
		vec->data = data;
		vec->capacity = capacity;
	}
}

void vec_push_back(vec_t* vec, void* item) {
	if (vec->size >= vec->capacity) {
		vec_resize(vec, vec->capacity * 2);
	}
	vec->data[vec->size++] = item;
}

void vec_pop(vec_t* vec, int index) {
	if (vec->size <= 0 || index >= vec->size) {
		return;
	}
	vec->data[index] = NULL;

	for (int i = index; i < vec->size - 1; i++) {
        vec->data[i] = vec->data[i + 1];
        vec->data[i + 1] = NULL;
    }
    vec->size--;

    if (vec->size > 0 && vec->size == vec->capacity / 4) {
        vec_resize(vec, vec->capacity / 2);
	}
}

void vec_free(vec_t* vec) {
	free(vec->data);
}
