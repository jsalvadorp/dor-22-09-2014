#include "eval.h"

#define CALLCC 1

// if, define, lambda, set!, do, do-here
// at, set-at!

#include "forward.h"
#include "types.h"
#include "dictionary.h"
#include "cons.h"
#include "environment.h"
#include <assert.h>
#include <stdio.h>
#include "lambda.h"

#define HALT NULL

//void EVAL(Evaluator * this);

Lambda * listc = NULL; // listcopy lambda. for exclusive use of the evaluator, because it is
// uncollected. so that the evaluator may be independent of the standard library

void State_markGray(State * this) {
	if(this->env) GCObject_markGray((GCObject *) this->env);
	if(this->unevarg) GCObject_markGray((GCObject *) this->unevarg);
	if(this->evarg) GCObject_markGray((GCObject *) this->evarg);
	if(this->ip)GCObject_markGray((GCObject *) this->ip);
	Atom_markGray(&this->exp);
	//
}

void State_init(State * s) {
	s->env = NULL;
	s->exp = nil;
	s->unevarg = NULL;
	s->evarg = NULL;
	s->ip = NULL;
	
	s->instr = NULL;
}

void Continuation_grayChildren(GCObject * this) {
	Continuation * cont = (Continuation *) this;
	
	State_markGray(&cont->frame);
	if(cont->next) GCObject_markGray((GCObject *) cont->next);
}

void Continuation_destroy(GCObject * this) {
	
}

struct GCObject_vtbl_t Continuation_vtbl = {Continuation_grayChildren, Continuation_destroy};

/*Continuation * Continuation_new(Continuation * next) {
	Continuation * nc = malloc(sizeof(Continuation));
	nc->_GCObject_.vptr = &Continuation_vtbl;
	GCObject_INIT(nc)
	
	nc->next = next;
	
	if(next) {
		nc->env = next->env;
		nc->exp = next->exp;
		nc->unevarg = next->unevarg;
		nc->evarg = next->evarg;
		nc->ip = next->ip;
		nc->instr = next->instr;
	} else {
		nc->env = NULL;
		nc->exp = nil;
		nc->unevarg = NULL;
		nc->evarg = NULL;
		nc->ip = NULL;
		nc->instr = NULL;
	}
	
	return nc;
}*/

Continuation * Continuation_new(State * f, Continuation * next) {
	Continuation * nc = malloc(sizeof(Continuation));
	nc->_GCObject_.vptr = &Continuation_vtbl;
	GCObject_INIT(nc)
	
	nc->next = next;
	
	nc->frame = *f;
	
	return nc;
}

Evaluator Evaluator_make(bool debug) {
	Evaluator ev;
	ev.value = nil;
	ev.naughty = false;
	ev.debug = false;
	ev.quit = false;
	ev.next = NULL;
	return ev;
	// size_t stacksize?
	//bool quit not anymore
}

Atom Evaluator_eval(Evaluator * this, Atom exp, Environment * context) {
	State_init(&this->s);
	Evaluator_saveState(this, HALT); //a NULL s means halt. it's ok
	this->s.exp = exp;
	this->s.env = context;
	this->s.instr = EVAL;
	this->value = nil;
	Evaluator_trampoline(this);
	
	return this->value;
}

void Evaluator_trampoline(Evaluator * this) {
	while(!this->quit && this->s.instr) {
		//printf("pointer93: %p\n", this->s->instr);
		this->s.instr(this);
	}
}

Atom Evaluator_eval_seq(Evaluator * this, Cons * exps, Environment * context) {
	State_init(&this->s);
	Evaluator_saveState(this, HALT);
	this->s.ip = exps;
	this->s.env = context;
	this->s.instr = EVSE;
	this->value = nil;
	//printf("pointer104: %p\n", EVSE);
	Evaluator_trampoline(this);
	
	return this->value;
}

void Evaluator_saveState(Evaluator * this, instr_t return_to) {
	//puts("PUSH");
	this->next = Continuation_new(&this->s, this->next);
	this->next->frame.instr = return_to;
}

