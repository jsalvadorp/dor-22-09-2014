#include "analysis.h"

kind_t * kind0;
tenv_t * globalt;

tenv_t * typeenv(tenv_t * parent) {
	tenv_t * newtenv = malloc(sizeof(tenv_t));
	
	newtenv->parent = parent;
	newtenv->dict = hmap_Symbol_typeptr_new(32);
	
	return newtenv;
}

type_t * tvar(type_t * pointed) {
	tvar_t * newtvar = malloc(sizeof(tvar_t));
	((type_t *) newtvar)->tag = TVAR;
	
	newtvar->pointed = pointed;
	
	return newtvar;
}

type_t * cpmap(tenv_t * parent, ASTList * body) {
	ASTAtom l, r;
	l = body->car;
	r = body->next->car;
	
	tenv_t * env = typeenv(parent);
	
	if(l.type = ConsType) {
		foreach(el, l.d._ASTList) {
			if(el->car.type == SymbolType) { // type variable
				// ASSERT that variable is not previously defined
				
				hmap_Symbol_typeptr_put(&env->dict, el->car.d._Symbol, tvar(NULL));
			} else assert(0);
		}
	} else {
		assert(l.type == SymbolType);
		
		hmap_Symbol_typeptr_put(&env->dict, el->car.d._Symbol, tvar(NULL));
	}
	
	quantified_t * qt = malloc(sizeof(quantified_t));
	((type_t *) qt)->tag = QUANT;
	qt->env = env;
	qt->exp = ctexp(env, r);
	
	return (type_t *) qt;
}
 
type_t * funct(type_t * takes, type_t * rets);

type_t * ctexp(tenv_t * env, ASTAtom expr);

type_t * cfunct(tenv_t * env, ASTList * body);
type_t * cadt(tenv_t * env, ASTList * body);
type_t * ctuple(tenv_t * env, ASTList * body);




#if 0

fields: the symbol refers to the field offset and the getter function

Person := Person name :string age :int
age-list <- map age ps
(age juan) <- 45 ; mutation

 

modules as vtables as typeclasses
parameterized modules?
stack allocation?
custom allocation/gc?

vm stack is 8byte aligned on every architecture

iface (Stringable t)
	string :: t -> string

impl (Stringable string)
	string x = x

print :: s (Stringable s) =>  s -> void
print x = ...

println x = do
	print x
	print $\n
	
better handling of binary data with pattern matching, see erlang

letalloc myalloc
	list <- [1 2 3 4 5]
	

teeno
text-based gui. monospace. clean. lightweight.

bytecode binaries, no type information, only symbols. (similar to pcode
or java class files)
compiled interfaces, type information and module structure for __public__
symbols. used during compilation and type-safe dynamic linking, scripting.

only global constants and functions. variables hold addresses to a thread-
or process- local copy.

multiple entry points (?)

shared libs

multiple apps can share an address space, share a vm, etc, or run in
a separate one

garbage collection contexts? let allocator



type safe dorsh shell!! using streams and dynamic loading of modules.


fst x y = x

(= ((fst x) y) x)

x + y / 7
(+ x (/ y 7)	Q

a b c
fst :: a -> b -> c
x :: a
y :: b
<ret> :: c

	x :: a
	<ret> :: a
	a = c

print :: s : Stringable => a -> void
print = printstr << string

#endif

