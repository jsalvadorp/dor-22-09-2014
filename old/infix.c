#include "infix.h"
#include <assert.h>

typedef struct operator_description {
	int precedence;
	enum assoc associativity;
} operator_description;

typedef struct OperatorTree { 
	struct OperatorTree * left;
	struct OperatorTree * right;
	
	Symbol s;
	operator_description op;
} OperatorTree;


operator_description operator_description_make(int p, assoc a) {
	// default 0, la
	
	operator_description od;
	od.precedence = p;
	od.associativity = a;
}

OperatorTree * OperatorTree_new(Symbol name, int p, assoc a) {
	OperatorTree * ot = malloc(sizeof(OperatorTree));
	
	ot->s = name;
	ot->op = operator_description_make(p, a);
	
	ot->left = NULL;
	ot->right = NULL;
	
	return ot;
}

void OperatorTree_destroy(OperatorTree * this) {
	if(this->left) {
		OperatorTree_destroy(this->left);
		free(this->left);
	}
	
	if(this->right) {
		OperatorTree_destroy(this->right);
		free(this->right);
	}
}

OperatorTree * operators = NULL;
OperatorTree * global_operators = NULL;

void Ops_add(int precedence, Symbol name, assoc associativity) {
	OperatorTree ** node = &operators;
	
	int cmp;
	while (*node && Symbol_ne(name, (*node)->s)) {
		if (Symbol_gt(name, (*node)->s)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->s)) 
			node = &((*node)->left);
	}
	
	if(!(*node)) *node = OperatorTree_new(name, precedence, associativity);
	else (*node)->op = operator_description_make(precedence, associativity);
}

void GlobalOps_add(int precedence, Symbol name, assoc associativity) {
	OperatorTree ** node = &global_operators;
	
	int cmp;
	while (*node && Symbol_ne(name, (*node)->s)) {
		if (Symbol_gt(name, (*node)->s)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->s)) 
			node = &((*node)->left);
	}
	
	if(!(*node)) *node = OperatorTree_new(name, precedence, associativity);
	else (*node)->op = operator_description_make(precedence, associativity);
}

OperatorTree * Ops_search(Symbol name) {
	OperatorTree ** node = &operators;
	
	int cmp;
	while (*node && Symbol_ne(name, (*node)->s)) {
		if (Symbol_gt(name, (*node)->s)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->s)) 
			node = &((*node)->left);
	}
	
	return *node;
}

OperatorTree * GlobalOps_search(Symbol name) {
	OperatorTree ** node = &global_operators;
	
	int cmp;
	while (*node && Symbol_ne(name, (*node)->s)) {
		if (Symbol_gt(name, (*node)->s)) 
			node = &((*node)->right);
		else if (Symbol_lt(name, (*node)->s)) 
			node = &((*node)->left);
	}
	
	return *node;
}

operator_description Ops_get(Atom a) {
	OperatorTree * ot;
	if(a.type == SymbolType && (ot = Ops_search(a.d._Symbol))) {
		return ot->op;
	} else return operator_description_make(0, la);
}

operator_description GlobalOps_get(Atom a) {
	OperatorTree * ot;
	if(a.type == SymbolType && (ot = GlobalOps_search(a.d._Symbol))) {
		return ot->op;
	} else return operator_description_make(0, la);
}

void init_operators() {
	Ops_add(7, symbol("::"), la);
		
	Ops_add(6, symbol("."), la);
		
	Ops_add(5, symbol("<<"), ra);
	Ops_add(5, symbol(">>"), la);
		
	Ops_add(4, symbol("*"), va);
	Ops_add(4, symbol("/"), la);
	Ops_add(4, symbol("%"), la);
	
	Ops_add(3, symbol("+"), va);
	Ops_add(3, symbol("-"), la);
	
	Ops_add(2, symbol("<"), la);
	Ops_add(2, symbol(">"), la);
	Ops_add(2, symbol("<="), la);
	Ops_add(2, symbol(">="), la);
	Ops_add(2, symbol("=="), la);
	Ops_add(2, symbol("!="), la);
	
	Ops_add(1, symbol("&&"), la);
	Ops_add(1, symbol("||"), la);
	
	//Ops_add(0, symbol("cons"), ra);
	
	Ops_add(-99, symbol("="), ra);
	Ops_add(-99, symbol(":="), ra);
	
	// global
	
	GlobalOps_add(7, symbol("::"), la);
	
	GlobalOps_add(6, symbol("."), la);
	
	GlobalOps_add(5, symbol("<<"), ra);
	GlobalOps_add(5, symbol(">>"), la);
	
	GlobalOps_add(-99, symbol("="), ra);
	GlobalOps_add(-99, symbol(":="), ra);
};

void delete_operators() {
	if(operators) {
		OperatorTree_destroy(operators);
		free(operators);
	}
	
	if(global_operators) {
		OperatorTree_destroy(global_operators);
		free(global_operators);
	}
}

Atom global_infix_transform_ifl(Atom list);