void Evaluator_restoreState(Evaluator * this) {
	//puts("POP");
	this->s = this->next->frame;
	this->next = this->next->next;
}

void Evaluator_markGray(Evaluator * this) {
	Atom_markGray(&this->value);
	
	State_markGray(&this->s);
	if(this->next) GCObject_markGray((GCObject *) this->next);
}

DictNode * bfs = NULL; // dictionary of instr_t's

Atom Ainstr_t(instr_t i) {
	Atom a;
	zerofill(&a.d);
	a.d._instr_t = i;
	return a;
}

const char * instr_name(instr_t instr) {
	if (instr == &EVAL)				return "EVAL";
	else if (instr == &EVAR)	 	return "EVAR";
	else if (instr == &EVAR_)		return "EVAR_";
	else if (instr == &EV1A)		return "EV1A";
	else if (instr == &EVSE)	 	return "EVSE";
	else if (instr == &EVSE_)		return "EVSE_";
	//else if (instr == &EVSE)	 	return "SCOP";
	//else if (instr == &EVSE_)		return "SCOP_";
	//else if (instr == &EVSE)	 	return "IMPO";
	//else if (instr == &EVSE_)		return "IMPO_";
	else if (instr == &CALL)		return "CALL";
	else if (instr == HALT)			return "HALT";
	//else if (instr == &LET)			return "LET";
	else if (instr == &IF)			return "IF";
	else if (instr == &IF_)			return "IF_";
	else if (instr == &DO)			return "DO";
	else if (instr == &DOH)			return "DOH";
	else if (instr == &LET)			return "LET";
	//else if (instr == &COND)		return "COND";
	//else if (instr == &COND_)		return "COND_";
	//else if (instr == &DMOD)		return "DMOD";
	//else if (instr == &MODU)		return "MODU";
	//else if (instr == &MODU_)		return "MODU_";
	//else if (instr == &REQU)		return "REQU";
	//else if (instr == &REQU_)		return "REQU_";
	else if (instr == &SET)	 	return "ASSI";
	else if (instr == &SET_)		return "ASSI_";
	//else if (instr == &PL)			return "PL";
	//else if (instr == &PL_)			return "PL_";
	else if (instr == &LMDA)		return "LMDA";
	else if (instr == &LMDA_)		return "LMDA_";
	else if (instr == &DEFI)		return "DEFI";
	//else if (instr == &DEFI_)		return "DEFI_";
	else if (instr == &OR)			return "OR";
	else if (instr == &AND)			return "AND";
	else if (instr == &AND_)		return "AND_";
	//else if (instr == &QUIT)		return "QUIT";
	//else if (instr == &QUOT)		return "QUOT";
	//else if (instr == &DMAC)		return "DMAC";
	//else if (instr == &DMAC_)		return "DMAC_";
	//else if (instr == &MACR)		return "MACR";
	//else if (instr == &MACR_)		return "MACR_";
	//else if (instr == &CMAC_)		return "CMAC_";
	else							return "????";
}

void init_bfs() {
	listc = Lambda_new_c_simple_uncollected(Alistcopy);
	
	DictNode_add(&bfs, symbol("if"), Ainstr_t(IF));
	DictNode_add(&bfs, symbol("define"), Ainstr_t(DEFI));
	DictNode_add(&bfs, symbol("def"), Ainstr_t(DEFI));
	DictNode_add(&bfs, symbol(":="), Ainstr_t(DEFI));
	DictNode_add(&bfs, symbol("set!"), Ainstr_t(SET));
	DictNode_add(&bfs, symbol("do"), Ainstr_t(DO));
	DictNode_add(&bfs, symbol("do-here"), Ainstr_t(DOH));
	DictNode_add(&bfs, symbol("lambda"), Ainstr_t(LMDA));
	DictNode_add(&bfs, symbol("fun"), Ainstr_t(LMDA));
	//DictNode_add(&bfs, symbol("\\"), Ainstr_t(LMDA));
	
	DictNode_add(&bfs, symbol("&&"), Ainstr_t(AND));
	DictNode_add(&bfs, symbol("||"), Ainstr_t(OR));
	DictNode_add(&bfs, symbol("let"), Ainstr_t(LET));
}

