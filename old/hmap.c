#define define_hmap_interface(ktype, vtype)	\
	struct hnode_##ktype##_##vtype;\
	typedef struct hnode_##ktype##_##vtype hnode_##ktype##_##vtype;\
	struct hnode_##ktype##_##vtype {\
		string key;\
		int value;\
		hnode_##ktype##_##vtype * next;\
	};\
	typedef struct {\
		hnode_##ktype##_##vtype ** arr;\
		size_t size;\
		size_t capacity;\
	} hmap_##ktype##_##vtype;\
	hmap_##ktype##_##vtype hmap_##ktype##_##vtype##_new(size_t capacity);\
	hnode_##ktype##_##vtype ** hmap_##ktype##_##vtype##_getptrptr(hmap_##ktype##_##vtype * m, string k);\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_put(hmap_##ktype##_##vtype * m, string k, int v);\
	int hmap_##ktype##_##vtype##_get(hmap_##ktype##_##vtype * m, string k);\
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
	hnode_##ktype##_##vtype ** hmap_##ktype##_##vtype##_getptrptr(hmap_##ktype##_##vtype * m, string k) {\
		size_t hash = fhash(k) % m->capacity;\
		hnode_##ktype##_##vtype ** node = m->arr + hash;\
		while(*node && !fequals((*node)->key, k)) {\
			node = &((*node)->next);\
		}\
		return node;\
	}\
	hnode_##ktype##_##vtype * hmap_##ktype##_##vtype##_put(hmap_##ktype##_##vtype * m, string k, int v) {\
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
	int hmap_##ktype##_##vtype##_get(hmap_##ktype##_##vtype * m, string k) {\
		return (*hmap_##ktype##_##vtype##_getptrptr(m, k))->value;\
	}\
	void hmap_##ktype##_##vtype##_remove(hmap_##ktype##_##vtype * m, string k) {\
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
	
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define HMAP_MINIMUM_CAPACITY 8192

#define HASH_START 5381

int growths, collisions;

/**/
unsigned long fhash(const char * s) { // larson
	unsigned long hash = HASH_START;
	
	while(*s) {
		hash = hash * 101 + ((unsigned char) *s);
		s++;
	}
	
	return hash;
}
/**/


/*
	unsigned long
    fhash(const char *str)
    {
        unsigned long hash = 5381;
        int c;

        while (c = (unsigned char)*str++)
            hash = ((hash << 5) + hash) + c; 
            // hash * 33 + c 

        return hash;
    }
/**/

/*
	unsigned long
    fhash(const char *str)
    {
        unsigned long hash = 0;
        int c;

        while (c = (unsigned char)*str++)
            hash = c + (hash << 6) + (hash << 16) - hash;
            //hash(i) = hash(i - 1) * 65599 + str[i];

        return hash;
    }
/**/

int fequals(const char * s1, const char * s2) {
	return !strcmp(s1, s2);
}


char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}

typedef const char * string;

define_hmap_interface(string, int);
define_hmap_implementation(string, int, fhash, fequals);

void mput(hmap_string_int * m, string k) {
	hmap_string_int_put(m, strdup(k), 0);
}

char buf[1000];

int main() {
	hmap_string_int mapa = hmap_string_int_new(0);
	
	int p = 0;
	
	while(scanf("%s", buf) != EOF) {
		//printf("%s\n", buf);
		mput(&mapa, buf);
		p++;
	}
	
	
	printf("g %d c %d p %d\n", growths, collisions, p);
}

#if 0

#ifndef AUTOARR_H
#define AUTOARR_H

// 2 3 5 7 11 
// 3 7 31 127 2047 8191

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define max(a, b) ((a) > (b) ? (a) : (b))

#define HMAP_MINIMUM_CAPACITY 8192

#define HASH_START 5381

int growths, collisions;

/**/
unsigned long fhash(const char * s) { // larson
	unsigned long hash = HASH_START;
	
	while(*s) {
		hash = hash * 101 + ((unsigned char) *s);
		s++;
	}
	
	return hash;
}
/**/


/*
	unsigned long
    fhash(const char *str)
    {
        unsigned long hash = 5381;
        int c;

        while (c = (unsigned char)*str++)
            hash = ((hash << 5) + hash) + c; 
            // hash * 33 + c 

        return hash;
    }
/**/

/*
	unsigned long
    fhash(const char *str)
    {
        unsigned long hash = 0;
        int c;

        while (c = (unsigned char)*str++)
            hash = c + (hash << 6) + (hash << 16) - hash;
            //hash(i) = hash(i - 1) * 65599 + str[i];

        return hash;
    }
/**/

int fequals(const char * s1, const char * s2) {
	return !strcmp(s1, s2);
}

