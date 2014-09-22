#include "std.h"

#include "environment.h"
#include "lambda.h"
#include "cons.h"
#include "eval.h"
#include "string.h"
#include "read.h"
#include "infix.h"
#include "stream.h"

#include <stdio.h>

#define NUM_HIERARCHY(type) ( type == DoubleType ? 0 \
							: type == IntType ? 1 \
							: type == CharType ? 2 \
							: -42)

#define hitype(a, b) (NUM_HIERARCHY(a) < NUM_HIERARCHY(b) ? a : b)

#define BI_N_OP(op) ;\
	Atom a = *Cons_at(args, 0), \
		 b = *Cons_at(args, 1); \
	\
	assert(NUM_HIERARCHY(a.type) >= 0 && NUM_HIERARCHY(a.type) >= 0); \
	\
	Type * higher = hitype(a.type, b.type); \
		\
	if(higher == DoubleType) \
		return Adouble(Atom_as_double(&a) op Atom_as_double(&b)); \
	else if(higher == IntType) \
		return Along(Atom_as_long(&a) op Atom_as_long(&b)); \
	else if(higher == CharType) \
		return Achar(Atom_as_char(&a) op Atom_as_char(&b));

#define BI_N_BOOL_OP(op) ;\
	assert(NUM_HIERARCHY(a.type) >= 0 && NUM_HIERARCHY(a.type) >= 0); \
	\
	Type * higher = hitype(a.type, b.type); \
		\
	if(higher == DoubleType) \
		return Abool(Atom_as_double(&a) op Atom_as_double(&b)); \
	else if(higher == IntType) \
		return Abool(Atom_as_long(&a) op Atom_as_long(&b)); \
	else if(higher == CharType) \
		return Abool(Atom_as_char(&a) op Atom_as_char(&b));

Module * std = NULL;

Atom error(Cons * args) {
	printf("Error: %s\n", cstr(args->car.d._String));
	assert(false);
	return nil;
}

Atom add(Cons * args) {
	if(args == NULL) return nil;
		
	Atom sum = Achar(0);
	
	foreach(arg, args) {
		assert(NUM_HIERARCHY(arg->car.type) >= 0 && NUM_HIERARCHY(sum.type) >= 0); // both numerical
		Type * higher = hitype(arg->car.type, sum.type);
		
		if(higher == DoubleType)
			sum = Adouble(Atom_as_double(&arg->car) + Atom_as_double(&sum));
		else if(higher == IntType)
			sum = Along(Atom_as_long(&arg->car) + Atom_as_long(&sum));
		else if(higher == CharType)
			sum = Along(Atom_as_char(&arg->car) + Atom_as_char(&sum));
	}
	
	return sum;
}

Atom mult(Cons * args) {
	if(args == NULL) return nil;
		
	Atom product = Achar(1);
	
	foreach(arg, args) {
		assert(NUM_HIERARCHY(arg->car.type) >= 0 && NUM_HIERARCHY(product.type) >= 0);
		Type * higher = hitype(arg->car.type, product.type);
		
		if(higher == DoubleType)
			product = Adouble(Atom_as_double(&arg->car) * Atom_as_double(&product));
		else if(higher == IntType)
			product = Along(Atom_as_long(&arg->car) * Atom_as_long(&product));
		else if(higher == CharType)
			product = Along(Atom_as_char(&arg->car) * Atom_as_char(&product));
	}
	
	return product;
}

Atom subs(Cons * args) {	
	BI_N_OP(-)
}

Atom divf(Cons * args) {	
	BI_N_OP(/);
}

Atom mod(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	assert(a.type == IntType && a.type == IntType);
	
	return Along(a.d._long % b.d._long);
}

Atom lt(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(a.type == SymbolType && b.type == SymbolType)
		return Abool(Symbol_lt(a.d._Symbol, b.d._Symbol));
	else if(a.type == StringType && b.type == StringType)
		return Abool(strcmp(a.d._String->str, b.d._String->str) < 0);
	
	BI_N_BOOL_OP( < )
}

Atom gt(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(a.type == SymbolType && b.type == SymbolType)
		return Abool(Symbol_gt(a.d._Symbol, b.d._Symbol));
	else if(a.type == StringType && b.type == StringType)
		return Abool(strcmp(a.d._String->str, b.d._String->str) > 0);
	
	BI_N_BOOL_OP( > )
}

Atom le(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(a.type == SymbolType && b.type == SymbolType)
		return Abool(Symbol_le(a.d._Symbol, b.d._Symbol));
	else if(a.type == StringType && b.type == StringType)
		return Abool(strcmp(a.d._String->str, b.d._String->str) <= 0);
	
	BI_N_BOOL_OP( <= )
}