void delete_bfs() {
	free(listc);
	DictNode_destroy(bfs);
	free(bfs);
}

void EVAL(Evaluator * self) {
	if(self->debug) {
		printf("EVAL ");
		dump_inline(self->s.exp);
		printf("\n");
	}

	if (self->s.exp.type == ConsType) {
		self->s.evarg = NULL;
		self->s.unevarg = self->s.exp.d._Cons;
		self->s.instr = EV1A;
	} else if (self->s.exp.type == SymbolType) {
		const Atom * a = Environment_ev(self->s.env, self->s.exp.d._Symbol);
		assert(a);
		self->value = *a;
		Evaluator_restoreState(self);
	} else {
		self->value = self->s.exp;
		Evaluator_restoreState(self);
	}
}

void EVSE(Evaluator * self) {
	//puts("EVSE");
	if(self->s.ip == NULL) {
		Evaluator_restoreState(self);
	} else {
		if(Cons_next(self->s.ip)) 
			Evaluator_saveState(self, EVSE_);  
		
		self->s.exp = self->s.ip->car;
		self->s.instr = EVAL;
	}
}

void EVSE_(Evaluator * self) {
	//puts("EVSE_");
	self->s.ip = Cons_next(self->s.ip);
	self->s.instr = EVSE;
}

void EV1A(Evaluator * self) {
	//puts("EV1A");
	Atom * sf = NULL;
	
	if (self->s.unevarg == NULL) {
		self->value = nil;
		Evaluator_restoreState(self);
	} else if (self->s.unevarg->car.type == SymbolType) {
		Symbol x = self->s.unevarg->car.d._Symbol;
		
		if(Symbol_eq(x, s_quote)) {
			self->value = Cons_next(self->s.unevarg)->car;
			Evaluator_restoreState(self);
		} else if(sf = DictNode_search(bfs, x)) {
			
			self->value = nil;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = sf->d._instr_t;
			
			//printf("builtin %zu\n", sf->d._instr_t);
			//printf("found builtin: %s = %p", x.s->str, self->s.instr);
		} else {
			const Atom * sp = Environment_ev(self->s.env, x);
			
			
			//printf("symbol %s\n", x.s->str);
			assert(sp);
			
			//handle macros...
			
			//else
			
			self->s.evarg = Cons_new(*sp, self->s.evarg);
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAR;
			
			//self->s.unevarg = global_infix_transform(self->s.unevarg); of macro output
		}
	} else {
		Evaluator_saveState(self, EVAR_);
		self->s.exp = self->s.unevarg->car;
		self->s.instr = EVAL;
	}
}

void EVAR(Evaluator * self) {
	if(self->debug) {
		printf("EVAR ");
		dump_inline(ACons(self->s.evarg));
		printf("\n");
	}
	
	if (self->s.unevarg == NULL) {
		self->s.evarg = Cons_reversed(self->s.evarg);
		
		self->s.instr = CALL;
	} else {
		Evaluator_saveState(self, EVAR_);
		self->s.exp = self->s.unevarg->car;
		self->s.instr = EVAL;
	}
}

void EVAR_(Evaluator * self) {
	//puts("EVAR_");
	self->s.evarg = Cons_new(self->value, self->s.evarg);
	self->s.unevarg = Cons_next(self->s.unevarg);
	self->s.instr = EVAR;
}

/*
Cons * foreval_lambda_list(Cons * llist);
Cons * args_in_form(Cons * params, Cons * args);
bool params_args_to_env(Environment * target, Cons * params, Cons * args);
nl->parameters = &Lambda_CFUNC_NOPCHECK;
*/

