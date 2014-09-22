#include <string.h>
#include <stdlib.h>
#include "symbol.h"

#define HASH_START 5381


unsigned long cstring_hash(const char * s) { // larson
	unsigned long hash = HASH_START;
	
	while(*s) {
		hash = hash * 101 + ((unsigned char) *s);
		s++;
	}
	
	return hash;
}

int cstring_eq(const char * s1, const char * s2) {
	return !strcmp(s1, s2);
}

char * strdup (const char *s) {
	char * buf = malloc(strlen (s) + 1);
	strcpy(buf, s);
	return buf;
}

define_hmap_implementation(cstring, hash_t, cstring_hash, cstring_eq);

hmap_cstring_hash_t table;


Symbol symbol(const char * s) {
	Symbol sym;
	sym.s = hmap_cstring_hash_t_put(&table, strdup(s), cstring_hash(s));
	return sym;
}

bool Symbol_eq(Symbol a, Symbol b) {
	return a.s == b.s;
}

bool Symbol_ne(Symbol a, Symbol b) {
	return a.s != b.s;
}

bool Symbol_lt(Symbol a, Symbol b) {
	return a.s < b.s;
}

bool Symbol_gt(Symbol a, Symbol b) {
	return a.s > b.s;
}

bool Symbol_le(Symbol a, Symbol b) {
	return a.s <= b.s;
}

bool Symbol_ge(Symbol a, Symbol b) {
	return a.s >= b.s;
}

void init_symbols() {
	table = hmap_cstring_hash_t_new(8192);
}

cstring Symbol_getstr(Symbol s) {
	return s.s->key;
}


hash_t Symbol_hash(Symbol s) {
	return s.s->value;
}
