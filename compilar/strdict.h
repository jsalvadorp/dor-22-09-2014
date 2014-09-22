#ifndef STRDIC_H
#define STRDIC_H

#include <string.h>
#include <stdlib.h>
#include "vm.h"

char * strdup(const char *str);

struct strdict;
typedef struct strdict strdict;

struct strdict {
	const char * key;
	
	word_t value;
	
	strdict * left;
	strdict * right;
};

strdict * strdict_insert(strdict ** d, const char * key, word_t value);
strdict ** strdict_find(strdict ** d, const char * key);
word_t strdict_get(strdict * d, const char * key);
strdict * strdict_new(const char * key, word_t value, strdict * left, strdict * right);

#endif
