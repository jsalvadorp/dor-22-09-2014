#ifndef A3
#define A3

#include <stdlib.h>
#include "symbol.h"
#include "ast.h"
#include "hmap.h"
#include "array.h"

#include "typing.h"

// variables are disjoint set forest nodes, possibly bound to a quantification


define_array_interface(Symbol)


typedef enum {
	GLOBAL, PARAMETER, LOCAL, CLOSURE, RECUR // recur should just be a global binding!! problematic for closured lambdas
} scope_t;

typedef struct {
	type_t * type;
	scope_t scope;
	int id;
	
	Symbol name;
	
	int hasvalue;
	anydata value;
	//int arity;
} binding_t;

typedef binding_t * binding_ptr;


define_array_interface(binding_ptr)

define_hmap_interface(Symbol, binding_t);

typedef struct env_t {
	int tag;
	struct env_t * parent;
	hmap_Symbol_binding_t dict;
} env_t;

binding_t * getb(env_t *, Symbol name);



#endif
