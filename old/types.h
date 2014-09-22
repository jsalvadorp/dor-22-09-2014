#ifndef TYPES_H
#define TYPES_H

#include "forward.h"
#include "gc.h"
#include "symbol.h"

// Types

typedef struct Type {
	GCObject_HEAD
} Type;

#define Type_HEAD Type _Type_;

// Primitive Types

#define DEFTYPE(name) \
	typedef struct { \
		Type_HEAD \
	} name##Type_t; \
	\
	extern Type * name##Type; \
	Type * name##Type_new();

DEFTYPE(Cons)
DEFTYPE(String)
DEFTYPE(Lambda)
DEFTYPE(Continuation)
DEFTYPE(Environment)
DEFTYPE(Module)
DEFTYPE(Int)
DEFTYPE(Char)
DEFTYPE(Double)
DEFTYPE(Bool)
DEFTYPE(Symbol)
DEFTYPE(Keyword)
DEFTYPE(Type)
DEFTYPE(Macro)

void PrimitiveType_grayChildren();
void PrimitiveType_destroy();

extern struct GCObject_vtbl_t Cons_vtbl;

void init_types();

// typed container

typedef union {
	Cons *				_Cons;
	/*
	Dict *				_Dict;
	Lambda *			_Lambda;
	Environment *		_Environment;
	Array *				_Array;
	FunctionGeneric *	_FunctionGeneric;
	String * 			_String;
	Object *			_Object;
	Class *				_Class;
	Type *				_Type;
	Macro *				_Macro;
	
	#if CALLCC
	Continuation *	_Continuation;
	#endif
	*/
	
	String * _String;
	Lambda * _Lambda;
	Continuation * _Continuation;
	Type * _Type;
	Environment * _Environment;
	Module * _Module;
	
	// general way to access any of the above.
	GCObject * _GCObject;
	
	
	// The following are the ONLY types that Atom can hold that are stored in-place
	// and as such are not garbage collectible. (Though that may change for symbol)
	// this is to guarantee that, if the type of the atom isn't any of the above, it
	// is safe to assume it is a pointer to a garbage-collectible object.
	//cfunc _c_lambda;
	long  _long;
	char _char;
	double _double;
	bool _bool;
	Symbol _Symbol;
	Keyword _Keyword;
	/*symbol _symbol;
	keyword _keyword;*/
	
	// used only in special cases. type tag is unnecessary (we'll always know beforehand)
	Atom * _Atom_transparent;
	instr_t _instr_t;
	Evaluator * _Evaluator;
} anydata;

#define gc_collectable(a) (a->type != IntType \
							&& a->type != CharType \
							&& a->type != DoubleType \
							&& a->type != BoolType \
							&& a->type != SymbolType \
							&& a->type != KeywordType \
							 )
							 


struct Atom {
	Type * type;
	anydata d;
};

Atom Along(long x);
Atom Adouble(double x);
Atom Abool(bool x);
Atom Achar(char x);
Atom ASymbol(Symbol x);
Atom Asymbol(const char * x);
Atom AKeyword(Keyword x);
Atom AType(Type * x);
Atom AContinuation(Continuation * x);

bool Atom_as_bool(Atom * a);
long Atom_as_long(Atom * a);
double Atom_as_double(Atom * a);
char Atom_as_char(Atom * a);

bool Atom_isNil(Atom * this);

// destroys an object. in case incremental GC requires special handling of pointers.
void Atom_destroy(Atom * a);
void Atom_copy(Atom * dest, const Atom * src);
// copy without destroying. when the dest object is new.
void Atom_copynd(Atom * dest, const Atom * src);
void Atom_markGray(Atom * a);

String * toREPLString(const Atom * what);
String * toString(Atom * what);
void dump (Atom what, int indent);
void dump_inline(Atom what);

// primitive types are not garbage collected and must be manually deleted on exit.
void delete_types();

void zerofill(anydata * a);

#endif



