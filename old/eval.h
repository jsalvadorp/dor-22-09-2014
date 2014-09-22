#ifndef EVAL_H
#define EVAL_H

// if, define, lambda, set!, do, do-here
// at, set-at!

#include "gc.h"
#include "forward.h"
#include "types.h"
#include "dictionary.h"

typedef struct State {
	Environment * env;
	Atom exp; // the expression to evaluate
	Cons * unevarg;
	Cons * evarg;
	Cons * ip;
	
	instr_t instr;
} State;

struct Evaluator { // GC-Collectable? evaluar posibilidades
	State s;
	
	Continuation * next;
	
	Atom value;

	bool naughty;
	bool debug;
	// size_t stacksize?
	bool quit;
};

struct Continuation { // an evaluation frame
	GCObject_HEAD

	State frame;
	
	Continuation * next;
};

Continuation * Continuation_new(State * f, Continuation * next);
void Continuation_grayChildren(GCObject * this);
void Continuation_destroy(GCObject * this);

Evaluator Evaluator_make(bool debug);
Atom Evaluator_eval(Evaluator * this, Atom exp, Environment * context);
Atom Evaluator_eval_seq(Evaluator * this, Cons * exps, Environment * context);

void Evaluator_saveState(Evaluator * this, instr_t return_to);
void Evaluator_restoreState(Evaluator * this);
void Evaluator_trampoline (Evaluator * this);
void Evaluator_markGray(Evaluator * this);

extern DictNode * bfs; // dictionary of instr_t's

void init_bfs();
void delete_bfs();

void EVAL(Evaluator * this);
void EVSE(Evaluator * this);
void CALL(Evaluator * this);
void EVSE_(Evaluator * this);
//void CALL(Evaluator * this);
void EVAR(Evaluator * this);
void EVAR_(Evaluator * this);
void EV1A(Evaluator * this);
void DEFI(Evaluator * this);
void SET(Evaluator * this);
void SET_(Evaluator * this);
void LMDA(Evaluator * this);
void LMDA_(Evaluator * this);
void IF(Evaluator * this);
void IF_(Evaluator * this);
void OR(Evaluator * this);
void AND(Evaluator * this);
void AND_(Evaluator * this);
void DO(Evaluator * this);
void DOH(Evaluator * this);
void LET(Evaluator * this);

const char * instr_name(instr_t i);


//const char * instr_name(void (*instr) (Evaluator *));
/*

	
	atom eval(atom what, gc_ptr<Environment> context) {
		saveState(HALT);
		s.exp = what;
		s.env = context;
		s.instr = &EVAL;
		value = nil;
		trampoline();
		return value;
	}
	
	atom eval(ConsList what, gc_ptr<Environment> context) {
		saveState(HALT);
		s.ip = what.first_;
		s.env = context;
		s.instr = &EVSE;
		value = nil;
		trampoline();
		//cout << "self->value is" << value.toREPLString() << endl;
		
		s.unevarg = ConsList();
		s.evarg = ConsList();
		s.ip = NULL;
		s.env = NULL;
		
		return value;
	}
	~Evaluator() {
		delete stack;	
	}
	
	void printSMargin() {
		//printf(":::: %6zu ::  ", stack->size());
		printf("::::::::::::  ");
	}
	
	void printMargin() {
		printf("::::::::::::::  ");
	}
	
	void saveState(void (*ret_to) (Evaluator *)) {
		if(debug) { printSMargin(); printf("%-9s", "PUSH "); cout << instr_name(ret_to) << endl; }
		
		stack->push(s);
		stack->topi()->instr = ret_to;
	}
	
	void restoreState() {
		if(debug) { printSMargin(); printf("%-9s", "POP"); cout << endl;}
		
		stack->popTo(s);
	}
	
	void trampoline () {
		//cout << "::::::::SIZE OF istate = " << sizeof(istate) << endl;
		while(s.instr && !quit) {
			s.instr(this);
		}
	}
	
	gc_ptr<Continuation> getCC() {
		return new Continuation(stack->top);
	}
	
	void markGray() {
		value.markGray();
		
		s.markGray();
		
		stack->top->markGray();
	}
};

extern Hash<symbol, void (*) (Evaluator *)> bfs;

inline void init_bfs() {
	bfs.add(s_do, &DO);
	bfs.add(s_if, &IF);
	bfs.add(s_fn, &LMDA);
	bfs.add(symbol("\\"), &LMDA);
	bfs.add(symbol("lambda"), &LMDA);
	bfs.add(s_cond, &COND);
	bfs.add(s_module, &MODU);
	bfs.add(s_define, &DEFI);
	bfs.add(s_assign, &ASSI);
	bfs.add(symbol("set!"), &ASSI);
	bfs.add(s_scopeop, &SCOP);
	bfs.add(s_import, &IMPO);
	bfs.add(symbol(":="), &DEFI);
	//bfs.add(symbol("require"), &REQU);
	bfs.add(symbol("quit"), &QUIT);
	bfs.add(symbol("||"), &OR);
	bfs.add(symbol("&&"), &AND);
	bfs.add(symbol("let"), &LET);
	bfs.add(symbol("quote"), &QUOT);
	// bfs.add(symbol("defmodule"), &DMOD);
	
	bfs.add(symbol("macro"), &MACR);
	bfs.add(symbol("do-here"), &DOH);
	
	//bfs.add(symbol("setcar"), _scar);
	//bfs.add(symbol("setcdr"), _scdr);
}

#endif*/

#endif