Atom ge(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(a.type == SymbolType && b.type == SymbolType)
		return Abool(Symbol_ge(a.d._Symbol, b.d._Symbol));
	else if(a.type == StringType && b.type == StringType)
		return Abool(strcmp(a.d._String->str, b.d._String->str) >= 0);
	
	BI_N_BOOL_OP( >= )
}

Atom print(Cons * args) {
	foreach(el, args) {
		printf("%s", cstr(toString(&el->car)));
	}
	
	return nil;
}

Atom println(Cons * args) {
	foreach(el, args) {
		printf("%s", cstr(toString(&el->car)));
	}
	
	printf("\n");
	
	return nil;
}

Atom equals(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
		 
	if(a.type == StringType && b.type == StringType) 
		return Abool(!strcmp(a.d._String->str, b.d._String->str));
				
	
	if(NUM_HIERARCHY(a.type) >= 0 && NUM_HIERARCHY(a.type) >= 0) {
		Type * higher = hitype(a.type, b.type);
		
		if(higher == DoubleType)
			return Abool(Atom_as_double(&a) == Atom_as_double(&b));
		else if(higher == IntType)
			return Abool(Atom_as_long(&a) == Atom_as_long(&b));
		else if(higher == CharType)
			return Abool(Atom_as_char(&a) == Atom_as_char(&b));
	} else {
		if(a.type == b.type) {
			return Abool(!memcmp(&a.d, &b.d, sizeof(anydata)));
		} else return Abool(false);
	}
}

Atom notequals(Cons * args) {
	return Abool(!(equals(args).d._bool));
}

Atom same(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(NUM_HIERARCHY(a.type) >= 0 && NUM_HIERARCHY(a.type) >= 0) {
		Type * higher = hitype(a.type, b.type);
		
		if(higher == DoubleType)
			return Abool(Atom_as_double(&a) == Atom_as_double(&b));
		else if(higher == IntType)
			return Abool(Atom_as_long(&a) == Atom_as_long(&b));
		else if(higher == CharType)
			return Abool(Atom_as_char(&a) == Atom_as_char(&b));
	} else {
		if(a.type == b.type) {
			return Abool(!memcmp(&a.d, &b.d, sizeof(anydata)));
		} else return Abool(false);
	}
}

Atom notsame(Cons * args) {
	Atom a = *Cons_at(args, 0),
		 b = *Cons_at(args, 1);
	
	if(NUM_HIERARCHY(a.type) >= 0 && NUM_HIERARCHY(a.type) >= 0) {
		Type * higher = hitype(a.type, b.type);
		
		if(higher == DoubleType)
			return Adouble(Atom_as_double(&a) != Atom_as_double(&b));
		else if(higher == IntType)
			return Along(Atom_as_long(&a) != Atom_as_long(&b));
		else if(higher == CharType)
			return Along(Atom_as_char(&a) != Atom_as_char(&b));
	} else {
		if(a.type != b.type) return Abool(true);
		else return Abool(memcmp(&a.d, &b.d, sizeof(anydata)));
	}
}

Atom consf (Cons * args) {
	return ACons(Cons_newa(args->car, *Cons_at(args, 1)));
}

Atom type_of (Cons * args) {
	return AType(args->car.type);
}

Atom car (Cons * args) {
	assert(args->car.type == ConsType);
	return args->car.d._Cons->car;
}

Atom cdr (Cons * args) {
	assert(args->car.type == ConsType);
	return args->car.d._Cons->cdr;
}

Atom at (Cons * args) {
	if(args->car.type == StringType)
		return Achar(String_charAt(args->car.d._String, Atom_as_long(Cons_at(args, 1))));
	else if(args->car.type == ConsType)
		return *Cons_at(args->car.d._Cons, Atom_as_long(Cons_at(args, 1)));
	
	assert(false);
}

Atom length (Cons * args) {
	if(args->car.type == StringType)
		return Along(String_length(args->car.d._String));
	else if(args->car.type == ConsType)
		return Along(Cons_length(args->car.d._Cons));
	
	assert(false);
}

Atom not_ (Cons * args) {
	return Abool(!Atom_as_bool(&args->car));
}

Atom or_ (Cons * args) {
	foreach(el, args)
		if(Atom_as_bool(&el->car)) return Abool(true);

	return Abool(false);
}

Atom and_ (Cons * args) {
	foreach(el, args)
		if(!Atom_as_bool(&el->car)) return Abool(false);

	return Abool(true);
}

