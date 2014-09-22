#ifndef HMAP_H
#define HMAP_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "prelude.h"

#define HMAP_MINIMUM_CAPACITY 16

#define define_hmap_interface(ktype, vtype)	\
	struct hnode_##ktype##_##vtype;\
	typedef struct hnode_##ktype##_##vtype hnode_##ktype##_##vtype;\
	struct hnode_##ktype##_##vtype {\
		ktype key;\
		vtype value;\
		hnode_##ktype##_##vtype * next;\
	};\
	typedef struct {\
		hnode_##ktype##_##vtype ** arr;\
		size_t size;\
		size_t capacity;\
	} hmap_##ktype##_##vtype;\
	hmap_##ktype##_##vtype hmap_##ktype##_##vtype##_new(size_t capacity);\
	hnode_##ktype##_##vtype ** hmap_##ktype##_##vtype##_getptrptr(hmap_##ktype##_##vtype * m, ktype k);\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_put(hmap_##ktype##_##vtype * m, ktype k, vtype v);\
	vtype hmap_##ktype##_##vtype##_get(hmap_##ktype##_##vtype * m, ktype k);\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_grow(hmap_##ktype##_##vtype * m, hnode_##ktype##_##vtype * keeptrack);\
	void hmap_##ktype##_##vtype##_shrink(hmap_##ktype##_##vtype * m);\

#define define_hmap_implementation(ktype, vtype, fhash, fequals) \
	hmap_##ktype##_##vtype hmap_##ktype##_##vtype##_new(size_t capacity) {\
		hmap_##ktype##_##vtype a;\
		a.size = 0;\
		a.capacity = max(capacity, HMAP_MINIMUM_CAPACITY);\
		a.arr = malloc(a.capacity * sizeof(hnode_##ktype##_##vtype *));\
		memset(a.arr, 0, a.capacity * sizeof(hnode_##ktype##_##vtype *));\
		return a;\
	}\
	hnode_##ktype##_##vtype ** hmap_##ktype##_##vtype##_getptrptr(hmap_##ktype##_##vtype * m, ktype k) {\
		size_t hash = fhash(k) % m->capacity;\
		hnode_##ktype##_##vtype ** node = m->arr + hash;\
		while(*node && !fequals((*node)->key, k)) {\
			node = &((*node)->next);\
		}\
		return node;\
	}\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_getptr(hmap_##ktype##_##vtype * m, ktype k) {\
		size_t hash = fhash(k) % m->capacity;\
		hnode_##ktype##_##vtype * node = m->arr[hash];\
		while(node && !fequals((node)->key, k)) {\
			node = ((node)->next);\
		}\
		return node;\
	}\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_put(hmap_##ktype##_##vtype * m, ktype k, vtype v) {\
		hnode_##ktype##_##vtype ** node = hmap_##ktype##_##vtype##_getptrptr(m, k);\
		if((*node) == NULL) {\
			*node = malloc(sizeof(hnode_##ktype##_##vtype));\
			(*node)->key = k;\
			(*node)->next = NULL;\
		}\
		(*node)->value = v;\
		m->size++;\
		return hmap_##ktype##_##vtype##_grow(m, *node);\
	}\
	vtype hmap_##ktype##_##vtype##_get(hmap_##ktype##_##vtype * m, ktype k) {\
		return (*hmap_##ktype##_##vtype##_getptrptr(m, k))->value;\
	}\
	void hmap_##ktype##_##vtype##_remove(hmap_##ktype##_##vtype * m, ktype k) {\
		hnode_##ktype##_##vtype ** node = hmap_##ktype##_##vtype##_getptrptr(m, k), \
			*next = (*node)->next;\
		free(*node);\
		*node = next;\
		m->size--;\
		hmap_##ktype##_##vtype##_shrink(m);\
	}\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_grow(hmap_##ktype##_##vtype * m, hnode_##ktype##_##vtype * keeptrack) {\
		hnode_##ktype##_##vtype * pos = keeptrack;\
		if(m->size == m->capacity) {\
			hmap_##ktype##_##vtype newmap = hmap_##ktype##_##vtype##_new(m->capacity * 2);\
			for(int i = 0; i < m->capacity; i++) {\
				for(hnode_##ktype##_##vtype * node = m->arr[i], * tmp;node;tmp = node, node = node->next, free(tmp)) {\
					hnode_##ktype##_##vtype * r = hmap_##ktype##_##vtype##_put(&newmap, node->key, node->value);\
					if(r == keeptrack) pos = r;\
				}\
			}\
			free(m->arr);\
			m->arr = newmap.arr;\
			m->size = newmap.size;\
			m->capacity = newmap.capacity;\
		}\
		return pos;\
	}\
	void hmap_##ktype##_##vtype##_shrink(hmap_##ktype##_##vtype * m) {\
		if(m->size == m->capacity / 4 && m->capacity >= HMAP_MINIMUM_CAPACITY * 2) {\
			hmap_##ktype##_##vtype newmap = hmap_##ktype##_##vtype##_new(m->capacity / 2);\
			for(int i = 0; i < m->capacity; i++) {\
				for(hnode_##ktype##_##vtype * node = m->arr[i], * tmp;node;tmp = node, node = node->next, free(tmp)) {\
					hmap_##ktype##_##vtype##_put(&newmap, node->key, node->value);\
				}\
			}\
			free(m->arr);\
			m->arr = newmap.arr;\
			m->size = newmap.size;\
			m->capacity = newmap.capacity;\
		}\
	}
	
#endif
