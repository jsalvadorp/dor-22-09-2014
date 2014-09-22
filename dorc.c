#include <stdio.h>

#include "../dc/opcodes.h"

#include "read3.h"

//#include "analysis.h"

// {} should be the value of the unit type

// CONSIDER RIGHT TO LEFT ARGEVAL! could simplify 
// currying...	|		|		||		|		

// lvalues
// comparison of types consider memoizing type creating functions	
// structural comparison of tuples and function types	
// unit type, isomporphic with void but with one value	
// interfacing with c, ffi and dlopen...cd devel	
// escape analysis, closures	


#define MAX_NULLARY_CONSTRUCTORS 1024

char gcode[200000];
int codepos;

// types!!
//char poolt[65536];
int pool[65536];
bool arity[65536];
#define UNKNOWNAR -1
int pools = 0;

int entry;

//Symbol globnames[65536];

Symbol ursym[1024];
int urloc[1024];
// urinst??
int urc;

typedef struct {
	int size; // in words
	int compactsize; // in bytes if positive, words if negative
	int tag;
	//bool heap 
} types;

typedef struct {
	int n;
	types ** nt;
} typel;

// for each compiled function, show up to which parameter it is compiled

typedef struct {
	types t;
	
	types * takes;
	types * rets;
} fts;
#define FTS 0

typedef struct {
	types t;
	
	int * cnsizes;
	int * cnconsts;
	
	int c0; // number of nullary constructors
	int cn;
} uts;
#define UTS 1

typedef struct {
	types t;
	int cs;
	
	typel els;
} tuplets;
#define TTS 2

typedef struct {
	types t;
	
	types ** pointed;
} tvars;
#define TVAR 99

types * tvar(types * pointed) {
	tvars * tv = malloc(sizeof(tvars));
	tv->t.tag = TVAR;
	tv->pointed = NULL;
	return (types *) tv;
}

typedef struct {
	Symbol name;
	types * type;
} binding;

binding globalc[65536];
bool hasvalue[65536];

bool type_eq(types * a, types * b) {
	if(a->tag != b->tag) return false;
	if(a == b) return true;
	
	if(a->tag == TTS) {
		tuplets * ta = (tuplets *) a;
		tuplets * tb = (tuplets *) b;
		
		if(ta->els.n != tb->els.n) return false;
		
		for(int i = 0; i < ta->els.n; i++) {
			if(!type_eq(ta->els.nt[i], tb->els.nt[i])) return false;
		}
		
		return true;
	} else if(a->tag == FTS) {
		fts * fa = (fts *) a;
		fts * fb = (fts *) b;
		
		return type_eq(fa->takes, fb->takes) && type_eq(fa->rets, fb->rets);
	}
	
	// unions are not compared structurally. 
	
	return false;
}

fts * functype(types * car, types * cdr) {
	fts * f = malloc(sizeof(fts));
	((types *)f)->tag = FTS;
	((types *)f)->size = 1;
	
	f->takes = car;
	f->rets = cdr;
	
	return f;
}

// how to compare nameless types...
// int, int must return the same type anywhere it appears

// memoize type returning functions...


bool tiscomplete(types * s) {
	if(s == NULL) return false;
	
	bool complete = true;
	
	switch(s->tag) {
	/*case UTS: {
		uts * u = (uts *) s;
		for(int i = 0; i < u->cn; i++) {
			for(int j = 0; j < u->cnparams[i].n; j++) {
				complete = complete && tiscomplete(u->cnparams[i].nt[j]);
			}
		}
		break;
		}*/
		
	}
	
	return complete;
}

Symbol typenames[1024];
int typec;
types * gtypes[1024];

#define MAXPSIZE 65536
#define MAXPLOCS 1024
/*
typedef types * typesptr;
define_hmap_interface(Symbol, typesptr);
define_hmap_implementation(Symbol, typesptr, Symbol_hash, Symbol_eq);



typedef struct {
	hmap_Symbol_typesptr t;
	typenvs * parent;
} typenvs;

types * ctexpr2(ASTAtom a, typenvs * e) {
	
}*/

typedef struct {
	char code[MAXPSIZE];
	int cpos;
	//Symbol locnames[MAXPLOCS];
	
	binding locals[MAXPLOCS];
	
	bool deconstruct[MAXPLOCS];
	ASTAtom rexp[MAXPLOCS];
	
	Symbol lursym[1024];
	int lurloc[1024];
	int lurc;
	
	// max ops???
	int nargs;
	int nlocs;
	
	int toplocs;
	int scopelevel;
	
	int topops;
	int exps; // will be substituted when there is a type system
} procs;

// bitfield representations to save up tons of space..?


types * t_int, * t_void, * t_bool; // bool should be a union type

void init_types() {
	t_int = malloc(sizeof(types));
	t_int->tag = -1;
	t_int->size = 1;
	t_int->compactsize = -1;
	
	t_bool = malloc(sizeof(types));
	t_bool->tag = -1;
	t_bool->size = 1;
	t_bool->compactsize = -1;
	
	t_void = malloc(sizeof(types));
	t_void->tag = -1;
	t_void->size = 0;
	t_void->compactsize = 0;
	
	typenames[typec] = symbol("int");
	gtypes[typec] = t_int;
	typec++;
	
	typenames[typec] = symbol("bool");
	gtypes[typec] = t_bool;
	typec++;
	
	typenames[typec] = symbol("void");
	gtypes[typec] = t_void;
	typec++;
	
	//fprintf(stderr, "types %p int %p bool %p void\n", t_int, t_bool, t_void);
}


/*
 * 
 *TODO
 * types. ADTs, function types
 * closures
 * constant folding
 * lambda lifting, closure avoidance, function inlining
 * static and dynamic allocation of structures and optimization
 * */



#define WRITEI32(y) \
	{unsigned int x = y; putchar((x >> 24) & 0xFF); putchar((x >> 16) & 0xFF); putchar((x >> 8) & 0xFF); putchar((x) & 0xFF);}

#define GETI32(src, pos) \
	((src[pos] << 24) | (src[pos + 1] << 16) | (src[pos + 2] << 8) | src[pos + 3])


#define SETI32(src, pos, val) \
	{src[pos] = (val >> 24) & 0xFF; src[pos + 1] = (val >> 16) & 0xFF; src[pos + 2] = (val >> 8) & 0xFF; src[pos + 3] = (val) & 0xFF; }

// typing!!!!!!!!
int finds(Symbol * tbl, int size, Symbol s) {
	for(int i = 0; i < size; i++) if(Symbol_eq(tbl[i], s)) return i;
	
	return -1;
}

int findb(binding * tbl, int size, Symbol s) {
	//for(int i = 0; i < size; i++) if(Symbol_eq(tbl[i].name, s)) return i;
	
	//return -1;
	int i;
	for(i = size - 1; i >= 0 && !Symbol_eq(tbl[i].name, s); i--);
	return i;
}
// trusting char is 8 bits for the masking/modulus...

void emit(char * dest, int offset, int val) {
	dest[offset] = val;
}

void emit2(char * dest, int offset, int val) {
	dest[offset] = val >> 8;
	dest[offset + 1] = val;
}