Atom setcar(Cons * args) {
	assert(args->car.type == ConsType);
	return args->car.d._Cons->car = *Cons_at(args, 1);
}

Atom setcdr(Cons * args) {
	assert(args->car.type == ConsType);
	return args->car.d._Cons->cdr = *Cons_at(args, 1);
}

Atom collect(Cons * args) {
	GC_mark();
	GC_sweep();
	return nil;
}

Atom eval_naughty(Cons * args) {
	//args[0] what to eval
	//args[1] environment
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	args = Cons_next(args);
	
	evaluator_naughty_access->s.exp = args->car;
	evaluator_naughty_access->value = nil;
	
	if(Cons_next(args)) 
		evaluator_naughty_access->s.env = Cons_at(args, 1)->d._Environment; // or module
	
	evaluator_naughty_access->s.instr = EVAL;
	evaluator_naughty_access->naughty = true;
	
	return nil; // no use
}

Atom apply_naughty(Cons * args) {
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	args = Cons_next(args);
	
	evaluator_naughty_access->value = nil;
	evaluator_naughty_access->s.evarg = Cons_new(args->car, Cons_at(args, 1)->d._Cons);
	evaluator_naughty_access->s.instr = CALL;
	evaluator_naughty_access->naughty = true;
	
	return nil; // no use
}

Atom callcc_naughty(Cons * args) {
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	args = Cons_next(args);
	
	Continuation * c = evaluator_naughty_access->next;
	
	evaluator_naughty_access->value = nil;
	
	evaluator_naughty_access->s.evarg = NULL;
	Cons_add(&evaluator_naughty_access->s.evarg, args->car); // the function
	Cons_add(&evaluator_naughty_access->s.evarg, AContinuation(c)); // the cc
	
	evaluator_naughty_access->s.instr = CALL;
	evaluator_naughty_access->naughty = true;
	
	return nil;
}

Atom quit_naughty(Cons * args) {
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	
	evaluator_naughty_access->quit = true;
	evaluator_naughty_access->s.instr = NULL;
	
	return nil;
}

Atom stacktrace_naughty(Cons * args) {
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	
	printf("Frame #%03d: ", 0);
	dump_inline(evaluator_naughty_access->s.exp);
	printf("\n");
	
	Continuation * frame = evaluator_naughty_access->next;
	
	size_t frameno = 1;
	
	while(frame) {
		printf("Frame #%03d: ", frameno++);
		dump_inline(frame->frame.exp);
		printf("\n");
	
		frame = frame->next;
	}
	
	//Evaluator_restoreState(evaluator_naughty_access);
	
	return nil;
}
/*
Atom syntactic_closure_naughty(Cons * args) {
	//args[0] what to eval
	//args[1] environment
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	args = Cons_next(args);
	
	evaluator_naughty_access->s.exp = args->car;
	evaluator_naughty_access->value = nil;
	
	if(Cons_next(args)) 
		evaluator_naughty_access->s.env = Cons_at(args, 1)->d._Environment; // or module
	
	evaluator_naughty_access->s.instr = EVAL;
	evaluator_naughty_access->naughty = true;
	
	return nil; // no use
	
	
	
	Evaluator * evaluator_naughty_access = args->car.d._Evaluator;
	
	printf("Frame #%03d: ", 0);
	dump_inline(evaluator_naughty_access->s.exp);
	printf("\n");
	
	Continuation * frame = evaluator_naughty_access->next;
	
	size_t frameno = 1;
	
	while(frame) {
		printf("Frame #%03d: ", frameno++);
		dump_inline(frame->frame.exp);
		printf("\n");
	
		frame = frame->next;
	}
	
	//Evaluator_restoreState(evaluator_naughty_access);
	
	return nil;
}*/

Atom stacktrace_of(Cons * args) {
	assert(args->car.type == ContinuationType);
	
	Continuation * frame = args->car.d._Continuation;
	
	size_t frameno = 0;
	
	while(frame) {
		printf("Frame #%03d (%s): ", frameno++, instr_name(frame->frame.instr));
		dump_inline(frame->frame.exp);
		printf("\n");
	
		frame = frame->next;
	}
	
	return nil;
}

Atom strcat_(Cons * args) {
	String * result = String_new("");
	
	foreach(arg, args) {
		result = String_cat(result, toString(&arg->car));
	}
	
	return AString(result);
	
	// should also concatenate arrays.
}

Atom iscons(Cons * args) {
	return Abool(args->car.type == ConsType);
	
	// should also concatenate arrays.
}