void CALL(Evaluator * self) {
	if(self->debug) {
		printf("CALL ");
		dump_inline(ACons(self->s.evarg));
		printf("\n");
	}
	
	Atom head = self->s.evarg->car;
	
	if(head.type == LambdaType) {
		Lambda * l = head.d._Lambda;
		
		if (l->cimpl) {
			Cons * args = Cons_next(self->s.evarg);
			
			if(l->parameters != &Lambda_CFUNC_NOPCHECK) 
				args = args_in_form(l->parameters, args);
			
			Atom naughty;
			naughty.d._Evaluator = self;
			
			if(l->body == &Lambda_CFUNC_NAUGHTY) 
				args = Cons_new(naughty, args); // give naughty access to evaluator as hidden arg
				
			self->value = l->cimpl(args);
			
			if (!self->naughty) Evaluator_restoreState(self);
			else self->naughty = false;	//naughty function left its own instruction
		} else {
			self->s.env = Environment_new(NULL, l->parent);
			ini_lambda_env(l, self->s.env);
			bool good = params_args_to_env(self->s.env, l->parameters, args_in_form(l->parameters, Cons_next(self->s.evarg)));
			assert(good);
			self->s.instr = EVSE;
			self->s.ip = l->body;
			self->value = nil;
			// fill context with args
			// instr = evseq
			// ip = lambda body
			// self->value = nil
			
			//self->restoreState();
		}	
	} else if(head.type == ContinuationType) {
		self->value = Cons_next(self->s.evarg)->car;
		self->next = head.d._Continuation;
		Evaluator_restoreState(self);
	}
	
	else {
		assert(false);
	}
}

void DO (Evaluator * self) {
	self->s.env = Environment_new(NULL, self->s.env);
	self->s.instr = EVSE;
	self->s.ip = self->s.unevarg;
	self->value = nil;
}

void DOH (Evaluator * self) {
	self->s.instr = EVSE;
	self->s.ip = self->s.unevarg;
	self->value = nil;
}

void LET(Evaluator * self) {
	Cons * args = NULL;
	Cons * params = NULL;
	
	Cons ** src = self->s.unevarg->car.type == ConsType
		? &(self->s.unevarg->car.d._Cons)
		: &(Cons_at(self->s.unevarg, 1)->d._Cons);
	
	foreach(el, (*src)) {
		Cons_add(&params, el->car);
		el = Cons_next(el);
		Cons_add(&args, el->car);
	}
	
	//*src = params;
	
	
	Cons *lam = NULL;
	
	if(self->s.unevarg->car.type != ConsType) {
		Cons_add(&lam, self->s.unevarg->car);
		self->s.unevarg = Cons_next(self->s.unevarg);
	}
	
	Cons_add(&lam, ACons(params));
	self->s.unevarg = Cons_next(self->s.unevarg);
	
	Cons_append(&lam, self->s.unevarg);
	
	self->s.unevarg = Cons_new(ASymbol(s_lambda), lam);
	self->s.exp = ACons(Cons_new(ACons(self->s.unevarg), args));
	self->s.instr = EVAL;
}

void LMDA(Evaluator * self) {
	Evaluator_saveState(self, LMDA_);
	Cons * paraml = self->s.unevarg->car.type == ConsType
		? self->s.unevarg->car.d._Cons
		: Cons_at(self->s.unevarg, 1)->d._Cons;
	self->s.exp = ACons(Cons_new(ALambda(listc), foreval_lambda_list(paraml)));
	self->s.instr = EVAL;
}

void LMDA_(Evaluator * self) {
	Symbol name = symbol(NULL);
	
	if(self->s.unevarg->car.type != ConsType) {
		name = self->s.unevarg->car.d._Symbol;
		self->s.unevarg = Cons_next(Cons_next(self->s.unevarg)); // skip to body
	} else self->s.unevarg = Cons_next(self->s.unevarg);
	
	self->value = ALambda(Lambda_new(name, self->s.env, self->value.d._Cons, self->s.unevarg));
	
	Evaluator_restoreState(self);
}

