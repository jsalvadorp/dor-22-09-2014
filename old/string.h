#ifndef STRING_H
#define STRING_H

#include "gc.h"
#include "forward.h"
#include <string.h>
#include <stdlib.h>

// returns a malloc'd NULL-terminated copy of the string, including
char * strdup(const char *str);





struct String {
	GCObject_HEAD
	size_t size;
	// the following is an array member of size at least 1 (for the NULL termination)
	// if at any point Strings are not null-terminated, C99 flexible array member may
	// be used in order to represent "" in less space.
	char str[1];
};

String * String_new(const char * s);
String * String_newn(const char * s, size_t count);

extern struct GCObject_vtbl_t String_vtbl;

Atom AString(String * c);
Atom AStr(const char * c);
size_t String_length(String * this);

void String_grayChildren(GCObject * this);
void String_destroy(GCObject * this);

char String_charAt(String * this, size_t i);
String * String_cat(String * a, String * b);
String * String_cat_Ss(String * a, const char * b);
String * String_cat_sS(const char * a, String * b);
String * String_cat_ss(const char * a, const char * b);

#define cstr(x) ((const char *) x->str)
#define Str(x) String_new(x)

#endif