Atom dumpp(Cons * args) {
	dump(args->car, 0);
	
	// should also concatenate arrays.
}


Atom isint(Cons * args) {
	return Abool(args->car.type == IntType);
	
	// should also concatenate arrays.
}

Atom isnil(Cons * args) {
	return Abool(Atom_isNil(&args->car));
	
	// should also concatenate arrays.
}


/*Atom environ(Cons * args) {
	Environment * en = Environment_new(NULL, NULL),
				* parent = args ? args->car.d._Environment : NULL;
	if(parent) Environment_addImport(en, parent);
	return AEnvironment(en);
	
	// should also concatenate arrays.
}*/
/*

Atom eval_naughty(Cons * args)
Atom apply_naughty(Cons * args)
Atom callcc_naughty(Cons * args)*/

void definefunction(Module * m, Symbol s, Lambda * l) {
	Environment_define((Environment *) m, s, ALambda(l));
	l->name = s;
}

void init_std(Evaluator * e) {
	std = Module_new(NULL, NULL, NULL);
	
	Environment_define((Environment *) std, symbol("Cons"), AType(ConsType));
	Environment_define((Environment *) std, symbol("String"), AType(StringType));
	Environment_define((Environment *) std, symbol("Lambda"), AType(LambdaType));
	Environment_define((Environment *) std, symbol("Continuation"), AType(ContinuationType));
	Environment_define((Environment *) std, symbol("Environment"), AType(EnvironmentType));
	Environment_define((Environment *) std, symbol("Module"), AType(ModuleType));
	Environment_define((Environment *) std, symbol("Int"), AType(IntType));
	Environment_define((Environment *) std, symbol("Char"), AType(CharType));
	Environment_define((Environment *) std, symbol("Double"), AType(DoubleType));
	Environment_define((Environment *) std, symbol("Bool"), AType(BoolType));
	Environment_define((Environment *) std, symbol("Symbol"), AType(SymbolType));
	Environment_define((Environment *) std, symbol("Keyword"), AType(KeywordType));
	Environment_define((Environment *) std, symbol("Type"), AType(TypeType));
	
	Environment_define((Environment *) std, symbol("endl"), Achar('\n'));
	//Environment_define((Environment *) std, symbol("std"), AModule(std));
	
	definefunction(std, symbol("error"), Lambda_new_c_simple(error));
	
	definefunction(std, symbol("+"), Lambda_new_c_simple(add));
	definefunction(std, symbol("*"), Lambda_new_c_simple(mult));
	definefunction(std, symbol("-"), Lambda_new_c_simple(subs));
	definefunction(std, symbol("/"), Lambda_new_c_simple(divf)); // should not be integer div
	definefunction(std, symbol("%"), Lambda_new_c_simple(mod));
	
	definefunction(std, symbol("<"), Lambda_new_c_simple(lt));
	definefunction(std, symbol(">"), Lambda_new_c_simple(gt));
	definefunction(std, symbol("<="), Lambda_new_c_simple(le));
	definefunction(std, symbol(">="), Lambda_new_c_simple(ge));
	
	definefunction(std, symbol("=="), Lambda_new_c_simple(equals)); 
	definefunction(std, symbol("!="), Lambda_new_c_simple(notequals));
	
	definefunction(std, symbol("and"), Lambda_new_c_simple(and_));
	definefunction(std, symbol("not"), Lambda_new_c_simple(not_));
	definefunction(std, symbol("or"), Lambda_new_c_simple(or_));
	
	// should be in std.io
	definefunction(std, symbol("print"), Lambda_new_c_simple(print)); 
	definefunction(std, symbol("println"), Lambda_new_c_simple(println));
	
	definefunction(std, symbol("strcat"), Lambda_new_c_simple(strcat_));
	
	definefunction(std, symbol("list"), Lambda_new_c_simple(Alistcopy));
	definefunction(std, symbol("cons"), Lambda_new_c_simple(consf));
	definefunction(std, symbol("car"), Lambda_new_c_simple(car));
	definefunction(std, symbol("cdr"), Lambda_new_c_simple(cdr));
	definefunction(std, symbol("set-car!"), Lambda_new_c_simple(setcar));
	definefunction(std, symbol("set-cdr!"), Lambda_new_c_simple(setcdr));
	definefunction(std, symbol("nil?"), Lambda_new_c_simple(isnil));
	
	definefunction(std, symbol("length"), Lambda_new_c_simple(length));
	definefunction(std, symbol("at"), Lambda_new_c_simple(at));
	
	definefunction(std, symbol("type-of"), Lambda_new_c_simple(type_of));
	definefunction(std, symbol("cons?"), Lambda_new_c_simple(iscons));
	definefunction(std, symbol("int?"), Lambda_new_c_simple(isint));
	
	definefunction(std, symbol("gc-collect"), Lambda_new_c_simple(collect));
	
	definefunction(std, symbol("eval"), Lambda_new_c_simple_naughty(eval_naughty));
	definefunction(std, symbol("apply"), Lambda_new_c_simple_naughty(apply_naughty));
	definefunction(std, symbol("call-with-current-continuation"), Lambda_new_c_simple_naughty(callcc_naughty));
	definefunction(std, symbol("call/cc"), Lambda_new_c_simple_naughty(callcc_naughty));
	definefunction(std, symbol("quit"), Lambda_new_c_simple_naughty(quit_naughty));
	definefunction(std, symbol("stacktrace"), Lambda_new_c_simple_naughty(stacktrace_naughty));
	definefunction(std, symbol("stacktrace-of"), Lambda_new_c_simple(stacktrace_of));
	definefunction(std, symbol("dump"), Lambda_new_c_simple(dumpp));
	
	/*Environment_define((Environment *) std, symbol("error"), ALambda(Lambda_new_c_simple(error)));
	
	Environment_define((Environment *) std, symbol("+"), ALambda(Lambda_new_c_simple(add)));
	Environment_define((Environment *) std, symbol("*"), ALambda(Lambda_new_c_simple(mult)));
	Environment_define((Environment *) std, symbol("-"), ALambda(Lambda_new_c_simple(subs)));
	Environment_define((Environment *) std, symbol("/"), ALambda(Lambda_new_c_simple(divf))); // should not be integer div
	Environment_define((Environment *) std, symbol("%"), ALambda(Lambda_new_c_simple(mod)));
	
	Environment_define((Environment *) std, symbol("<"), ALambda(Lambda_new_c_simple(lt)));
	Environment_define((Environment *) std, symbol(">"), ALambda(Lambda_new_c_simple(gt)));
	Environment_define((Environment *) std, symbol("<="), ALambda(Lambda_new_c_simple(le)));
	Environment_define((Environment *) std, symbol(">="), ALambda(Lambda_new_c_simple(ge)));
	
	Environment_define((Environment *) std, symbol("=="), ALambda(Lambda_new_c_simple(same))); 
	Environment_define((Environment *) std, symbol("!="), ALambda(Lambda_new_c_simple(notsame)));
	
	Environment_define((Environment *) std, symbol("and"), ALambda(Lambda_new_c_simple(and_)));
	Environment_define((Environment *) std, symbol("not"), ALambda(Lambda_new_c_simple(not_)));
	Environment_define((Environment *) std, symbol("or"), ALambda(Lambda_new_c_simple(or_)));
	
	// should be in std.io
	Environment_define((Environment *) std, symbol("print"), ALambda(Lambda_new_c_simple(print))); 
	Environment_define((Environment *) std, symbol("println"), ALambda(Lambda_new_c_simple(println)));
	
	Environment_define((Environment *) std, symbol("strcat"), ALambda(Lambda_new_c_simple(strcat_)));
	
	Environment_define((Environment *) std, symbol("list"), ALambda(Lambda_new_c_simple(Alistcopy)));
	Environment_define((Environment *) std, symbol("cons"), ALambda(Lambda_new_c_simple(consf)));
	Environment_define((Environment *) std, symbol("car"), ALambda(Lambda_new_c_simple(car)));
	Environment_define((Environment *) std, symbol("cdr"), ALambda(Lambda_new_c_simple(cdr)));
	Environment_define((Environment *) std, symbol("set-car!"), ALambda(Lambda_new_c_simple(setcar)));
	Environment_define((Environment *) std, symbol("set-cdr!"), ALambda(Lambda_new_c_simple(setcdr)));
	Environment_define((Environment *) std, symbol("nil?"), ALambda(Lambda_new_c_simple(isnil)));
	
	Environment_define((Environment *) std, symbol("length"), ALambda(Lambda_new_c_simple(length)));
	Environment_define((Environment *) std, symbol("at"), ALambda(Lambda_new_c_simple(at)));
	
	Environment_define((Environment *) std, symbol("type-of"), ALambda(Lambda_new_c_simple(type_of)));
	Environment_define((Environment *) std, symbol("cons?"), ALambda(Lambda_new_c_simple(iscons)));
	Environment_define((Environment *) std, symbol("int?"), ALambda(Lambda_new_c_simple(isint)));
	
	Environment_define((Environment *) std, symbol("gc-collect"), ALambda(Lambda_new_c_simple(collect)));
	
	Environment_define((Environment *) std, symbol("eval"), ALambda(Lambda_new_c_simple_naughty(eval_naughty)));
	Environment_define((Environment *) std, symbol("apply"), ALambda(Lambda_new_c_simple_naughty(apply_naughty)));
	Environment_define((Environment *) std, symbol("call-with-current-continuation"), ALambda(Lambda_new_c_simple_naughty(callcc_naughty)));
	Environment_define((Environment *) std, symbol("call/cc"), ALambda(Lambda_new_c_simple_naughty(callcc_naughty)));
	Environment_define((Environment *) std, symbol("quit"), ALambda(Lambda_new_c_simple_naughty(quit_naughty)));
	Environment_define((Environment *) std, symbol("stacktrace"), ALambda(Lambda_new_c_simple_naughty(stacktrace_naughty)));
	Environment_define((Environment *) std, symbol("stacktrace-of"), ALambda(Lambda_new_c_simple(stacktrace_of)));*/
	
	//if(!global_buffer) global_buffer = malloc(sizeof(char) * READ_BUFFER_SIZE);
	
	isstream iss = isstream_make(
	"(define (foldr f v l) (if (nil? l) v (f (car l) (foldr f v (cdr l))) ) )"
	"(define (foldl f v l) (if (nil? l) v (foldl f (f v (car l)) (cdr l)) ) )"
	"(define (foldr1 f l) (if (nil? (cdr l)) (car l) (f (car l) (foldr1 f (cdr l))) ) )"
	"(define (foldl1 f l) (foldl f (car l) (cdr l)) )"
	"(define (filter f l) (foldr (lambda (x y) (if (f x) (cons x y) y)) nil l) )"
	"(define (reverse l) (foldl (lambda (x y) (cons y x)) nil l) )"
	"(define (append (args)) (foldr (lambda (a b) (foldr cons b a)) nil args) )"
	"(define (map-1 f l) (foldr (lambda (a b) (cons (f a) b)) nil l))"
	"(define (map f (rest)) (if (apply and rest) (cons (apply f (map-1 car rest)) (apply map (cons f (map-1 cdr rest)))) nil) )"
	"(define (id (args)) (car args) )"
	"(define (flip f) (lambda (a b) (f b a)) )"
	"(define (unpacker f) (lambda (args) (apply f args)) )"
	"(define (compose f g) (lambda ((args)) (f (apply g args))) )"
	"(define (rcompose f g) (lambda ((args)) (g (apply f args))) )"
	"(define (<< (args)) (foldr1 compose args) )"
	"(define (>> (args)) (foldl1 rcompose args) )"
	"(define (flip f) (lambda (a b) (f b a)) )"
	"(define (iterate f n) (if (zero? n) id (== n 1) f (compose f (iterate f (- n 1))) ) )"
	"(define (const x) (lambda ((args)) x))"
	"(define (drop n l) (if (zero? n) l (nil? l) nil (drop (- n 1) (cdr l)) ) )"
	"(define (take n l) (if (zero? n) nil (nil? l) nil (cons (car l) (take (- n 1) (cdr l))) ) )"
	"(define (even? n) (== (% n 2) 0) )"
	"(define (odd? n) (!= (% n 2) 0) )"
	"(define (zero? n) (== n 0) )"
	// "(define (nil? x) (== x nil) )"
	"(define (negative? n) (< n 0) )"
	"(define (positive? n) (> n 0) )"
	
	"(define (. obj func) (lambda ((args)) (apply func (cons obj args)) ) )"
	"(define (max l) (if (nil? (cdr l)) (car l) (let (m (max (cdr l))) (if (> (car l) m)  (car l) m) ) ) )"
	"(define (min l) (if (nil? (cdr l)) (car l) (let (m (max (cdr l))) (if (< (car l) m)  (car l) m) ) ) )"
	"(define (drop-while p? l) (if (nil? l) nil (p? (car l)) (drop-while p? (cdr l)) l ) )"
	"(define (take-while p? l) (if (nil? l) nil (p? (car l)) (cons (car l) (take-while p? (cdr l))) nil) )"
	
	"(define (cons? x) (== (type-of x) (type-of '()) ) )"
	"(define (isa? x t) (== (type-of x) t ) )"
	"(define (caris? l c) (and l (== (car l) c)) )"
	//" cadr := car << cdr "
	//" caddr := car << cdr << cdr "
	//" select := filter "
	" (define cadr (compose car cdr)) "
	" (define caddr (compose cadr cdr)) "
	" (define select filter) "
	" (define (reject f l) (filter not << f l)) "
	);
	
	rflags rf;
	rflags_reset(&rf);
	
	Cons * program = global_infix_transform(readl((istream *)&iss, global_buffer, &rf));
	
	Evaluator_eval_seq(e, program, (Environment *) std);
	
	//dump(program, 0);
	
	Module_export(std, Cons_list(88, 
		Asymbol("quit"), 
		Asymbol("Cons"), 
		Asymbol("String"),
		Asymbol("Lambda"),
		Asymbol("Continuation"),
		Asymbol("Environment"),
		Asymbol("Module"),
		Asymbol("Int"), 
		Asymbol("Char"),
		Asymbol("Double"),
		Asymbol("Bool"),
		Asymbol("Symbol"),
		Asymbol("Keyword"),
		Asymbol("Type"),
		Asymbol("endl"),
		Asymbol("error"),
		Asymbol("+"),
		Asymbol("*"),
		Asymbol("-"),
		Asymbol("/"),
		Asymbol("%"),
		Asymbol("<"),
		Asymbol(">"),
		Asymbol("<="),
		Asymbol(">="),
		Asymbol("=="),
		Asymbol("!="),
		Asymbol("and"),
		Asymbol("not"),
		Asymbol("or"),
		Asymbol("print"),
		Asymbol("println"),
		Asymbol("strcat"),
		Asymbol("list"),
		Asymbol("cons"),
		Asymbol("car"),
		Asymbol("cdr"),
		Asymbol("set-car!"),
		Asymbol("set-cdr!"),
		Asymbol("length"),
		Asymbol("at"),
		Asymbol("type-of"),
		Asymbol("gc-collect"),
		Asymbol("eval"),
		Asymbol("apply"),
		Asymbol("call-with-current-continuation"),
		Asymbol("call/cc"),
		Asymbol("foldr"),
		Asymbol("foldl"),
		Asymbol("foldr1"),
		Asymbol("foldl1"),
		Asymbol("filter"),
		Asymbol("select"),
		Asymbol("reject"),
		Asymbol("reverse"),
		Asymbol("append"),
		Asymbol("map"),
		Asymbol("id"),
		Asymbol("flip"),
		Asymbol("unpacker"),
		Asymbol("compose"),
		Asymbol("rcompose"),
		Asymbol("<<"),
		Asymbol(">>"),
		Asymbol("iterate"),
		Asymbol("const"),
		Asymbol("drop"),
		Asymbol("take"),
		Asymbol("int?"),
		Asymbol("even?"),
		Asymbol("odd?"),
		Asymbol("zero?"),
		Asymbol("nil?"),
		Asymbol("cons?"),
		Asymbol("isa?"),
		Asymbol("caris?"),
		Asymbol("negative?"),
		Asymbol("positive?"),
		Asymbol("."),
		Asymbol("max"),
		Asymbol("min"),
		Asymbol("drop-while"),
		Asymbol("take-while"),
		Asymbol("cadr"),
		Asymbol("caddr"),
		Asymbol("stacktrace"),
		Asymbol("dump"),
		Asymbol("stacktrace-of")
	));
}