void emit4(char * dest, int offset, int val) {
	dest[offset] = val >> 24;
	dest[offset + 1] = val >> 16;
	dest[offset + 2] = val >> 8;
	dest[offset + 3] = val;
}


Symbol urtn[1024];
types ** urt[1024];
int urtc;

bool issym(ASTAtom a, Symbol s) {
	return a.type == SymbolType && Symbol_eq(a.d._Symbol, s);
}

bool headis(ASTAtom a, Symbol s) {
	return a.type == ASTListType 
		&& a.d._ASTList
		&& issym(a.d._ASTList->car, s);
}

void addtref(Symbol tn, types ** place) {
	int i;
	for(i = 0; urtn[i].s && i < urtc; i++);
	
	urtn[i] = tn;
	urt[i] = place;
	
	if(i == urtc) urtc++;
}

void resolvetref(Symbol tn, types * t) {
	for(int i = 0; i < urtc; i++) {
		if(Symbol_eq(urtn[i], tn)) {
			*urt[i] = t;
			urtn[i].s = NULL;
		}
	}
}

int cctor(Symbol name, types * f, int cnsize, int constant) {
	int c = findb(globalc, pools, name);
	assert(c == -1);
	
	globalc[pools].name = name;
	globalc[pools].type = (types *) f;
	arity[pools] = cnsize;
	pool[pools] = codepos;
	
	SETI32(gcode, codepos + P_NARGS, cnsize);
	SETI32(gcode, codepos + P_NLOCS, 0);
	int psizeloc = codepos + P_PSIZE;
	
	codepos += PROCSTART;
	int opos = codepos;
	
	for(int i = cnsize - 1; i >= 0; i--) {
		emit(gcode, codepos++, O_lda);
		emit2(gcode, codepos, i);
		codepos += 2;
	}
	
	emit(gcode, codepos++, O_ldi);
	emit4(gcode, codepos, constant);
	codepos += 4;
	
	emit(gcode, codepos++, O_chunk);
	emit2(gcode, codepos, cnsize + 1);
	codepos += 2;
	
	emit(gcode, codepos++, O_ret);
	
	SETI32(gcode, psizeloc, codepos - opos);
	
	pools++;
	
	return pools - 1;
}

types * ctexpr(ASTAtom texp);

types * ctunion(ASTList * cases) { //
	//fprintf(stderr, "ctunion\n");
	uts * u = malloc(sizeof(uts));
	((types *)u)->tag = UTS;
	((types *)u)->size = 1;
	((types *)u)->compactsize = -1;
	u->cn = 0;
	u->c0 = 0;
	
	//binding ctors[1024];
	int cnsizes[1024];
	int cnconsts[1024];
	
	// if cn then assert(c0 < 1024) or something. to protect pointers
	
	foreach(c, cases) {
		ASTAtom a = c->car;
		
		if(a.type == SymbolType) { // nullary constructor
			//fprintf(stderr, "sym %s is %d\n", Symbol_getstr(a.d._Symbol), u->c0);
			// assert it is a symbol!
			int c = findb(globalc, pools, a.d._Symbol);
			assert(c == -1);
			globalc[pools].name = a.d._Symbol;
			globalc[pools].type = (types *)u;
			pool[pools++] = u->c0++;
			
		} else if(a.type == ASTListType) { // nary constructor
			//u->cn++;
			// read name and args
			// make global constructor function
			
			// assert it is a symbol
			Symbol name = a.d._ASTList->car.d._Symbol;
			int cnsize = 0;
			types * f = NULL, ** fp = &f;
			
			foreach(el, a.d._ASTList->next) {
				types * t = ctexpr(el->car);
				*fp = (types *) functype(t, NULL);
				if(t == NULL) {
					assert(el->car.type == SymbolType);
					addtref(el->car.d._Symbol, &(((fts *)(*fp))->takes));
				}
				fp = &(((fts *)*fp)->rets);
				
				//cnsize += t->size;
				cnsize++;
			}
			
			*fp = (types *) u;
			
			cnsizes[u->cn] = cnsize;
			cnconsts[u->cn] = cctor(name, f, cnsize, u->cn);
			u->cn++;
			
			//ctors[u->cn].name = name;
			//ctors[u->cn++].type = f;
		}
	}
	
	if(u->cn) assert(u->c0 < MAX_NULLARY_CONSTRUCTORS);
	
	u->cnsizes = malloc(u->cn * sizeof(int));
	u->cnconsts = malloc(u->cn * sizeof(int));
	
	for(int i = 0; i < u->cn; i++) {
		u->cnsizes[i] = cnsizes[i];
		u->cnconsts[i] = cnconsts[i];
	}
	
	return (types *) u;
}



types * cttuple(ASTList * elements) { //
	int nels = ASTList_length(elements);
	//fprintf(stderr, "ctunion\n");
	tuplets * u = malloc(sizeof(tuplets));
	((types *)u)->tag = TTS;
	((types *)u)->size = 1;
	((types *)u)->compactsize = -1;
	u->els.n = nels;
	u->els.nt = malloc(nels * sizeof(types *));
	u->cs = 0;
	
	
	// if cn then assert(c0 < 1024) or something. to protect pointers
	int i = 0;
	foreach(el, elements) {
		u->els.nt[i] = ctexpr(el->car);
		u->cs += u->els.nt[i]->size;
		i++;
	}
	
	return (types *) u;
}

types * ctfmap(ASTList * cons) { //
	//fprintf(stderr, "ctfmap\n");
	fts * u = functype(NULL, NULL);
	
	u->takes = ctexpr(cons->car);
	u->rets = ctexpr(*ASTList_at(cons, 1));
	
	return (types *) u;
}

types * ctexpr(ASTAtom texp) {
	if(texp.type == SymbolType) { // nullary constructor
		int c;
		if((c = finds(typenames, typec, texp.d._Symbol)) != -1)
			return gtypes[c];
		else return NULL;
	} else if(texp.type == ASTListType) { // nary constructor
		ASTAtom head = texp.d._ASTList->car;
		
		if(issym(head, symbol("|"))) {
			return ctunion(texp.d._ASTList->next);
		} else if(issym(head, symbol("->"))) {
			return ctfmap(texp.d._ASTList->next);
		} else if(issym(head, symbol(","))) {
			return cttuple(texp.d._ASTList->next);
		}
	}
}

void ctdef(ASTList * cons) {
	Symbol lhs;
	bool stored = false;
	
	if(cons->car.type == SymbolType)
		lhs = cons->car.d._Symbol;
	
	int c;
	if((c = finds(typenames, typec, lhs)) != -1) {
		fprintf(stderr, "%d:%d: error: duplicate type %s\n", cons->car.line, cons->car.column, Symbol_getstr(lhs));
		assert(0);
	}
	
	types * s = ctexpr(*ASTList_at(cons, 1));
	
	typenames[typec] = lhs;
	gtypes[typec] = s;
	
	if(s == NULL) {
		assert(0); // should not happen
		//addtref(lhs, gtypes + typec); // wrong
	} else {
		resolvetref(lhs, s);
	}
	
	typec++;
	
	//fprintf(stderr, "defined type %s\n", Symbol_getstr(lhs));
	
}

types * cexpr(ASTAtom exp, procs * s, bool emitting);


