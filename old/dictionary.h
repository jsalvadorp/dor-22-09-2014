#ifndef DICTIONARY_H
#define DICTIONARY_H

#include "types.h"

typedef struct DictNode {
	struct DictNode * left;
	struct DictNode * right;
	
	Symbol name;
	Atom value;
} DictNode;



DictNode * DictNode_new(Symbol name, Atom value);
Atom * DictNode_search(DictNode * root, Symbol name);
Atom * DictNode_add(DictNode ** root, Symbol name, Atom value);
DictNode ** DictNode_searchToAdd(DictNode ** root, Symbol name);

void DictNode_destroy(DictNode * root);
void DictNode_map(DictNode * root, void (*fun) (DictNode * arg));

void DictNode_markGray(DictNode * this);

#endif
