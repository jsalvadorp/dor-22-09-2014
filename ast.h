#ifndef AST_H
#define AST_H

#include "symbol.h"

//typedef int bool;


struct ASTList;
typedef struct ASTList ASTList;

typedef enum {
	ASTListType = 0,
	StringType,
	LongType,
	CharType,
	DoubleType,
	BoolType,
	SymbolType
} ASTType;

typedef union {
	ASTList * _ASTList;
	cstring _cstring;
	
	long  _long;
	long _char;
	double _double;
	bool _bool;
	
	Symbol _Symbol;
} anydata;
							 
typedef struct {
	int line;
	int column;
	
	ASTType type;
	anydata d;
} ASTAtom;

struct ASTList {
	ASTAtom car;
	ASTList * next;
};

ASTAtom Alc(ASTAtom, int, int);

ASTAtom Along(long x);
ASTAtom Acstring(cstring x); 
ASTAtom Adouble(double x);
ASTAtom Abool(bool x); 
ASTAtom Achar(long x); 
ASTAtom ASymbol(Symbol x); 
ASTAtom Asymbol(cstring x); 
bool ASTAtom_isNil(ASTAtom x); 

ASTAtom AList(ASTList * c);
ASTAtom AASTList(ASTList * c);
ASTList * ASTList_new(ASTAtom car, ASTList * next);
ASTList * ASTList_add(ASTList ** l, ASTAtom n);
ASTList * ASTList_append(ASTList ** l, ASTList * l2);
ASTList * ASTList_list(size_t count, ...);
ASTList * ASTList_last(ASTList * l);
ASTList * ASTList_reversed(ASTList * l);
ASTAtom * ASTList_at(ASTList * this, size_t index);
size_t ASTList_length(ASTList * this);
ASTList * ASTList_next(ASTList * c);
ASTList ** ASTList_nextr(ASTList * c);

#define foreach(el, li) for(ASTList * el = li; el; el = el->next)

extern ASTAtom nil;

void to_str(char * buf, ASTAtom a);
void dump(ASTAtom what, int indent); 

#endif