typedef struct {} empty;
typedef const char * string;

	struct hnode_string_int;
	typedef struct hnode_string_int hnode_string_int;
	struct hnode_string_int {
		string key;
		int value;
		hnode_string_int * next;
	};
	typedef struct { 
		hnode_string_int ** arr;
		size_t size;
		size_t capacity;
	} hmap_string_int; 
	
	hmap_string_int hmap_string_int_new(size_t capacity) { 
		hmap_string_int a;
		a.size = 0;
		a.capacity = max(capacity, HMAP_MINIMUM_CAPACITY);
		a.arr = malloc(a.capacity * sizeof(hnode_string_int *));
		//printf("acc %d %p \n", a.capacity, a.arr);
		memset(a.arr, 0, a.capacity * sizeof(hnode_string_int *));
		return a; 
	} 
	
	hnode_string_int ** hmap_string_int_getptrptr(hmap_string_int * m, string k) { 
		size_t hash = fhash(k) % m->capacity; /*power of two! do something*/ 
		
		hnode_string_int ** node = m->arr + hash;
		
		//printf("++ %p %d h %d %s r %p\n", node, (int)(node - m->arr), hash, k, *node);
		
		while(*node && !fequals((*node)->key, k)) {
			//fputs("collision ", stderr);
			//collisions++;
			node = &((*node)->next);
		}
		
		//printf("-- %p %d h %d %s r %p\n", node, (int)(node - m->arr), hash, k, *node);
		
		return node;
	} 
	
	hnode_string_int * hmap_string_int_grow(hmap_string_int * m, hnode_string_int * n);
	void hmap_string_int_shrink(hmap_string_int * m);
	
	hnode_string_int * hmap_string_int_put(hmap_string_int * m, string k, int v) { 
		hnode_string_int ** node = hmap_string_int_getptrptr(m, k);
		
		
		if((*node) == NULL) {
			if(!(node >= m->arr && node < m->arr + m->capacity)) collisions++;
			
			*node = malloc(sizeof(hnode_string_int));
			memset(*node, 0, sizeof(hnode_string_int));
			//printf("nodex %p %p %p %zu\n", node, *node, &((*node)->next), sizeof(hnode_string_int));
			assert(*node);
			(*node)->key = k;
			(*node)->value = v;
			//printf("node1 %p\n", *node);
			assert(*node);
		} else {
			
			
			(*node)->value = v;
		}
		//printf("node2 %p\n", *node);
		
		assert(*node);
		
		//(*node)->value = v;
		m->size++;
		
		return hmap_string_int_grow(m, *node);
	} 
	
	int hmap_string_int_get(hmap_string_int * m, string k) { 
		return (*hmap_string_int_getptrptr(m, k))->value;
	} 
	
	void hmap_string_int_remove(hmap_string_int * m, string k) { 
		hnode_string_int ** node = hmap_string_int_getptrptr(m, k),
			*next = (*node)->next;
		
		free(*node);
		*node = next;
		m->size--;
		
		hmap_string_int_shrink(m);
	}
	
	hnode_string_int * hmap_string_int_grow(hmap_string_int * m, hnode_string_int * keeptrack) { 
		//puts("grow");
		
		hnode_string_int * pos = keeptrack;
		
		if(m->size == m->capacity) {
			//fputs("grow", stderr);
			growths++;
			hmap_string_int newmap = hmap_string_int_new(m->capacity * 2);
			
			for(int i = 0; i < m->capacity; i++) {
				for(hnode_string_int * node = m->arr[i], * tmp; node; tmp = node, node = node->next, free(tmp)) {
					hnode_string_int * r = hmap_string_int_put(&newmap, node->key, node->value);
					if(r == keeptrack) pos = r;
				}
			}
			
			free(m->arr);
			
			m->arr = newmap.arr;
			m->size = newmap.size;
			m->capacity = newmap.capacity;
		}
		
		return pos;
	}
	
	void hmap_string_int_shrink(hmap_string_int * m) { 
		if(m->size == m->capacity / 4 && m->capacity >= HMAP_MINIMUM_CAPACITY * 2) {
			hmap_string_int newmap = hmap_string_int_new(m->capacity / 2);
			
			for(int i = 0; i < m->capacity; i++) {
				for(hnode_string_int * node = m->arr[i], * tmp; node; tmp = node, node = node->next, free(tmp)) {
					hmap_string_int_put(&newmap, node->key, node->value);
				}
			}
			
			free(m->arr);
			
			m->arr = newmap.arr;
			m->size = newmap.size;
			m->capacity = newmap.capacity;
		}
	} 

char *strdup (const char *s) {
    char *d = malloc (strlen (s) + 1);   // Space for length plus nul
    if (d == NULL) return NULL;          // No memory
    strcpy (d,s);                        // Copy the characters
    return d;                            // Return the new string
}

void mput(hmap_string_int * m, string k) {
	hmap_string_int_put(m, strdup(k), 0);
}

char buf[1000];

