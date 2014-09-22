#include <stdlib.h>
#include "gc.h"

GCRoot * root = NULL;

GCObject * whites = NULL;
GCObject * grays = NULL;
GCObject * blacks = NULL;

void delete(GCObject * this) {
	this->vptr->destroy(this); 
	free(this);
}

void GCObject_init(GCObject * this) {
	this->color = GC_white;
	this->next = NULL;
	this->prev = NULL;
	
	GC_consTo(this, &whites);
}

//1-11020387188

void GCObject_markGray(GCObject * this) {
	if(this->color == GC_white) {
		this->color = GC_gray;
		
		GC_consTo(this, &grays);
	}
}
	
void GCObject_markBlack(GCObject * this) {
	if(this->color == GC_gray) {
		this->color = GC_black;
		
		GC_consTo(this, &blacks);
		
		this->vptr->grayChildren(this);
	}
}

void GC_setup(GCRoot * r) {
	root = r;
}

void GC_mark() {
	root->grayChildren(root);
	
	while(grays) {
		GCObject_markBlack(grays);
	}
}

void GC_sweep() {
	GCObject * tmp;
	
	while(whites) {
		tmp = whites->next;
		
		delete(whites);
		whites = tmp;
		if(whites) whites->prev = NULL;
	}
	
	while(blacks) {
		blacks->color = GC_white;
		GC_consTo(blacks, &whites);
	}
}

void GC_shutdown() {
	GCObject * tmp;
	
	while(whites) {
		tmp = whites->next;
		
		delete(whites);
		whites = tmp;
	}
	
	while(grays) {
		tmp = grays->next;
		delete(grays);
		grays = tmp;
	}
	
	while(blacks) {
		tmp = blacks->next;
		delete(blacks);
		blacks = tmp;
	}
}

void GC_remove(GCObject * node) {
	if(node->next) node->next->prev = node->prev;
	if(node->prev) node->prev->next = node->next;
	
	if(node == whites) whites = node->next;
	else if(node == grays) grays = node->next;
	else if(node == blacks) blacks = node->next;
	
	node->next = NULL;
	node->prev = NULL;
}

void GC_consTo(GCObject * what, GCObject ** dest) {
	GC_remove(what);
	what->next = *dest;
	
	if(*dest) (*dest)->prev = what;
	*dest = what;
}
