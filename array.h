#ifndef ARRAY_H
#define ARRAY_H

// 2 3 5 7 11 
// 3 7 31 127 2047 8191

#include <stdlib.h>

#include "prelude.h"

#define ARRAY_MINIMUM_CAPACITY 16

#define define_array_interface(type) \
	typedef struct { \
		type * arr; \
		size_t size; \
		size_t capacity; \
	} array_##type; \
	\
	array_##type array_##type##_new(); \
	void array_##type##_push(array_##type * a, type val); \
	void array_##type##_pop(array_##type * a);

#define define_array_implementation(type) \
	array_##type array_##type##_new(size_t capacity) { \
		array_##type a; \
		a.capacity = max(ARRAY_MINIMUM_CAPACITY, capacity); \
		a.arr = malloc(ARRAY_MINIMUM_CAPACITY * sizeof(type)); \
		memset(a.arr, 0, a.capacity * sizeof(type)); \
		a.size = 0; \
		return a; \
	} \
	\
	void array_##type##_push(array_##type * a, type val) { \
		a->arr[a->size++] = val; \
		if(a->size == a->capacity) { \
			a->capacity = a->capacity * 2; \
			a->arr = realloc(a->arr, a->capacity * sizeof(type)); \
			memset(a->arr + a->size, 0, a->size * sizeof(type)); \
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
		
	
//#define array(type) array_##type
#define array_at(a, i) a.arr[i]
#define array_rat(a, i) &(a.arr[i])

#endif
