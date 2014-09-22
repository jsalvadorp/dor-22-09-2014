#ifndef ANALYSIS_H
#define ANALYSIS_H

#include "symbol.h"
#include "hmap.h"
#include "ast.h"

typedef struct {
	kind_t * from;
	kind_t * to;
} kind_t;

#define K0 (&k0)

extern kind_t k0;

typedef struct {
	int tag;
	int size;
	int compactsize;
	kind_t * kind;
	//bool heap 
} type_t;

typedef struct {
	type_t t;
	
	qtype_t * q;
	int id;
} tvar_t;

typedef struct {
	type_t t;
	
	type_t left;
	type_t right;
} tapp_t;

typedef struct {
	type_t t;
	
	tvar_t var;
	type_t * body;
} qtype_t;

typedef struct {
	int n;
	type_t ** nt;
} typel_t;

// for each compiled function, show up to which parameter it is compiled

typedef struct {
	type_t t;
	
	type_t * takes;
	type_t * rets;
} funct_t;
#define FUNCT 0

typedef struct {
	type_t t;
	
	int * cnsizes;
	int * cnconsts;
	
	int c0; // number of nullary constructors
	int cn;
} adt_t;
#define ADT 1

typedef struct {
	type_t t;
	int cs;
	
	typel_t els;
} tuple_t;
#define TUPLE 2

typedef struct {
	type_t t;
	
	type_t * pointed;
} tvar_t;
#define TVAR 99

typedef struct {
	type_t t;
	
	tenv_t * env;
	type_t * exp;
} quantified_t;
#define QUANT 100

typedef type_t * typeptr;

define_hmap_interface(Symbol, typeptr);

typedef struct tenv {
	hmap_Symbol_typeptr dict;
	struct tenv * parent;
} tenv_t;

type_t * funct(type_t * takes, type_t * rets);

type_t * cfunct(tenv_t * env, ASTList * body);
type_t * cadt(tenv_t * env, ASTList * body);
type_t * ctuple(tenv_t * env, ASTList * body);
type_t * cpmap(tenv_t * env, ASTList * body);

extern kind_t * kind0;
extern tenv_t * globalt;

#endif
