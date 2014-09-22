#ifndef CONS_H
#define CONS_H

#include "types.h"
#include "gc.h"
#include <string.h>
#include <assert.h>

// CONSIDER MAKING CDR A Cons *
// for type safety and speed (no need to check if it's a cons)
// implications: cons may only store proper lists.
// 				 must make a new type, pair, that can accept a cdr of any type.

typedef struct Cons {
	GCObject_HEAD
	Atom car;
	Atom cdr;
} Cons;

extern struct GCObject_vtbl_t Cons_vtbl;

Atom ACons(Cons * c);
Cons * Cons_new(Atom car, Cons * cdr);
Cons * Cons_newa(Atom car, Atom cdr);
Cons * Cons_add(Cons ** l, Atom n);
Cons * Cons_append(Cons ** l, Cons * l2);
Cons * Cons_list(size_t count, ...);
Cons * Cons_last(Cons * l);
Cons * Cons_reversed(Cons * l);
Atom * Cons_at(Cons * this, size_t index);

size_t Cons_length(Cons * this);

void Cons_grayChildren(GCObject * this);
void Cons_destroy(GCObject * this);

// get a pointer to the next element
Cons * Cons_next(Cons * c);

// get a reference to the cdr as a pointer to the next element
Cons ** Cons_nextr(Cons * c);

Atom cons(Atom car, Atom cdr);
Atom list(size_t count, ...);

Atom Alistcopy(Cons * list);

#define foreach(el, li) for(Cons * el = li; el; el = Cons_next(el))
#define QUOTE(a) list(2, ASymbol(s_quote), a)




#endif
