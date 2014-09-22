#include <stdlib.h>
#include "strdict.h"

char * strdup(const char *str) {
	size_t len = strlen(str);
	
	char * nstr = malloc(len + 1);
	// CHECK IF NSTR IS NULL (ALLOATION FAILED)
	strcpy(nstr, str);
	return nstr;
}

strdict ** strdict_find(strdict ** d, const char * key) {
	if(*d == NULL || !strcmp((*d)->key, key)) return d;
	else if(strcmp(key, (*d)->key) > 0) return strdict_find(&((*d)->right), key);
	else return strdict_find(&((*d)->left), key);
}

strdict * strdict_insert(strdict ** d, const char * key, word_t value) {
	d = strdict_find(d, key);
	
	if(*d)
		(*d)->value = value;
	else
		*d = strdict_new(key, value, NULL, NULL);
	
	return *d;
}

word_t strdict_get(strdict * d, const char * key) {
	if(!strcmp(key, d->key)) return d->value;
	else if(strcmp(key, d->key) > 0) return strdict_get(d->right, key);
	else return strdict_get(d->left, key);
}

strdict * strdict_new(const char * key, word_t value, strdict * left, strdict * right) {
	strdict * n = malloc(sizeof(strdict));
	
	n->value = value;
	n->key = strdup(key);
	n->left = left;
	n->right = right;
	
	return n;
}

