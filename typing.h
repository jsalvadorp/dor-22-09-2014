#ifndef TYPING_H
#define TYPING_H

#include "prelude.h"

struct type_t;
typedef struct type_t type_t;
struct tvar_t;
typedef struct tvar_t tvar_t;
struct kind_t;
typedef struct kind_t kind_t;

struct kind_t {
	kind_t * from;
	kind_t * to;
};

extern kind_t _StarKind, _VoidKind;
#define StarKind &_StarKind
#define VoidKind &_VoidKind

struct type_t {
	int tag;
	
	kind_t * kind;
	
	int compactsize;
};

struct tvar_t {
	type_t t;
	
	// substitution, actual type, constraints...
	type_t * subst;
	const char * name;
	
	// quantification binding and id
	bool bound; // quantification
	
	struct tvar_t * dsf_parent;
	int dsf_rank;
};

struct tapp_t {
	type_t t;
	
	type_t * left;
	type_t * right;
};

typedef struct tapp_t tapp_t;

#define TYPE_TYPE 0
#define TYPE_TAPP 1
#define TYPE_TVAR 2

tvar_t * tvar_new();
tvar_t * tvar_getrep(tvar_t *);
bool tvar_unite(tvar_t * a, tvar_t * b);

extern type_t * type_int, * type_bool, * type_void, * type_func;

void type_init();
type_t * functype_new(type_t * from, type_t * to);
type_t * functype_from(type_t * fun);
type_t * functype_to(type_t * fun);

bool type_unite(type_t ** a, type_t ** b);
bool type_unite_app(type_t **fun, type_t **arg, type_t **ret);

#endif
