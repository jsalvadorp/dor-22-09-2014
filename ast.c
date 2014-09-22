#include <stdio.h> 
#include <stdarg.h>
#include <stdlib.h>

#include "ast.h"

ASTAtom nil;

#define ATOMINIT(n) ASTAtom n; memset(&a.d, 0, sizeof(anydata)); a.line = -1; a.column = -1;

ASTAtom Alc(ASTAtom e, int l, int c) {
	e.line = l;
	e.column = c;
	
	return e;
}

ASTAtom AList(ASTList * c) {
	ATOMINIT(a);
	a.type = ASTListType;
	a.d._ASTList = c;
	
	return a;
}
ASTAtom AASTList(ASTList * c) {
	ATOMINIT(a);
	a.type = ASTListType;
	a.d._ASTList = c;
	
	return a;
}

ASTList * ASTList_new(ASTAtom car, ASTList * cdr) {
	ASTList * nc = malloc(sizeof(ASTList));
	
	nc->car = car;
	nc->next = cdr;
	
	return nc;
}

size_t ASTList_length(ASTList * this) {
	size_t suma = 0;
	for(; this; this = this->next, suma++);
	return suma;
}

ASTList * ASTList_add(ASTList ** l, ASTAtom n) {
	ASTList_append(l, ASTList_new(n, NULL));
	
	return *l;
}

ASTList * ASTList_append(ASTList ** l, ASTList * l2) {
	while(*l != NULL) l = &((*l)->next);
	*l = l2;
}

ASTList * ASTList_last(ASTList * l) {
	while(l && l->next) l = l->next;
	return l;
}

ASTList * ASTList_list(size_t count, ...) {
	va_list args;
	
	ASTList * l = NULL;
	ASTList ** next = &l;
	
	va_start(args, count);
	
	for(size_t i = 0; i < count; i++) {
		*next = ASTList_new(va_arg(args, ASTAtom), NULL);
		next = &((*next)->next);
	}
	
	va_end(args);
	return l;
}


ASTList * ASTList_reversed(ASTList * l) {
	ASTList * newlist = NULL;
	
	foreach(el, l) newlist = ASTList_new(el->car, newlist);
		
	return newlist;
}


ASTAtom * ASTList_at(ASTList * this, size_t i) {
	ASTList * node = this;
	
	for (;i > 0; i--)
		node = node->next;
	
	return &(node->car);
}


ASTAtom Along(long x) {
	ATOMINIT(a);
	a.type = LongType;
	a.d._long = x;
	return a;
}

ASTAtom Acstring(cstring x) {
	ATOMINIT(a);
	a.type = StringType;
	a.d._cstring = x;
	return a;
}

ASTAtom Adouble(double x) {
	ATOMINIT(a);
	a.type = DoubleType;
	a.d._double = x;
	return a;
}

ASTAtom Abool(bool x) {
	ATOMINIT(a);
	a.type = BoolType;
	a.d._bool = x;
	return a;
}

ASTAtom Achar(long x) {
	ATOMINIT(a);
	a.type = CharType;
	a.d._char = x;
	return a;
} 

ASTAtom ASymbol(Symbol x) {
	ATOMINIT(a);
	a.type = SymbolType;
	a.d._Symbol = x;
	return a;
} 

ASTAtom Asymbol(cstring x) {
	ATOMINIT(a);
	a.type = SymbolType;
	a.d._Symbol = symbol(x);
	return a;
} 

bool ASTAtom_isNil(ASTAtom x) {
	return x.type == ASTListType && x.d._ASTList == NULL;
} 


#include "assert.h"

void to_str(char * buf, ASTAtom a) {
	if(a.type == StringType)
		sprintf(buf, "%s", a.d._cstring);
	else if(a.type == LongType)
		sprintf(buf, "%ld", a.d._long);
	else if(a.type == BoolType)
		sprintf(buf, "%s", a.d._bool ? "true" : "false");
	else if(a.type == CharType)
		sprintf(buf, "$%c", (char) a.d._char);
	else if(a.type == SymbolType)
		sprintf(buf, "%s", Symbol_getstr(a.d._Symbol));
	else assert(0);
}

char buf[1000];

void dump(ASTAtom what, int indent) {
	if(what.type == ASTListType) {
		times(i, indent) fprintf(stderr, "   ");
		fprintf(stderr, "(\n");
		
		foreach(el, what.d._ASTList) {
			dump(el->car, indent + 1);
		}
		
		times(i, indent) fprintf(stderr, "   ");
		fprintf(stderr, ")\n");
	} else {
		times(i, indent) fprintf(stderr, "   ");
		to_str(buf, what);
		fprintf(stderr, "%d: %s", what.line, buf);
		fprintf(stderr, "\n");
	}
}

#if 0

#include "read3.h"
int main(int argc, char * argv[]) {
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
	
	ASTList * l = ASTList_list(6, Along(28), Asymbol("hello"), Along(46), Along(55), Asymbol("hi"), Along(73));
	
	char buf[500];
	
	foreach(el, l) {
		to_str(buf, el->car);
		printf("%s\n", buf);
	}
}

#endif