#ifdef putin












Atom array (Cons * args) {
	if(args[0].isa<int>())
		return Atom(gc_ptr<Array>(new Array(args[0].as<int>())));
		
	throw BadFunctionCall("Illegal argument."); 
}

Atom at (Cons * args) {
	if(args[0].getType() == a_type(gc_ptr<Array>))
		return args[0].get<gc_ptr<Array> >()->get_at(args[1].as<int>());
	else if(args[0].getType() == a_type(gc_ptr<String>))
		return args[0].get<gc_ptr<String> >()->charAt(args[1].as<int>());
	else if(args[0].getType() == a_type(gc_ptr<DictP>)) {
		HashNode<symbol, Atom> * hn = args[0].get<gc_ptr<DictP> >()->search(args[1].get<symbol>());
		if(hn != NULL) return hn->value();
		throw UndefinedSymbol(args[1].get<symbol>().s);
	} else if(args[0].getType() == a_type(gc_ptr<Cons>))
		return args[0].getList()[args[1].as<int>()];
	else if(args[0].isa<gc_ptr<Object> >()) { 
		Atom * r = args[0].get<gc_ptr<Object> >()->getSlot(args[1].get<symbol>());
		return *r;
	}
		
	throw BadFunctionCall("Illegal argument."); 
}

