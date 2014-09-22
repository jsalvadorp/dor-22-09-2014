#ifndef LAMBDA_H
#define LAMBDA_H

#include "forward.h"
#include "types.h"
#include "cons.h"
#include "environment.h"

// process a lambda list for evaluation (quote every parameter name)
Cons * foreval_lambda_list(Cons * llist);

// put arguments from call in the same order as the param list (named arguments...)
Cons * args_in_form(Cons * params, Cons * args);
//bool let_to_lambda_list(Cons * letlist, Cons ** lparams, Cons ** largs);

void ini_lambda_env(Lambda * l, Environment * e);
bool params_args_to_env(Environment * target, Cons * params, Cons * args);

typedef Atom (*cfunc) (Cons *);

struct Lambda {
	GCObject_HEAD
	
	Environment * parent;
	
	Cons * parameters;
	Cons * body;
	cfunc cimpl;
	
	Symbol name; // for the named lambda 
};

// on a compiled program, lambdas should have a part that is the same for all 'instances' of the lambda (including the implementation, parameter names, lambda name, etc) and a part that varies with each instance: the closure, the pointer to the surrounding environment/stack frame.

extern struct GCObject_vtbl_t Lambda_vtbl;

extern Cons Lambda_CFUNC_NOPCHECK; // dont process params for this cfunc lambda
extern Cons Lambda_CFUNC_NAUGHTY; // when called, pass a hidden argument with the evaluator

Lambda * Lambda_new(Symbol name, Environment * parent, Cons * parameters, Cons * body);
Lambda * Lambda_new_c(Environment * parent, Cons * parameters, cfunc f);
Lambda * Lambda_new_c_simple(cfunc f);
Lambda * Lambda_new_c_simple_naughty(cfunc f);

Lambda * Lambda_new_c_simple_uncollected(cfunc f);

void Lambda_grayChildren(GCObject * this);
void Lambda_destroy(GCObject * this);

Atom ALambda(Lambda * l);

#endif
