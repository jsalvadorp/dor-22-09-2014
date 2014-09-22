#include "dictionary.h"

#include "types.h"


DictNode * DictNode_new(Symbol name, Atom value) {
	DictNode * newn = malloc(sizeof(DictNode));
	
	newn->left = NULL;
	newn->right = NULL;
	
	newn->name = name;
	newn->value = value;
	
	return newn;
}

Atom * DictNode_search(DictNode * node, Symbol name) {
	while (node) {
		if (Symbol_gt(name, node->name)) 
			node = node->right;
		else if (Symbol_lt(name, node->name)) 
			node = node->left;
		else break;
	}
	
	if(node) return &(node->value);
	else return NULL;
}

Atom * DictNode_add(DictNode ** node, Symbol name, Atom value) {
	while (*node) {
		if (Symbol_gt(name, (*node)->name)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->name)) 
			node = &((*node)->left);
		else break;
	}
	
	if(!(*node)) *node = DictNode_new(name, value);
	
	return &((*node)->value);
}

DictNode ** DictNode_searchToAdd(DictNode ** node, Symbol name) {
	while (*node) {
		if (Symbol_gt(name, (*node)->name)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->name)) 
			node = &((*node)->left);
		else break;
	}
	
	return node;
}

void DictNode_destroy(DictNode * this) {
	if(this->left) {
		DictNode_destroy(this->left);
		free(this->left);
	}
	
	if(this->right) {
		DictNode_destroy(this->right);
		free(this->right);
	}
}

void DictNode_map(DictNode * root, void (*fun) (DictNode * arg)) {
	if(root) {
		fun(root);
		
		DictNode_map(root->left, fun);
		DictNode_map(root->right, fun);
	}
}

void DictNode_markGray(DictNode * this) {
	Atom_markGray(&(this->value));
}