Atom set_at (Cons * args) {
	if(args[0].getType() == a_type(gc_ptr<Array>))
		return args[0].get<gc_ptr<Array> >()->set_at(args[1].as<int>(), args[2]);
	else if(args[0].getType() == a_type(gc_ptr<DictP>)) {
		
		return args[0].get<gc_ptr<DictP> >()->add(args[1].get<symbol>(), args[2])->value();
	} else if(args[0].getType() == a_type(gc_ptr<Cons>))
		return args[0].getList()[args[1].as<int>()] = args[2];
	else if(args[0].isa<gc_ptr<Object> >()) {
		Atom * r = args[0].get<gc_ptr<Object> >()->getSlot(args[1].get<symbol>());
		*r = args[2];
		return args[2];
	}
		
	throw BadFunctionCall("Illegal argument."); 
}

Atom read(Cons * args) {
	string buffer;
	buffer.reserve(100);
	read_flags fl;
	if(args.first_ == NULL) {
		return Atom(global_infix_transform(readl(cin, buffer, fl)));
	} else {
		ifstream fs(args[0].get<gc_ptr<String> >()->get(), ifstream::in);
		return Atom(global_infix_transform(readl(fs, buffer, fl)));
	}
	
	
	//ifstream fs(argv[optind], ifstream::in);
	
	
	//return interpret(stdin, true).head();
	
}