void DEFI(Evaluator * self) {
	//puts("DEFI");
	if(self->s.unevarg->car.type == ConsType) { //define lambda
		//if(self->s.unevarg.head())
		
		Atom lambdaname = self->s.unevarg->car.d._Cons->car;
		Cons * parameters = Cons_next(self->s.unevarg->car.d._Cons);
		Cons * body = Cons_next(self->s.unevarg);
		
		// `(define (lambda ,parameters ,@body))
		
		self->s.unevarg = Cons_list(2, 
			lambdaname,
			ACons(
				Cons_new(ASymbol(s_lambda),
				Cons_new(lambdaname,
				Cons_new(ACons(parameters), 
				body)))
			)
		);
	} else if(!Cons_next(self->s.unevarg)) { // if no rhs
		assert(self->s.unevarg->car.type == SymbolType);
		
		DictNode ** hn = Environment_evOrAdd(self->s.env, self->s.unevarg->car.d._Symbol);
		
		if(hn == NULL) {
			*hn = DictNode_new(self->s.unevarg->car.d._Symbol, nil);
		}
		
		self->value = nil;
		Evaluator_restoreState(self);
		return;
	}
	
	assert(self->s.unevarg->car.type == SymbolType);
	Environment_define(self->s.env, self->s.unevarg->car.d._Symbol, nil); 
	self->s.instr = SET;
}

void SET(Evaluator * self) {
	assert(self->s.unevarg->car.type == SymbolType);
	
	self->s.exp = *Cons_at(self->s.unevarg, 1);
	self->s.instr = EVAL;
	Evaluator_saveState(self, SET_);
}

void SET_(Evaluator * self) {
	Environment_setp(self->s.env, self->s.unevarg->car.d._Symbol, self->value);
	Evaluator_restoreState(self);
}

void IF(Evaluator * self) {
	if(self->s.unevarg == NULL) {
		self->value = nil;
		Evaluator_restoreState(self);
	} else {
		//if(self->s.unevarg.head())
		
		if(Cons_next(self->s.unevarg)) {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate condition
			Evaluator_saveState(self, IF_); 
		} else {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate else clause
			// this is the else clause. thus, a tail call.
		}
	}
}

void IF_(Evaluator * self) {
	if(Atom_as_bool(&self->value)) {
		//puts("yay");
		self->s.exp = self->s.unevarg->car;
		
		self->s.instr = EVAL;
	} else {
		self->value = nil;
		
		self->s.unevarg = Cons_next(self->s.unevarg);
		self->s.instr = IF;
	}
	
	//self->s.ip = self->s.ip->next();
	//self->s.instr = &EVSE;
}

void OR(Evaluator * self) {
	if(Atom_as_bool(&self->value)) {
		self->value = Abool(true);
		Evaluator_restoreState(self);
	} else if(self->s.unevarg == NULL) {
		self->value = Abool(false);
		Evaluator_restoreState(self);
	} else {
		if(Cons_next(self->s.unevarg)) {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate condition
			Evaluator_saveState(self, OR); 
		} else {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate else clause
			// this is the last clause. thus, a tail call.
		}
	}
}

void AND(Evaluator * self) {
	if(self->s.unevarg == NULL) {
		self->value = Abool(false);
		Evaluator_restoreState(self);
	} else {
		self->value = Abool(true);
		self->s.instr =AND_;
	}
}

void AND_(Evaluator * self) {
	if(!Atom_as_bool(&self->value)) {
		self->value = Abool(false);
		Evaluator_restoreState(self);
	} else {
		if(Cons_next(self->s.unevarg)) {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate condition
			Evaluator_saveState(self, AND_); 
		} else {
			self->s.exp = self->s.unevarg->car;
			self->s.unevarg = Cons_next(self->s.unevarg);
			self->s.instr = EVAL; //evaluate else clause
			// this is the last clause. thus, a tail call.
		}
	}
}


