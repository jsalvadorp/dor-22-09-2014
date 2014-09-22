#ifndef INFIX_H
#define INFIX_H

#include "forward.h"
#include "types.h"
#include "cons.h"

#define ASTACK Cons *
#define AS_TOP(x) (x)->car
#define AS_POP(x) (x) = Cons_next((x))
#define AS_PUSH(x, y) (x) = Cons_new(y, (x))
#define AS_EMPTY(x) ((x) == NULL)
#define AS_NOT_EMPTY(x) (x)

typedef enum assoc {
	la, ra, va
} assoc;

/*
operator_description operator_description_make(int p, enum assoc a);
struct OperatorTree * OperatorTree_new(Symbol name, int p, enum assoc a);*/

/*extern struct OperatorTree * operators;
extern struct OperatorTree * global_operators;*/

void Ops_add(int precedence, Symbol name, enum assoc associativity);

void GlobalOps_add(int precedence, Symbol name, assoc associativity);

/*
struct OperatorTree * Ops_search(Symbol name);

struct OperatorTree * GlobalOps_search(Symbol name);

operator_description Ops_get(Atom a);

operator_description Ops_get(Atom a);*/

void init_operators();
void delete_operators();


Atom global_infix_transform_ifl(Atom list);

Cons * global_infix_transform(Cons * list);

Atom global_infix_transform_ifl(Atom list);

Atom git(Cons * args);

//void expr_helper(ASTACK * operators, ASTACK * operands, ASTACK * operand_dests);

Atom expr(Cons * args);

#endif