types * cblock(ASTList * block, procs * s, bool newscope, bool emitting) { // tail call!
	// handle locals!!!!!
	int oldlevel;
	if(newscope) {
		oldlevel = s->scopelevel;
		s->scopelevel = s->nlocs;
	}
	
	types * rettype = t_void;
	
	foreach(stmt, block) { // what about tail calls. what
		if(emitting)
		switch(rettype->size) {
		case 4: // 4 words
			emit(s->code, s->cpos++, O_popq);
			break;
		case 2: // two words
			emit(s->code, s->cpos++, O_popl);
			break;
		case 1: // one word
			emit(s->code, s->cpos++, O_pop);
			break;
		case 0: // one word
			break;
		}
		
		rettype = cexpr(stmt->car, s, emitting);
		assert(rettype);
	}
	
	
	if(newscope) {
		s->nlocs = s->scopelevel;
		s->scopelevel = oldlevel;
		
	}
	
	return rettype;
}

types * cclause(ASTList * clause, procs * s, bool emitting) {
	assert(clause->car.type == ASTListType);
	assert(issym(clause->car.d._ASTList->car, symbol("=>")));
	assert(clause->car.d._ASTList->next);
	assert(clause->car.d._ASTList->next->next);
	assert(!clause->car.d._ASTList->next->next->next);
	ASTAtom condition = clause->car.d._ASTList->next->car, 
			body = clause->car.d._ASTList->next->next->car;
	
	//ASTAtom condition = clause->car.d._ASTList->car;
	//ASTList * block = clause->car.d._ASTList->next;
	
	types * rettype = t_void;
	
	if(clause->next) {
		assert(!(issym(condition, symbol("else")) || issym(condition, symbol("true"))));
		assert(type_eq(cexpr(condition, s, emitting), t_bool));
		
		int fjump;
		if(emitting) {
		emit(s->code, s->cpos++, O_brf);
		fjump = s->cpos;
		emit2(s->code, s->cpos, 0);
		s->cpos += 2;
		}
		
		rettype = cexpr(body, s, emitting);//cblock(block, s, true, emitting);
		
		int endaddr;
		if(emitting) {
		emit(s->code, s->cpos++, O_bra);
		endaddr = s->cpos;
		emit2(s->code, s->cpos, 0);
		s->cpos += 2;
		}
	
		if(emitting) emit2(s->code, fjump, s->cpos);
		assert(type_eq(rettype, cclause(clause->next, s, emitting)));
		if(emitting) emit2(s->code, endaddr, s->cpos);
	} else {
		assert((issym(condition, symbol("else")) || issym(condition, symbol("true"))));
		assert(clause->next == NULL);
		
		rettype = cexpr(body, s, emitting);
	}
	
	return rettype;
}

// progra, excel, estudiar ams, estudiar lenguajes

types * cwhile(ASTList * cons, procs * s, bool emitting) {
	
	//fprintf(stderr, "while \n");
	ASTAtom condition = cons->car;
	ASTList * block = cons->next;
	
	int cmp = s->cpos;
	assert(type_eq(cexpr(condition, s, emitting), t_bool));
	
	int fjump;
	if(emitting) {
	emit(s->code, s->cpos++, O_brf);
	fjump = s->cpos;
	emit2(s->code, s->cpos, 0);
	s->cpos += 2;
	}
	
	types * rettype = cblock(block, s, true, emitting);
	
	if(emitting)
	switch(rettype->size) {
	case 4: // 4 words
		emit(s->code, s->cpos++, O_popq);
		break;
	case 2: // two words
		emit(s->code, s->cpos++, O_popl);
		break;
	case 1: // one word
		emit(s->code, s->cpos++, O_pop);
		break;
	case 0: // one word
		break;
	}
	if(emitting) {
	emit(s->code, s->cpos++, O_bra);
	emit2(s->code, s->cpos, cmp); // de vuelta
	s->cpos += 2;
	
	emit2(s->code, fjump, s->cpos);
	}
	
	return t_void;
}

binding cbinding(ASTAtom a) {
	binding b;
	
	if(a.type == SymbolType) {
		b.name = a.d._Symbol;
		b.type = NULL;
		return b;
	} else if(a.type == ASTListType) {
		if(issym(a.d._ASTList->car, symbol(":"))) {
			b.name = a.d._ASTList->car.d._Symbol;
			b.type = ctexpr(*ASTList_at(a.d._ASTList, 1));
			assert(tiscomplete(b.type));
			assert(b.type->size);
			return b;
		}
	}
	
	assert(0);
}

typedef struct {
	types * type;
	int globref;
	int value;
} constexpr;

constexpr isconstexpr(ASTAtom exp) {
	// constant folding requires code execution 
	// during compile time. 
	// or we could delay computations till early runtime, some 
	// startup routine before entry...
	
	// semantic analyzer should detect any free variables in an expression
	// determine if they are bound by other variables (thus computation
	// is delayed till runtime) or just constants known at compile time
	// (fold them).
	
	constexpr cexp;
	cexp.globref = -1;
	cexp.type = NULL;
	
	if(exp.type == LongType) {
		cexp.type = t_int;
		cexp.value = exp.d._long;
	} else if(exp.type == CharType) {
		cexp.type = t_int;
		cexp.value = exp.d._char;
	} else if(exp.type == BoolType) {
		cexp.type = t_bool;
		cexp.value = exp.d._bool;
	} else if(exp.type == SymbolType) { //
		//emit(code, (*cpos)++, O_ldi);
		int c;
		if((c = findb(globalc, pools, exp.d._Symbol)) != -1) {
			cexp.type = globalc[c].type;
			cexp.globref = c;
		} 
	}
	
	return cexp;
}

types * clit(ASTAtom exp, procs * s, bool emitting);

typedef struct {
	int popsize;
	int nbrfs;
	int brflocs[1024];
	int remainders[1024];
} matchs;

void cbind1(binding lhs, procs * s, bool emitting, types *rt, bool new, int remaining) {
	if(lhs.type) assert(type_eq(lhs.type, rt));
	else lhs.type = rt;
	
	int dest;
	
	
	
	if(new) {
		if(findb(s->locals + s->scopelevel, s->nlocs - s->scopelevel, lhs.name) != -1) {
			
			fprintf(stderr, "Duplicate local %s\n", Symbol_getstr(lhs.name));
			assert(0);
		}
		
		dest = s->nlocs;
		s->nlocs += rt->size;
		s->locals[dest] = lhs;
		if(s->nlocs > s->toplocs) s->toplocs = s->nlocs;
		
		
	} else {
		dest = findb(s->locals, s->nlocs, lhs.name);
		
		if(dest == -1) {
			fprintf(stderr, "undefined local %s\n", Symbol_getstr(lhs.name));
			assert(0);
		}
	}
	
	//fprintf(stderr, "bound %s %d\n", Symbol_getstr(lhs.name), dest);
	
	if(emitting) {
		if(dest < s->nargs) {
			emit(s->code, s->cpos++, O_sta);
			emit2(s->code, s->cpos, dest);
		} else {
			emit(s->code, s->cpos++, O_stl);
			emit2(s->code, s->cpos, dest - s->nargs);
		} 
		s->cpos += 2;
	}
	
	
}

