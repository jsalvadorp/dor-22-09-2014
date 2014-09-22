#ifndef AUTOARR_H
#define AUTOARR_H

// 2 3 5 7 11 
// 3 7 31 127 2047 8191

#include <stdlib.h>

#define HASHTABLE_MINIMUM_CAPACITY 8

#define define_array(type) \
	typedef struct { \
		type * arr; \
		size_t size; \
		size_t capacity; \
	} array_##type; \
	\
	array_##type array_##type##_new() { \
		array_##type a; \
		a.arr = malloc(ARRAY_MINIMUM_CAPACITY * sizeof(type)); \
		a.size = 0; \
		a.capacity = ARRAY_MINIMUM_CAPACITY; \
		return a; \
	} \
	\
	void array_##type##_push(array_##type * a, type val) { \
		a->arr[a->size++] = val; \
		if(a->size == a->capacity) { \
			a->capacity = a->capacity * 2; \
			a->arr = realloc(a->arr, a->capacity * sizeof(type)); \
		} \
	} \
	\
	void array_##type##_pop(array_##type * a) { \
		a->size--; \
		if(a->size + 1 == a->capacity / 4 && a->capacity >= ARRAY_MINIMUM_CAPACITY * 2) { \
			a->capacity = a->capacity / 2; \
			a->arr = realloc(a->arr, a->capacity * sizeof(type)); \
		} \
	}
	
#define array(type) array_##type
#define array_at(a, i) a.arr[i]

#endif

