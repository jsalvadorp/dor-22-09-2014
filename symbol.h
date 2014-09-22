
#ifndef SYMBOL_H
#define SYMBOL_H

#include "hmap.h"
#include "array.h"
#include "prelude.h"

typedef const char * cstring;

typedef unsigned long hash_t;


define_hmap_interface(cstring, hash_t);

typedef struct {
	hnode_cstring_hash_t * s;
} Symbol;

Symbol symbol(const char * s);

bool Symbol_eq(Symbol, Symbol);
bool Symbol_ne(Symbol, Symbol);
bool Symbol_lt(Symbol, Symbol);
bool Symbol_gt(Symbol, Symbol);
bool Symbol_le(Symbol, Symbol);
bool Symbol_ge(Symbol, Symbol);

cstring Symbol_getstr(Symbol s);

hash_t Symbol_hash(Symbol s);

void init_symbols();

#endif