int findint(int * arr, int size, int val) {
	for(int i = 0; i < size; i++) if(arr[i] == val) return i;
	return -1;
}

void cbind(ASTAtom a, procs * s, bool emitting, types *rt, bool new, int remaining, matchs * matchtbl) { // deconstruct
	//fprintf(stderr, "cbind!1 %p\n", matchtbl);
	
	if(headis(a, symbol(","))) {
		ASTList * lhsl = a.d._ASTList->next;
		assert(rt->tag == TTS);
		
		tuplets * tru = (tuplets *) rt;
		
		remaining -= rt->size;
		
		if(emitting) {
			emit(s->code, s->cpos++, O_unchunk);
			emit2(s->code, s->cpos, tru->cs);
			s->cpos += 2;
		}
		
		remaining += tru->cs;
		
		int i = 0;
		foreach(lhs, lhsl) {
			assert(i < tru->els.n);
			cbind(lhs->car, s, emitting, tru->els.nt[i], new, remaining, matchtbl);
			remaining -= tru->els.nt[i]->size;
			i++;
		}
		
		assert(i == tru->els.n);
	} else if(a.type == ASTListType && rt->tag == UTS) {
		assert(matchtbl);
		ASTAtom x = a.d._ASTList->car;
		ASTList * lhsl = a.d._ASTList->next;
		int c;
		
		constexpr cexpr = isconstexpr(x);
		assert(cexpr.globref != -1);
		
		uts * urt = (uts *)rt;
		
		if((c = findint(urt->cnconsts, urt->cn, cexpr.globref)) != -1) {
			// c is the constant!
			if(emitting) {
				emit(s->code, s->cpos++, O_dup);
				emit(s->code, s->cpos++, O_ldi);
				emit4(s->code, s->cpos, MAX_NULLARY_CONSTRUCTORS);
				s->cpos += 4;
				emit(s->code, s->cpos++, O_clti);
				emit(s->code, s->cpos++, O_brf);
				
				if(remaining > matchtbl->popsize) matchtbl->popsize = remaining;
				
				matchtbl->brflocs[matchtbl->nbrfs] = s->cpos;
				matchtbl->remainders[matchtbl->nbrfs] = remaining;
				
				matchtbl->nbrfs++;
			
				emit2(s->code, s->cpos, 0);
				s->cpos += 2;
				
				remaining -= rt->size; // minus the unchunked
				
				emit(s->code, s->cpos++, O_unchunk);
				emit2(s->code, s->cpos, urt->cnsizes[c] + 1);
				s->cpos += 2;
				
				remaining += urt->cnsizes[c]; // plus the chunk, not counting the type tag thats about to be compared
				
				emit(s->code, s->cpos++, O_ldi);
				emit4(s->code, s->cpos, c); // matches the constructor tag
				s->cpos += 4;
				
				emit(s->code, s->cpos++, O_ceqi);
				emit(s->code, s->cpos++, O_brf);
				
				if(remaining > matchtbl->popsize) matchtbl->popsize = remaining;
				
				matchtbl->brflocs[matchtbl->nbrfs] = s->cpos;
				matchtbl->remainders[matchtbl->nbrfs] = remaining;
				
				matchtbl->nbrfs++;
			
				emit2(s->code, s->cpos, 0);
				s->cpos += 2;
				
				types * f = globalc[cexpr.globref].type;
				
				int i = 0;
				foreach(lhs, lhsl) {
					assert(f != rt);
					assert(f);
					assert(f->tag == FTS);
					assert(i < urt->cnsizes[c]);
					
					cbind(lhs->car, s, emitting, ((fts *)f)->takes, new, remaining, matchtbl);
					remaining -= ((fts *)f)->takes->size;
					
					i++;
					f = ((fts *)f)->rets;
				}
				
				assert(f == rt);
			}
		} else assert(0);
	} else {
		constexpr cexpr = isconstexpr(a);
		if(cexpr.type != NULL && matchtbl) {
			// comparison!!!!
			
			//fprintf(stderr, "COMPARISON!1 %p\n", matchtbl);
			
			if(emitting) {
				if(cexpr.globref != -1) {
					emit(s->code, s->cpos++, O_ldc);
					emit2(s->code, s->cpos, cexpr.globref);
					s->cpos += 2;
				} else {
					// literal
					
					clit(a, s, emitting);
				}
				
				emit(s->code, s->cpos++, O_ceqi); // USE COMPARISON FUNCTION!!!
				// or really? cause its just constvals
				
				emit(s->code, s->cpos++, O_brf);
				
				int r = remaining - 1;
				
				if(r > matchtbl->popsize) matchtbl->popsize = r;
				
				matchtbl->brflocs[matchtbl->nbrfs] = s->cpos;
				matchtbl->remainders[matchtbl->nbrfs] = r;
				
				matchtbl->nbrfs++;
			
				emit2(s->code, s->cpos, 0);
				s->cpos += 2;
			}
		} else cbind1(cbinding(a), s, emitting, rt, new, remaining);
	}
	
}

// is it an expression? NO!!
types * cvar(ASTList * cons, procs * s, bool emitting) {
	bool stored = false;
	
	//binding b = cbinding(cons->car);
	
	types * trhs = cexpr(*ASTList_at(cons, 1), s, emitting);
	
	cbind(cons->car, s, emitting, trhs, true, 1, NULL);
	
	return t_void;
}

types * cset(ASTList * cons, procs * s, bool emitting) {
	bool stored = false;
	
	//binding b = cbinding(cons->car);
	
	types * trhs = cexpr(*ASTList_at(cons, 1), s, emitting);
	
	cbind(cons->car, s, emitting, trhs, false, 1, NULL);
	
	return t_void;
}

bool isbinop(fts * f) {
	return (f && f->takes == t_int 
		&& f->rets && f->rets->tag == FTS 
		&& ((fts *)(f->rets))->takes == t_int
		&& ((fts *)(f->rets))->rets == NULL
			|| (((fts *)(f->rets))->rets == t_int)); 
}

bool iscmpop(fts * f) {
	return (f && f->takes == t_int 
		&& f->rets && f->rets->tag == FTS 
		&& ((fts *)(f->rets))->takes == t_int
		&& ((fts *)(f->rets))->rets == NULL
			|| (((fts *)(f->rets))->rets == t_bool)); 
}

types * matchfunct(types * funct, fts * call) {
	//if(((fts *) funct)->takes == t_void && call == NULL) return ((fts *) funct)->rets;
	
	while(call) {
		if(!funct || funct->tag != FTS) return NULL;
		if(!type_eq(call->takes, ((fts *)funct)->takes)) return NULL;
		
		// structurally compare the types...
		
		call = (fts *) call->rets;
		funct = (((fts *) funct)->rets);
	}
	
	return funct;
}

fts * righttoleft(ASTList * args, procs * s, bool emitting, int * argc, int * rems) {
	if(!args) {
		*rems = 0;
		return NULL;
	}
	
	types * r = (types *) righttoleft(args->next, s, emitting, argc, rems ? rems + 1 : NULL),
		* a = cexpr(args->car, s, emitting);
	
	if(rems) rems[0] = a->size ? rems[1] + a->size : 0;
	
	//(*realargc) += a->size;
	(*argc)++;
	return functype(a, r);
}
// handle function inlining!!!! to handle operators
// constant folding!!!!

