#include "string.h"
#include "types.h"
#include <assert.h>
#include <stdio.h>

char * strdup(const char *str) {
	size_t len = strlen(str);
	
	char * nstr = malloc(len + 1);
	// CHECK IF NSTR IS NULL (ALLOATION FAILED)
	strcpy(nstr, str);
	return nstr;
}

String * String_new(const char * s) {
	//puts("new string");
	size_t len = strlen(s);
	String * ns = malloc(sizeof(String) + len);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = len;
	strcpy(ns->str, s);
	
	return ns;
}

String * String_newn(const char * s, size_t count) {
	String * ns = malloc(sizeof(String) + count);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = count;
	strncpy(ns->str, s, count);
	ns->str[count] = '\0';
	
	return ns;
}

Atom AString(String * c) {
	Atom a;
	zerofill(&a.d);
	a.type = StringType;
	a.d._String = c;
	
	return a;
}

Atom AStr(const char * c) {
	Atom a;
	zerofill(&a.d);
	a.type = StringType;
	a.d._String = String_new(c);
	
	return a;
}

size_t String_length(String * this) {
	return this->size;
}

void String_grayChildren(GCObject * this) {
}

void String_destroy(GCObject * this) {
	//puts("deleted string");
}

char String_charAt(String * this, size_t i) {
	assert(i < this->size);
	return this->str[i];
}

String * String_cat(String * a, String * b) {
	size_t len = a->size + b->size;
	String * ns = malloc(sizeof(String) + len);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = len;
	strcpy(ns->str, a->str);
	strcpy(ns->str + a->size, b->str);
	
	return ns;
}

String * String_cat_Ss(String * a, const char * b) {
	size_t len = a->size + strlen(b);
	String * ns = malloc(sizeof(String) + len);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = len;
	strcpy(ns->str, a->str);
	strcpy(ns->str + a->size, b);
	
	return ns;
}

String * String_cat_sS(const char * a, String * b) {
	size_t lena = strlen(a);
	size_t len = lena + b->size;
	String * ns = malloc(sizeof(String) + len);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = len;
	strcpy(ns->str, a);
	strcpy(ns->str + lena, b->str);
	
	return ns;
}

String * String_cat_ss(const char * a, const char * b) {
	size_t lena = strlen(a);
	size_t len = lena + strlen(b);
	String * ns = malloc(sizeof(String) + len);
	ns->_GCObject_.vptr = &String_vtbl;
	GCObject_INIT(ns)
	
	ns->size = len;
	strcpy(ns->str, a);
	strcpy(ns->str + lena, b);
	
	return ns;
}

struct GCObject_vtbl_t String_vtbl = {String_grayChildren, String_destroy};