Atom dict(Cons * args) {
	gc_ptr<DictP> d = new DictP();
	
	gc_ptr<Cons> el = args.first_;
	
	for (;el != NULL && el->next() != NULL; el = el->next()->next())
		d->add(el->content().getSym(), el->next()->content());
		
	return Atom(d);
}

Atom classm(Cons * args) {
	//args[0] parents
	//args[1] members
	return Atom(gc_ptr<Class>(new Class(args[0].getList(), args[1].getList(), args[2], args[3])));
}

Atom instance(Cons * args) {
	//args[0] parents
	//args[1] members
	
	return Atom(gc_ptr<Object>(new Object(args[0].get<gc_ptr<Class> >())));
}

Atom ns_since_epoch(Cons * args) { 
	// valid up to the year 2260 or something like that.
	// does not have the full timespec precision
	timespec a;
	clock_gettime(CLOCK_REALTIME, &a);
	return Atom(a.tv_sec * 1000000000L + a.tv_nsec); // in nanoseconds
}

Atom ns_sleep (Cons * args) {
	timespec s, r;
	long t = args[0].as<long>();
	s.tv_sec = t / 1000000000L;
	s.tv_nsec = t % 1000000000L;
	return Atom(nanosleep(&s, &r));
}

Atom expr(Cons * args) {
	//if(!optbl) optbl = new operator_table();
	
	args = args.tail();
	
	if(args.first_ == NULL) throw BadExpression("No arguments for expr.");
	else if(args.tail().first_ == NULL) return args.head();
	else {
		int l = args.length();
		
		if(l % 2) {
			Atom e = args.head();
			//Atom op = args.tail().head();
			
			stack<Atom> operators;
			stack<Atom> operands;
			
			stack<gc_ptr<Cons> > operand_dests;
			//Cons * output;
			
			int i = 0;
			
			gc_ptr<Cons> operand_destination = NULL;
			//Atom last_op;
			
			for(gc_ptr<Cons> op = args.first_; i < l; op = op->next(), i++) {
				//operator_table::operator_description d;
				
				//While there is an operator B of higher or equal precidence than A at the top of the stack, pop B off the stack and append it to the output.
				//Push A onto the stack.
							
				if(i % 2) {
					while(!operators.empty() && optbl->get(operators.top()).precedence 
						>= optbl->get(op->content()).precedence ) {
						
						 expr_helper(operators, operands, operand_dests);
					} 
					
					operators.push(op->content());
					
				} else {
					operands.push(op->content());
					operand_dests.push(gc_ptr<Cons>());
				}
			}
			
			while(!operators.empty()) {
				expr_helper(operators, operands, operand_dests);
			}
			
			// dump(operands.top(), "");
			
			return operands.top();
			
			
			
		} else throw BadExpression("Bad arguments for expr.");
		
		
	}
}

Atom isa(Cons * args) {
	// a is a b
	gc_ptr<Class> a = args[0].get<gc_ptr<Class> >();
	gc_ptr<Class> b = args[1].get<gc_ptr<Class> >();
	return a == b 
		|| find(a->parent_line.begin(), a->parent_line.end(), b) != a->parent_line.end();
}

Atom constructor_params(Cons * args) {
	return args[0].get<gc_ptr<Class> >()->ctor_params;
	
	// should also concatenate arrays.
}

#endif