types * clet(ASTList * cons, procs * s, bool emitting) {
	// see cfun, cvar...
	// cmatch!!
	assert(cons && cons->car.type == ASTListType); //  handle named let later...
	
	ASTList * hl = cons->car.d._ASTList, *paraml = NULL, *argl = NULL;
	foreach(el, hl) {
		ASTList_add(&paraml, el->car);
		el = el->next;
		ASTList_add(&argl, el->car);
	}
	
	int argc = 0, realargc = 0;
	
	fts * t = righttoleft(argl, s, emitting, &argc, NULL);
	
	// block for unamed let, lambda for named
	//
	
	int oldlevel;
	//if(newscope) {
		oldlevel = s->scopelevel;
		s->scopelevel = s->nlocs;
	//}
	
	foreach(param, paraml) {
		cbind(param->car, s, emitting, t->takes, true, 1, NULL);
		t = (fts *) t->rets;
	}
	
	types * rettype = cblock(cons->next, s, false, emitting);
	
	
	//if(newscope) {
		s->nlocs = s->scopelevel;
		s->scopelevel = oldlevel;
		
	//}
	return rettype;
}

types * cmatchclause(ASTList * clause, procs * s, bool emitting, matchs * matchtbl, types * m);

types * cmatch(ASTList * cons, procs * s, bool emitting) {
	assert(cons);
	
	ASTAtom h = cons->car;
	
	matchs matchtbl;
	
	types * m = cexpr(h, s, emitting);
	
	types * t = cmatchclause(cons->next, s, emitting, &matchtbl, m);
	
	if(emitting) {
		emit(s->code, s->cpos++, O_red);
		emit2(s->code, s->cpos, 1); // m->size!
		s->cpos += 2;
	}
	
	return t;
}
	
types * cmatchclause(ASTList * clause, procs * s, bool emitting, matchs * matchtbl, types * m) {
	matchtbl->popsize = 0;
	matchtbl->nbrfs = 0;
	
	types * rettype = t_void;
	
	if(clause) {
		//assert(!(issym(condition, symbol("else")) || issym(condition, symbol("true"))));
		//assert(type_eq(cexpr(condition, s, emitting), t_bool));
		assert(clause->car.type == ASTListType);
		assert(issym(clause->car.d._ASTList->car, symbol("=>")));
		assert(clause->car.d._ASTList->next);
		assert(clause->car.d._ASTList->next->next);
		assert(!clause->car.d._ASTList->next->next->next);
		ASTAtom pattern = clause->car.d._ASTList->next->car, 
				body = clause->car.d._ASTList->next->next->car;
		//ASTList * block = clause->car.d._ASTList->next;
		
		if(emitting) emit(s->code, s->cpos++, O_dup);
		
		int oldlevel;
		oldlevel = s->scopelevel;
		s->scopelevel = s->nlocs;
		
		cbind(pattern, s, emitting, m, true, 1, matchtbl); // m->size!!
		
		rettype = cexpr(body, s, emitting);//cblock(block, s, true, emitting);
		
		s->nlocs = s->scopelevel;
		s->scopelevel = oldlevel;
		
		int brendloc;
		if(emitting) {
			emit(s->code, s->cpos++, O_bra);
			brendloc = s->cpos;
			emit2(s->code, s->cpos, 0);
			s->cpos += 2;
			
			for(int i = 0; i < matchtbl->popsize; i++) {
				emit(s->code, s->cpos++, O_pop);
			}
			
			for(int i = 0; i < matchtbl->nbrfs; i++) {
				emit2(s->code, matchtbl->brflocs[i], s->cpos - matchtbl->remainders[i]);
			}
		}
		
		types * rtypenc = cmatchclause(clause->next, s, emitting, matchtbl, m);
		assert(!rtypenc || type_eq(rettype, rtypenc));
		if(emitting) emit2(s->code, brendloc, s->cpos);
	} else {
		//assert((issym(condition, symbol("else")) || issym(condition, symbol("true"))));
		//assert(ASTList_next(clause) == NULL);
		
		//rettype = cblock(block, s, true, emitting);
		if(emitting) {
			emit(s->code, s->cpos++, O_int);
			emit(s->code, s->cpos++, I_ERR);
		}
		
		return NULL;
	}
	
	return rettype;
}



types * call2tuplets(fts * call) {
	int nels = 0;
	
	for(fts * e = call; e != NULL; assert(!e->rets || e->rets->tag == FTS), e = (fts *) e->rets, nels++);
	
	tuplets * u = malloc(sizeof(tuplets));
	((types *)u)->tag = TTS;
	((types *)u)->size = 1;
	((types *)u)->compactsize = -1;
	u->els.n = nels;
	u->els.nt = malloc(nels * sizeof(types *));
	u->cs = 0;
	
	// if cn then assert(c0 < 1024) or something. to protect pointers
	int i = 0;
	for(fts * e = call; e != NULL; assert(!e->rets || e->rets->tag == FTS), e = (fts *) e->rets, i++) {
		u->els.nt[i] = e->takes;
		u->cs += e->takes->size;
	}
	
	return (types *) u;
}