Cons * global_infix_transform(Cons * list) {
	Cons * el = NULL, * prev = NULL;
	if((el = list) == NULL) return list;
	else if ((Cons_next(list)) == NULL) {
		Cons * l = NULL;
		Cons_add(&l, global_infix_transform_ifl(list->car));
		return l;
	}
	
	OperatorTree * o;
	// if(el->content().isa<symbol>())
	
	Cons * out = NULL;
	bool notcar = false;
	while(el != NULL) {
		if(notcar && Cons_next(el) != NULL 
			&& el->car.type == SymbolType
			&& (o = GlobalOps_search(el->car.d._Symbol))) {
			
			Cons * opexp = NULL;
			Cons_add(&opexp, ASymbol(s_expr));
			/*if(out.last()->car.isa<gc_ptr<Cons> >())
				opexp.add(Atom(global_infix_transform(out.last()->car.getList())));
			else
				*/
			Cons_add(&opexp, Cons_last(out)->car);
			/*opexp.add(el->content());
			opexp.add(el->next()->content());
			
			el = el->next()->next();*/
			
			do {
				
				Cons_add(&opexp, el->car);
				//opexp.add(el->next()->content());
				Cons_add(&opexp, global_infix_transform_ifl(Cons_next(el)->car));
				
				el = Cons_next(Cons_next(el));
			} while(el && Cons_next(el)
				&& el->car.type == SymbolType
				&& (o = GlobalOps_search(el->car.d._Symbol)));
			
			Cons_last(out)->car = ACons(opexp);
		} else {
			//out.add(el->content());
			Cons_add(&out, global_infix_transform_ifl(el->car));
			el = Cons_next(el);
			notcar = true;
		}
	}
	
	return out;
}

Atom global_infix_transform_ifl(Atom list) {
	if(list.type == ConsType && list.d._Cons  && (list.d._Cons->car.type != SymbolType || Symbol_ne(list.d._Cons->car.d._Symbol, s_expr)))
		return ACons(global_infix_transform(list.d._Cons));
	else
		return list;
}

Atom git(Cons * args) {
	return ACons(global_infix_transform(args));
}

void expr_helper(ASTACK * operators, ASTACK * operands, ASTACK * operand_dests) {
	
	Atom operand2 = AS_TOP(*operands);
	AS_POP(*operands);
	AS_POP(*operand_dests);
						
	Atom operand1 = AS_TOP(*operands);
	Cons * operand_destination = AS_TOP(*operand_dests).d._Cons;
	AS_POP(*operands);
	AS_POP(*operand_dests);
	
	// MAKE ATOM COPARISON OPS!!!!!!!
	
	if (operand_destination && operand1.d._Cons->car.type == AS_TOP(*operators).type
							&& Symbol_eq(operand1.d._Cons->car.d._Symbol, AS_TOP(*operators).d._Symbol)) {
		// variadic associative
		// 4 + 3 + 2 + 1 becomes (+ 4 3 2 1)
		if(Ops_get(AS_TOP(*operators)).associativity == va) {
			operand_destination = (*Cons_nextr(operand_destination) = Cons_new(operand2, NULL));
			AS_PUSH(*operands, operand1); // modified to include operand2
			AS_PUSH(*operand_dests, ACons(operand_destination));
		}
							
		// right associative
		// 4 + 3 + 2 + 1 becomes (+ 4 (+ 3 (+ 2 1)))
		else if (Ops_get(AS_TOP(*operators)).associativity == ra) {
			Cons * sexp = NULL;
			Cons * tmp = (sexp = Cons_new(operand2, sexp));
			sexp = Cons_new(operand_destination->car, sexp);	
			sexp = Cons_new(AS_TOP(*operators), sexp);
			operand_destination->car = ACons(sexp);
			
			operand_destination = tmp;	
			
			//operands.push(Atom(sexp));
			
			AS_PUSH(*operands, operand1); // modified to include operand2
			AS_PUSH(*operand_dests, ACons(operand_destination));
		}
		
		AS_POP(*operators);
	}
							
	// left associative
	// 4 + 3 + 2 + 1 becomes (+ (+ (+ 4 3) 2) 1)
	// or first application
	// 5 # 6 becomes (# 5 6) regardless of #'s associativity
	else {
		Cons * sexp = NULL;
		Cons * tmp = (sexp = Cons_new(operand2, sexp));
		sexp = Cons_new(operand1, sexp);
		sexp = Cons_new(AS_TOP(*operators), sexp);
		
		operand_destination = (Ops_get(AS_TOP(*operators)).associativity == la)
			? NULL
			: tmp;
		
		AS_POP(*operators);
		
		AS_PUSH(*operands, ACons(sexp));
		AS_PUSH(*operand_dests, ACons(operand_destination));
	}
}

Atom expr(Cons * args) {
	//if(!optbl) optbl = new operator_table();
	
	args = Cons_next(args);
	
	assert(args);
	
	if(Cons_next(args) == NULL) return args->car;
	else {
		int l = Cons_length(args);
		
		if(l % 2) {
			Atom e = args->car;
			//Atom op = args.tail().head();
			
			ASTACK operators;
			ASTACK operands;
			
			ASTACK operand_dests;
			//ConsList output;
			
			int i = 0;
			
			Cons * operand_destination = NULL;
			//Atom last_op;
			
			foreach(op, args) {
				//operator_table::operator_description d;
				
				//While there is an operator B of higher or equal precidence than A at the top of the stack, pop B off the stack and append it to the output.
				//Push A onto the stack.
							
				if(i % 2) {
					while(AS_NOT_EMPTY(operators) && Ops_get(AS_TOP(operators)).precedence 
						>= Ops_get(op->car).precedence ) {
						
						 expr_helper(&operators, &operands, &operand_dests);
					} 
					
					AS_PUSH(operators, op->car);
					
				} else {
					AS_PUSH(operands, op->car);
					AS_PUSH(operand_dests, nil);
				}
				
				i++;
			}
			
			while(AS_NOT_EMPTY(operators)) {
				expr_helper(&operators, &operands, &operand_dests);
			}
			
			// dump(operands.top(), "");
			
			return AS_TOP(operands);
			
			
			
		} else assert(("bad no of arguments", false));
		
		
	}
}
