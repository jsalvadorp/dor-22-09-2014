#include "lambda.h"
#include "std.h"
#include <stdlib.h>
#include <assert.h>

// process a lambda list for evaluation (quote every parameter name)
Cons * foreval_lambda_list(Cons * llist) {
	if(llist) {
		if(llist->car.type == ConsType && llist->car.d._Cons) {
			return Cons_new(
				ACons(Cons_new(ALambda(listc), 
					  Cons_new(QUOTE(llist->car.d._Cons->car), Cons_next(llist->car.d._Cons))
				)), 
				
				foreval_lambda_list(Cons_next(llist))
			);
		} else if(llist->car.type == SymbolType) {
			// this is an untyped lambda list. no need to check for type annotations (:)
			
			return Cons_new(QUOTE(llist->car), foreval_lambda_list(Cons_next(llist)));
		} else assert(false);
	} else return NULL;
}


#define GETPNAME(x) (x.type == SymbolType \
						? x.d._Symbol \
					: (x.type == ConsType \
					&& x.d._Cons \
					&& x.d._Cons->car.type == SymbolType) \
						? x.d._Cons->car.d._Symbol \
					: (assert(false), symbol("BADBADBAD")))

Cons * search_symbol(Cons * l, Symbol s) {
	foreach(el, l) {
		if(el->car.type == SymbolType && Symbol_eq(s, el->car.d._Symbol))
			return el;
		else if(el->car.type == ConsType && el->car.d._Cons && el->car.d._Cons 
			&& el->car.d._Cons->car.type == SymbolType 
			&& Symbol_eq(el->car.d._Cons->car.d._Symbol, s))
			return el;
	}
	
	return NULL;
}

// put evaluated arguments from call in the same order as the param list (named arguments...)
Cons * args_in_form(Cons * param_list, Cons * args) {
	// DOES NOTHING YET
	Cons * new_argl = NULL, * arg = args;
	bool * given = NULL;
	
	foreach(param, param_list) {
		if(param->car.type == ConsType) {
			if(!param->car.d._Cons) { // nil param
				// ignore
				assert(arg);
				Cons_add(&new_argl, arg->car);
			} else if(!Cons_next(param->car.d._Cons)) { // rest param
				Cons_add(&new_argl, ACons(arg));
				break;
			} else { // param with default arg
				
			}
		} else {
			assert(param->car.type == SymbolType);
			
			Cons_add(&new_argl, arg->car);
		}
		
		arg = Cons_next(arg);
	}
	
	return args;
}

//bool let_to_lambda_list(Cons * letlist, Cons ** lparams, Cons ** largs);


void ini_lambda_env(Lambda * l, Environment * e) {
	if(l->name.s) Environment_define(e, l->name, ALambda(l));
}

bool params_args_to_env(Environment * target, Cons * params, Cons * args) {
	Cons * arg = args;
	Cons * param = params;
	
	// return curried/closure on insufficient arguments
	
	while(param) {
		if(!Atom_isNil(&param->car)) {
			if(param->car.type == ConsType && param->car.d._Cons->cdr.d._Cons ==  NULL) {
				Environment_define(target, GETPNAME(param->car.d._Cons->car), ACons(arg));
				arg = NULL;
				param = Cons_next(param);
				break;
			} else Environment_define(target, GETPNAME(param->car), arg->car);
		}
		arg = Cons_next(arg);
		param = Cons_next(param);
	}
	
	if(arg || param) // both should be NULL
		return false;
	return true;
}

Cons Lambda_CFUNC_NOPCHECK; // dont process params for this cfunc lambda
Cons Lambda_CFUNC_NAUGHTY;

Lambda * Lambda_new(Symbol name, Environment * parent, Cons * parameters, Cons * body) {
	Lambda * nl = malloc(sizeof(Lambda));
	nl->_GCObject_.vptr = &Lambda_vtbl;
	GCObject_INIT(nl)
	
	nl->parent = parent;
	nl->parameters = parameters;
	nl->body = body;
	nl->cimpl = NULL;
	nl->name = name;
	
	return nl;
}

Lambda * Lambda_new_c(Environment * parent, Cons * parameters, cfunc f) {
	Lambda * nl = malloc(sizeof(Lambda));
	nl->_GCObject_.vptr = &Lambda_vtbl;
	GCObject_INIT(nl)
	
	nl->parent = parent;
	nl->parameters = parameters;
	nl->body = NULL;
	nl->cimpl = f;
	nl->name = symbol(NULL);
	
	return nl;
}

Lambda * Lambda_new_c_simple(cfunc f) {
	Lambda * nl = malloc(sizeof(Lambda));
	nl->_GCObject_.vptr = &Lambda_vtbl;
	GCObject_INIT(nl)
	
	nl->parent = NULL;
	nl->parameters = &Lambda_CFUNC_NOPCHECK;
	nl->body = NULL;
	nl->cimpl = f;
	nl->name = symbol(NULL);
	
	return nl;
}

Lambda * Lambda_new_c_simple_naughty(cfunc f) {
	Lambda * nl = malloc(sizeof(Lambda));
	nl->_GCObject_.vptr = &Lambda_vtbl;
	GCObject_INIT(nl)
	
	nl->parent = NULL;
	nl->parameters = &Lambda_CFUNC_NOPCHECK;
	nl->body = &Lambda_CFUNC_NAUGHTY;
	nl->cimpl = f;
	nl->name = symbol(NULL);
	
	return nl;
}

Lambda * Lambda_new_c_simple_uncollected(cfunc f) {
	Lambda * nl = malloc(sizeof(Lambda));
	nl->_GCObject_.vptr = &Lambda_vtbl;
	
	nl->parent = NULL;
	nl->parameters = &Lambda_CFUNC_NOPCHECK;
	nl->body = NULL;
	nl->cimpl = f;
	nl->name = symbol(NULL);
	
	return nl;
}

void Lambda_grayChildren(GCObject * this) {
	Lambda * lam = (Lambda *) this;
	
	if(lam->body && lam->body != &Lambda_CFUNC_NAUGHTY) GCObject_markGray((GCObject *) lam->body);
	if(lam->parent) GCObject_markGray((GCObject *) lam->parent);
	if(lam->parameters && lam->parameters != &Lambda_CFUNC_NOPCHECK) GCObject_markGray((GCObject *) lam->parameters);
}

void Lambda_destroy(GCObject * this) {
	
}

struct GCObject_vtbl_t Lambda_vtbl = {Lambda_grayChildren, Lambda_destroy};

Atom ALambda(Lambda * l) {
	Atom a;
	zerofill(&a.d);
	a.type = LambdaType;
	a.d._Lambda = l;
	
	return a;
}