types * funcall(ASTAtom f, ASTList * args, procs * s, bool emitting) {
	// right to left
	int argc = 0, realargc = 0;
	int rems[1024];
	fts * expectedt = righttoleft(args, s, emitting, &argc, rems);
	
	assert(argc);
	
	int c;
			
	if(f.type == SymbolType) {
		//s->exps = 1;
		
		
		if(Symbol_eq(f.d._Symbol, symbol(","))) {
			if(emitting) {
				assert(argc == rems[0]);
				emit(s->code, s->cpos++, O_chunk);
				emit2(s->code, s->cpos, argc);
				s->cpos += 2;
			}
			return call2tuplets(expectedt);
		} else if(Symbol_eq(f.d._Symbol, symbol("+")) && isbinop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_addi);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("-")) && isbinop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_subi);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("*")) && isbinop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_muli);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("/")) && isbinop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_divi);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("%")) && isbinop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_modi);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("<")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_clti);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol(">")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_cgti);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol("<=")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_clei);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol(">=")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_cgei);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol("==")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_ceqi);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol("!=")) && iscmpop(expectedt)) {
			if(emitting) emit(s->code, s->cpos++, O_cnei);
			return t_bool;
		} else if(Symbol_eq(f.d._Symbol, symbol("putc"))) {
			//assert(argc == 1 && re);
			assert(expectedt->takes == t_int && expectedt->rets == NULL);
			emit(s->code, s->cpos++, O_int);
			emit(s->code, s->cpos++, I_PUTC);
			return t_void;
			//s->exps = 0;
		} else if(Symbol_eq(f.d._Symbol, symbol("getc"))) {
			//assert(argc == 0);
			assert(expectedt->takes == t_void && expectedt->rets == NULL);
			emit(s->code, s->cpos++, O_int);
			emit(s->code, s->cpos++, I_GETC);
			return t_int;
		} else if(Symbol_eq(f.d._Symbol, symbol("error"))) {
			//assert(argc == 0);
			assert(expectedt->takes == t_void && expectedt->rets == NULL);
			emit(s->code, s->cpos++, O_int);
			emit(s->code, s->cpos++, I_ERR);
			return t_void;
		} else if((c = findb(s->locals, s->nlocs, f.d._Symbol)) != -1) {
			types * e = cexpr(f, s, emitting);
			assert(e && e->tag == FTS);
			types * ret = matchfunct(e, expectedt);
			assert(ret);
			
			if(emitting) for(int i = 0; i < argc; i++) { // arg eater table
				//assert(e && e->tag == FTS && ((fts *)e)->takes);
				
				if(rems[i] == 0) {
					emit(s->code, s->cpos++, O_appv);
				} else {
					emit(s->code, s->cpos++, O_appj);
					emit2(s->code, s->cpos, rems[i]);
					s->cpos += 2;
				}
				
				//realargc -= ((fts *)e)->takes->size;
				//argc--;
				//e = (fts *)e)->rets;
			}
			
			return ret;
		} else if((c = findb(globalc, pools, f.d._Symbol)) != -1 && arity[c] != UNKNOWNAR) {
			//assert(poolt[c] == 'p');
			assert(globalc[c].type->tag == FTS);
			
			types * ret = matchfunct(globalc[c].type, expectedt);
			
			// f :: int 
	
					
			if(!ret) {
				fprintf(stderr, "%d:%d: error: argument mismatch in call of function %s\n", 
					f.line, f.column, f.type == SymbolType ? Symbol_getstr(f.d._Symbol) : " (expression)");
		
				assert(0);
			}
			
			int i = 0;
			
			if(rems[0] < arity[c]) {
				if(emitting) {
				emit(s->code, s->cpos++, O_clo);
				emit2(s->code, s->cpos, c);
				s->cpos += 2;
				emit2(s->code, s->cpos, argc);
				s->cpos += 2;
				}
				
				i += arity[c];
			} else {
				if(emitting) {
				emit(s->code, s->cpos++, O_call);
				emit2(s->code, s->cpos, c);
				s->cpos += 2;
				
				}
				
				
				if(rems[i] == 0) {
					i++; // arity was 0, consume 1 void param
				} else i += arity[c]; // arity was nonzero, consume arity params...
			}
			
			for(; i < argc; i++) {
				if(emitting) {
					if(rems[i] == 0) {
						emit(s->code, s->cpos++, O_appv);
					} else {
						emit(s->code, s->cpos++, O_appj);
						emit2(s->code, s->cpos, rems[i]);
						s->cpos += 2;
					}
				}
			}
			
			 /*else {
				
				
				types * e = globalc[c].type;
				
				while(argc - arity[c]) {
					assert(e && e->tag == FTS && ((fts *)e)->takes);
					argc--;
					realargc -= ((fts *)e)->takes->size;
					e = (fts *)e)->rets;
				}
				
				
				
				if(emitting) {
					while(argc || ) { // arg eater table
						assert(e && e->tag == FTS && ((fts *)e)->takes);
						emit(s->code, s->cpos++, O_appj);
						emit2(s->code, s->cpos, argc);
						s->cpos += 2;
						argc -= ((fts *)e)->takes->size;
						e = (fts *)e)->rets;
					}
					
					 
				}
			}*/
			
			return ret;
			
			
		}
					
	}
	
	{
		types * e = cexpr(f, s, emitting);
		assert(e && e->tag == FTS);
		types * ret = matchfunct(e, expectedt);
		assert(ret);
			
		for(int i = 0; i < argc; i++) {
			if(emitting) {
				if(rems[i] == 0) {
					emit(s->code, s->cpos++, O_appv);
				} else {
					emit(s->code, s->cpos++, O_appj);
					emit2(s->code, s->cpos, rems[i]);
					s->cpos += 2;
				}
			}
		}
		
		assert(e == ret);
		
		return ret;
	}
}

types * clit(ASTAtom exp, procs * s, bool emitting) {
	if(exp.type == LongType) {
		if(emitting) {
		emit(s->code, s->cpos++, O_ldi);
		emit4(s->code, s->cpos, exp.d._long);
		s->cpos += 4;
		}
		return t_int;
		//s->exps = 1;
	} else if(exp.type == CharType) {
		if(emitting)  {
		emit(s->code, s->cpos++, O_ldi);
		emit4(s->code, s->cpos, exp.d._char);
		s->cpos += 4;
		}
		return t_int;
		//s->exps = 1;
	} else if(exp.type == BoolType) {
		if(emitting) {
		emit(s->code, s->cpos++, O_ldi);
		emit4(s->code, s->cpos, exp.d._bool);
		s->cpos += 4;
		}
		return t_bool;
		//s->exps = 1;
	}
	
	assert(0);
}

types * cexpr(ASTAtom exp, procs * s, bool emitting) {
	// globals wtfslsls
	
	if(exp.type == SymbolType) { //
		//emit(s->code, s->cpos++, O_ldi);
		int c;
		//if((c = finds(s->locnames, s->nlocs, exp.d._Symbol)) != -1) {
		if((c = findb(s->locals, s->nlocs, exp.d._Symbol)) != -1) {
			if(emitting)  {
			switch(s->locals[c].type->size) {
			case 1:
				if(c < s->nargs) {
					emit(s->code, s->cpos++, O_lda);
					emit2(s->code, s->cpos, c);
					s->cpos += 2;
					
				} else {
					emit(s->code, s->cpos++, O_ldl);
					emit2(s->code, s->cpos, c - s->nargs);
					s->cpos += 2;
				}
			}
			}
			//s->exps = 1;
			return s->locals[c].type;
		} else if((c = findb(globalc, pools, exp.d._Symbol)) != -1) {
			if(emitting) {
				
				if(globalc[c].type->tag == FTS) {
					emit(s->code, s->cpos++, O_clo);
					emit2(s->code, s->cpos, c);
					s->cpos += 2;
					emit2(s->code, s->cpos, 0);
					s->cpos += 2;
				} else
				switch(globalc[c].type->size) {
				case 1:
					emit(s->code, s->cpos++, O_ldc);
					emit2(s->code, s->cpos, c);
					s->cpos += 2;
				}
			}
			//s->exps = 1;
			return globalc[c].type;
			
			
			//s->exps = 1;
		}  /*else {
			// not only ldc. what if it's a closure value? indirection??
			emit(s->code, s->cpos++, O_ldc);
			
			s->lursym[s->lurc] = exp.d._Symbol;
			s->lurloc[s->lurc++] = s->cpos;
			
			emit2(s->code, s->cpos, 0);
			s->cpos += 2;
			//s->exps = 1;
			
			//fprintf(stderr, "Undefined symbol %s\n", exp.d._Symbol.s->str);
			//assert(0);
		}*/ else {
			fprintf(stderr, "%d:%d: error: undefined symbol %s\n", exp.line, exp.column, Symbol_getstr(exp.d._Symbol));
			assert(0);
		}
	} else if(exp.type == ASTListType) {
		if(exp.d._ASTList) {
			// handle builtins!!! cond fun etc
			// is expr in tail position???
			ASTAtom f = exp.d._ASTList->car;
			
			types * t;
			
			if(f.type == SymbolType) {
				if(Symbol_eq(f.d._Symbol, symbol("cond"))) {
					ASTList * clauses;
					assert(clauses = exp.d._ASTList->next);
					
					return cclause(clauses, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("do"))) {
					return cblock(exp.d._ASTList->next, s, true, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("while"))) {
					return cwhile(exp.d._ASTList->next, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("let"))) {
					return clet(exp.d._ASTList->next, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("match"))) {
					return cmatch(exp.d._ASTList->next, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("puts"))) {
					assert(exp.d._ASTList->next && exp.d._ASTList->next->car.type == StringType);
					
					const char *c = exp.d._ASTList->next->car.d._cstring;
					
					while(*c && emitting) {
						emit(s->code, s->cpos++, O_ldi);
						emit4(s->code, s->cpos, *c);
						s->cpos += 4;
						emit(s->code, s->cpos++, O_int);
						emit(s->code, s->cpos++, I_PUTC);
						c++;
					}
					
					return t_void;
					//return cmatch(exp.d._ASTList->next, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("<-"))) {
					ASTList * body;
					assert(body = exp.d._ASTList->next);
					
					
					
					//s->exps = 0; // no return!!!! VOID!!!!
					
					return cset(body, s, emitting);
				} else if(Symbol_eq(f.d._Symbol, symbol("var")) || Symbol_eq(f.d._Symbol, symbol("<="))) {
					ASTList * body;
					assert(body = exp.d._ASTList->next);
					
					
					
					//s->exps = 0; // no return!!!! VOID!!!!
					
					return cvar(body, s, emitting);
				} 
			}
			
			return funcall(f, exp.d._ASTList->next, s, emitting);
		} else return t_void;// else ldnil...
	} else return clit(exp, s, emitting);
	

}