int main() {
	hmap_string_int mapa = hmap_string_int_new(0);
	
	int p = 0;
	
	while(scanf("%s", buf) != EOF) {
		//printf("%s\n", buf);
		mput(&mapa, buf);
		p++;
	}
	
	for(int i = 0; i < 200000; i++
	
	printf("g %d c %d p %d\n", growths, collisions, p);
}	

#endif

#endif

#if 0

// 2 3 5 7 11 
// 3 7 31 127 2047 8191

#include <stdlib.h>

#define HMAP_MINIMUM_CAPACITY 16

#define define_hmap(ktype, vtype, fhash, fequals) \
	struct hnode_##ktype##_##vtype; \
	typedef struct hnode_##ktype##_##vtype hnode_##ktype##_##vtype; \
	struct hnode_##ktype##_##vtype { \
		ktype key; \
		vtype value; \
		hnode_##ktype##_##vtype * next; \
	}; \
	typedef struct { \
		hnode_##ktype##_##vtype ** arr; \
		size_t size; \
		size_t capacity; \
	} hmap_##ktype##_##vtype; \
	\
	hmap_##ktype##_##vtype hmap_##ktype##_##vtype##_new(size_t capacity) { \
		hmap_##ktype##_##vtype a; \
		a.size = 0; \
		a.capacity = capacity; \
		a.arr = malloc(sizeof(capacity * sizeof(hnode_##ktype##_##vtype *))); \
		memset(a.arr, 0, sizeof(capacity * sizeof(hnode_##ktype##_##vtype *)); \
		return a; \
	} \
	\
	hnode_##ktype##_##vtype ** hmap_##ktype##_##vtype##_getptrptr(hmap_##ktype##_##vtype * m, ktype k) { \
		size_t hash = fhash(k) % m->capacity; /*power of two! do something*/ \
		\
		hnode_##ktype##_##vtype ** node = &(m->arr[hash]); \
		\
		while(*node && !fequals((*node)->key, k)) node = &((*node)->next); \
		\
		return node; \
	} \
	\
	void hmap_##ktype##_##vtype##_grow(hmap_##ktype##_##vtype * m); \
	void hmap_##ktype##_##vtype##_shrink(hmap_##ktype##_##vtype * m); \
	\
	void hmap_##ktype##_##vtype##_put(hmap_##ktype##_##vtype * m, ktype k, vtype v) { \
		hnode_##ktype##_##vtype ** node = hmap_##ktype##_##vtype##_getptrptr(m, k); \
		\
		if(!(*node)) { \
			*node = malloc(sizeof(hnode_##ktype##_##vtype)); \
			(*node)->key = k; \
			(*node)->next = NULL; \
		} \
		\
		(*node)->value = v; \
		m->size++; \
		hmap_##ktype##_##vtype##_grow(m); \
	} \
	\
	vtype array_##ktype##_##vtype##_get(hmap_##ktype##_##vtype * m, ktype k) { \
		return (*hmap_##ktype##_##vtype##_getptrptr(m, k))->value; \
	} \
	\
	void array_##ktype##_##vtype##_remove(hmap_##ktype##_##vtype * m, ktype k) { \
		hnode_##ktype##_##vtype ** node = hmap_##ktype##_##vtype##_getptrptr(m, k), \
			*next = (*node)->next; \
		\
		free(*node); \
		*node = next; \
		m->size--; \
		hmap_##ktype##_##vtype##_shrink(m); \
	} \
	\
	void hmap_##ktype##_##vtype##_grow(hmap_##ktype##_##vtype * m) { \
		if(m->size == m->capacity) { \
			hmap_##ktype##_##vtype newmap = hmap_##ktype##_##vtype##_new(m->capacity * 2); \
			\
			for(int i = 0; i < m->capacity; i++) { \
				for(hnode_##ktype##_##vtype * node = m->arr[i], * tmp; node; tmp = node, node = node->next, free(tmp)) { \
					hmap_##ktype##_##vtype##_put(&newmap, node->key, node->value); \
				} \
			}
			
			free(m->arr);
			
			m->arr = newmap.arr;
			m->size = newmap.size;
			m->capacity = newmap.capacity;
		}
	}
	
	void hmap_##ktype##_##vtype##_shrink(hmap_##ktype##_##vtype * m) { 
		if(m->size == m->capacity / 4 && m->capacity >= HMAP_MINIMUM_CAPACITY * 2) {
			hmap_##ktype##_##vtype newmap = hmap_##ktype##_##vtype##_new(m->capacity / 2);
			
			for(int i = 0; i < m->capacity; i++) {
				for(hnode_##ktype##_##vtype * node = m->arr[i], * tmp; node; tmp = node, node = node->next, free(tmp)) {
					hmap_##ktype##_##vtype##_put(&newmap, node->key, node->value);
				}
			}
			
			free(m->arr);
			
			m->arr = newmap.arr;
			m->size = newmap.size;
			m->capacity = newmap.capacity;
		}
	} 
	
#define hmap(ktype, vtype) hmap_##ktype##_##vtype

#endif