int cfun(int pooldest, types * type, ASTList * args, ASTList * body) {
	// return type!!
	// internal name for recursion???
	//fprintf(stderr, "func poold %d\n", pooldest);
	procs s;
	memset(s.deconstruct, 0, sizeof(s.deconstruct));
	s.cpos = 0;
	
	// detect non global nonlocal references when indirection is ready, for closures...
	s.lurc = 0;
	
	// max ops???
	s.nargs = 0;
	s.nlocs = 0;
	
	if(pooldest == -1) pooldest = pools++;
	
	types * ft = type;
	
	////fprintf(stderr, "int %p rets %p \n", );
	
	//if(((fts *)type)->takes == t_void) assert(args == NULL);
	
	assert(ft && ft->tag == FTS);
	
	int i = 0;
	
	foreach(arg, args) {
		assert(ft && ft->tag == FTS);
		
		//fprintf(stderr, "argtype %p next %p ft->\n", ((fts *) ft)->takes);
		
		if(((fts *) ft)->takes == t_void) {
			assert(ASTAtom_isNil(arg->car) && !i); // cannot take void param after other params!
			arg = arg->next;
			ft = (((fts *) ft)->rets);
			break; // void-consuming functions can only be defined one parameter at a time
		} else {
			s.locals[s.nargs].type = ((fts *) ft)->takes;
			if(s.locals[s.nargs].type == t_void) break; // void arg... break
		
			if(headis(arg->car, symbol(","))) {
				//fprintf(stderr, "decons param\n");
				s.deconstruct[s.nargs] = true;
				s.rexp[s.nargs] = arg->car;
				s.locals[s.nargs].name = symbol("");
			} else s.locals[s.nargs].name = arg->car.d._Symbol;
		}
		
		i = 1;
		s.nargs += ((fts *) ft)->takes->size;
		s.nlocs += ((fts *) ft)->takes->size;
		
		ft = (((fts *) ft)->rets);
	}
	
	arity[pooldest] = s.nargs;
	globalc[pooldest].type = type;
	
	//fprintf(stderr, "nargs is %d", s.nargs);
	
	assert(ft);
	
	//fprintf(stderr, "returns %p ", ft);
	
	s.toplocs = s.nlocs;
	s.scopelevel = 0;
	
	// handle tail calls
	
	
	
	for(int i = 0; i < s.nargs; i++) {
		if(s.deconstruct[i]) {
			//if(emitting) {
				emit(s.code, s.cpos++, O_lda);
				emit2(s.code, s.cpos, i);
				s.cpos += 2;
			//}
			
			cbind(s.rexp[i], &s, true, s.locals[i].type, true, 0, NULL);
		}
	}
	
	types * rettype = cblock(body, &s, false, true);
	
	
	//fprintf(stderr, " actually %p \n", rettype);
	
	if(!type_eq(rettype, ft) && ft == t_void) {
		//fprintf(stderr, "mismatch\n"); // popping when func rets void
		switch(rettype->size) {
		case 4: // 4 words
			emit(s.code, s.cpos++, O_popq);
			break;
		case 2: // two words
			emit(s.code, s.cpos++, O_popl);
			break;
		case 1: // one word
			emit(s.code, s.cpos++, O_pop);
			break;
		case 0: // one word
			break;
		}
	} else assert(type_eq(rettype, ft));
	
	if(ft->size) emit(s.code, s.cpos++, O_ret);
	else emit(s.code, s.cpos++, O_retv);
	
	//fprintf(stderr, "end cpos %d\n", s.cpos);
	
	// storing the proc
	//poolt[pools] = 'p';
	pool[pooldest] = codepos;
	
	SETI32(gcode, codepos + P_NARGS, s.nargs);
	SETI32(gcode, codepos + P_NLOCS, s.toplocs - s.nargs);
	SETI32(gcode, codepos + P_PSIZE, s.cpos);
	
	codepos += PROCSTART;
	int copiedc = codepos;
	
	for(int i = 0; i < s.cpos; i++)
		gcode[codepos++] = s.code[i];
		
	for(int i = 0; i < s.lurc; i++) {
		//fprintf(stderr, "forward use of symbol %s\n", Symbol_getstr(s.lursym[i]));
		ursym[urc] = s.lursym[i];
		urloc[urc++] = copiedc + s.lurloc[i];
	}

	return pools - 1;
	
	// write down name for recursion
}

int cconstexpr(ASTAtom exp, int * stored, types ** ty) {
	// constant folding requires code execution 
	// during compile time. 
	// or we could delay computations till early runtime, some 
	// startup routine before entry...
	
	// semantic analyzer should detect any free variables in an expression
	// determine if they are bound by other variables (thus computation
	// is delayed till runtime) or just constants known at compile time
	// (fold them).
	
	if(exp.type == LongType) {
		*ty = t_int;
		return exp.d._long;
	} else if(exp.type == CharType) {
		*ty = t_int;
		return exp.d._char;
	} else if(exp.type == BoolType) {
		*ty = t_bool;
		return exp.d._bool;
	} else if(exp.type == SymbolType) { //
		//emit(code, (*cpos)++, O_ldi);
		int c;
		if((c = findb(globalc, pools, exp.d._Symbol)) != -1) {
			*ty = globalc[c].type;
			return pool[c];
		}
	} /*else if(exp.type == ASTListType) { //
		if(exp.d._ASTList) {
			// handle builtins!!! cond fun etc
			
			ASTAtom f = exp.d._ASTList->car;
			
			if(f.type == SymbolType) {
				if(Symbol_eq(f.d._Symbol, symbol("->"))) {
					*stored = true;
					return cfun(ASTList_next(exp.d._ASTList));
				} 
			}
		}
	}*/
}

/*
fac :: int -> int
fac n =  cond
fac :int n :int = cond
	n == 0
		1
	else
		n * (fac n - 1)*/

// handle local definitions!!
void cdef(ASTList * cons) {
	Symbol lhs;
	bool stored = false;
	
	bool iscons = false;
	ASTList * args = NULL;
	
	types * proto = NULL;
	
	// also accept the notation fac :int n :int
	
	if(cons->car.type == SymbolType)
		lhs = cons->car.d._Symbol;
	else if (cons->car.type == ASTListType && cons->car.d._ASTList->car.type == SymbolType) {
		// check if its really a fun or just a typed lhs
		lhs = cons->car.d._ASTList->car.d._Symbol;
		iscons = true;
		args = cons->car.d._ASTList->next;
	}
	
	
	int c;
	if((c = findb(globalc, pools, lhs)) != -1) {
		//fprintf(stderr, "Duplicate global %s\n", lhs.s->str);
		assert(!hasvalue[c]);
		//assert(!globalc[c].type->tag == FTS);
		
		proto = globalc[c].type;
	} else {
		fprintf(stderr, "%d:%d: error: no proto for %s\n", cons->car.line, cons->car.column, Symbol_getstr(lhs));
		//fprintf(stderr, "undefd sym %s proto \n", Symbol_getstr(lhs));
	}
	
	if(iscons) {
		assert(proto);
		assert(c != -1);
		cfun(c, proto, args, cons->next);
		
	} else {
		int c = findb(globalc, pools, lhs), dest;
		if(c == -1) dest = pools;
		else dest = c;
		
		types * ty = NULL;
		int r = cconstexpr(*ASTList_at(cons, 1), &c, &ty);
		
		if(c != -1) {
			assert(ty == globalc[c].type);
		} else pools += ty->size;
		
		globalc[dest].type = ty;
		globalc[dest].name = lhs;
		hasvalue[dest] = true;
		pool[dest] = r;
	}
	
	
	fprintf(stderr, "defined global %s = %d \n", Symbol_getstr(lhs), pool[pools - 1]);
	
	
}


void cproto(ASTList * cons) {
	Symbol lhs;
	bool stored = false;
	
	if(cons->car.type == SymbolType)
		lhs = cons->car.d._Symbol;
	
	int c;
	if((c = findb(globalc, pools, lhs)) != -1) {
		fprintf(stderr, "%d:%d: error: duplicate global %s\n", cons->car.line, cons->car.column, Symbol_getstr(lhs));
		//fprintf(stderr, "Duplicate global %s\n", Symbol_getstr(lhs));
		assert(0);
	}
	
	types * t = ctexpr(*ASTList_at(cons, 1));
	assert(tiscomplete(t));
	
	globalc[pools].name = lhs;
	globalc[pools].type = t;
	hasvalue[pools] = false;
	
	
	if(Symbol_eq(symbol("main"), lhs))
		entry = pools;
	
	pools += t->size;
	
	fprintf(stderr, "defined global %s = %d at %d\n", 
		Symbol_getstr(lhs), pool[pools - 1], findb(globalc, pools, lhs));
	
	//return pools - 1;
}


void resolve() {
	for(int i = 0; i < urc; i++) { // wow quadratic such slow
		int c = findb(globalc, pools, ursym[i]);
		if(c == -1) {
			fprintf(stderr, "Undefined symbol %s\n", Symbol_getstr(ursym[i]));
			assert(0);
		}
		
		emit2(gcode, urloc[i], c);
	}
}

void compile(ASTList * code) {
	foreach(el, code) {
		if(el->car.type == ASTListType && el->car.d._ASTList) {
			ASTAtom f = el->car.d._ASTList->car;
			
			if(f.type == SymbolType) {
				if(Symbol_eq(f.d._Symbol, symbol("="))) {
					cdef(el->car.d._ASTList->next);
				} else if(Symbol_eq(f.d._Symbol, symbol(":="))) {
					ctdef(el->car.d._ASTList->next);
				} else if(Symbol_eq(f.d._Symbol, symbol("::"))) {
					cproto(el->car.d._ASTList->next);
				}
			}
		}
	}
	
	resolve();
	
	WRITEI32(pools);
	WRITEI32(codepos);
	WRITEI32(0);
	WRITEI32(entry);
	
	fprintf(stderr, "p %d c %d e %d\n", pools, codepos, entry);
	
	for(int i = 0; i < pools; i++) {
		putchar(globalc[i].type->tag == FTS ? 'p' : 0);
		WRITEI32(pool[i]);
	}
	
	for(int i = 0; i < codepos; i++)
		putchar(gcode[i]);
	
	
	
	//for(int i = 0; i < rawsize; i++)
	//	rawmem[i] = getc(in);
	
	
}

int main(int argc, char *argv[]) {
	/*
	init_symbols();
	
	FILE* fs = fopen(argv[1], "r");
	
	ifstream ifs = ifstream_make(fs);
	istream * in = (istream *) &ifs;
	
	nextchar(in);
	
	nextlex = buffer1;
	lex = buffer2;
	
	nexttoken(in);
	
	if(nexttok == TOK_LINE) nexttoken(in);
	
	ASTList * program = NULL, ** dest = &program;
	
	while(nexttok != TOK_EOF) {
		//nexttoken(in);
		
		*dest = ASTList_new(group(in, false), NULL);
		dest = &((*dest)->next);
	}
	
	dump(AList(program), 0);
	/**/
	
	/**/
	entry = -1;
	
	init_symbols();
	init_types();
	
	FILE* fs = fopen(argv[1], "r");
	
	ifstream ifs = ifstream_make(fs);
	istream * in = (istream *) &ifs;
	
	nextchar(in);
	
	nextlex = buffer1;
	lex = buffer2;
	
	nexttoken(in);
	
	if(nexttok == TOK_LINE) nexttoken(in);
	
	ASTList * program = NULL, ** dest = &program;
	
	while(nexttok != TOK_EOF) {
		//nexttoken(in);
		
		*dest = ASTList_new(group(in,	 false), NULL);
		dest = &((*dest)->next);
	}
	
	dump(AASTList(program), 0);
	
	memset(globalc, sizeof globalc, 0);
	
	compile(program);
	/**/
	
	//for(int i = 0; i < pools; i++) {
	//	fprintf(stderr, "global symbol %s = %d\n", globnames[i].s->str, pool[i]);
	//}
	
	return 0;
	
	/*rflags rf;
	rflags_reset(&rf);
		
		
	reader_state rs = {true, 0, 0};
		
	ASTList * program = global_infix_transform(
		readlines((istream *)&ifs, global_buffer, &global_buffer_size,
		&rf, &rs, TOK_EOF, false));
		
	assert(rf.depth == 0);
	
	memset(globnames, sizeof globnames, 0);
	
	inittypes();
	
	compile(program);
	
	for(int i = 0; i < pools; i++) {
		fprintf(stderr, "global symbol %s = %d\n", globnames[i].s->str, pool[i]);
	} */
}
