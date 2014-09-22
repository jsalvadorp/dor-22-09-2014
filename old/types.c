#include "types.h"
#include "cons.h"
#include "gc.h"
#include "string.h"
#include "lambda.h"
#include <stdlib.h>
#include <stdio.h>

void zerofill(anydata * a) {
	memset(a, 0, sizeof(anydata));
}

#define DEFTYPENEW(name) \
	Type * name##Type = NULL; \
	Type * name##Type_new() { \
		if(name##Type != NULL) \
			return name##Type; \
		else { \
			Type * r = malloc(sizeof(name##Type_t)); \
			r->_GCObject_.color = GC_noncollectable; \
			return r; \
		} \
	}

DEFTYPENEW(Cons)
DEFTYPENEW(String)
DEFTYPENEW(Lambda)
DEFTYPENEW(Continuation)
DEFTYPENEW(Environment)
DEFTYPENEW(Module)
DEFTYPENEW(Int)
DEFTYPENEW(Char)
DEFTYPENEW(Double)
DEFTYPENEW(Bool)
DEFTYPENEW(Symbol)
DEFTYPENEW(Keyword)
DEFTYPENEW(Type)
DEFTYPENEW(Macro)

void init_types() {
	IntType = IntType_new();
	ConsType = ConsType_new();
	StringType = StringType_new();
	BoolType = BoolType_new();
	CharType = CharType_new();
	DoubleType = DoubleType_new();
	SymbolType = SymbolType_new();
	KeywordType = KeywordType_new();
	LambdaType = LambdaType_new();
	ContinuationType = ContinuationType_new();
	TypeType = TypeType_new();
	EnvironmentType = EnvironmentType_new();
	ModuleType = ModuleType_new();
	MacroType = MacroType_new();
}

Atom Along(long x) {
	Atom a;
	zerofill(&a.d);
	a.type = IntType;
	a.d._long = x;
	
	return a;
}

Atom Adouble(double x) {
	Atom a;
	zerofill(&a.d);
	a.type = DoubleType;
	a.d._double = x;
	
	return a;
}

Atom Abool(bool x) {
	Atom a;
	zerofill(&a.d);
	a.type = BoolType;
	a.d._bool = x;
	
	return a;
}

Atom Achar(char x) {
	Atom a;
	zerofill(&a.d);
	a.type = CharType;
	a.d._char = x;
	
	return a;
}


Atom ASymbol(Symbol x) {
	Atom a;
	zerofill(&a.d);
	a.type = SymbolType;
	a.d._Symbol = x;
	
	return a;
}

Atom Asymbol(const char * x) {
	Atom a;
	zerofill(&a.d);
	a.type = SymbolType;
	a.d._Symbol = symbol(x);
	
	return a;
}

Atom AKeyword(Keyword x) {
	Atom a;
	zerofill(&a.d);
	a.type = KeywordType;
	a.d._Keyword = x;
	
	return a;
}

Atom AType(Type * x) {
	Atom a;
	zerofill(&a.d);
	a.type = TypeType;
	a.d._Type = x;
	
	return a;
}

Atom AContinuation(Continuation * x) {
	Atom a;
	zerofill(&a.d);
	a.type = ContinuationType;
	a.d._Continuation = x;
	
	return a;
}

bool Atom_as_bool(Atom * a) {
	return (a->type == BoolType && a->d._bool)
			|| (a->type == ConsType && a->d._Cons);
}

long Atom_as_long(Atom * a) {
	if(a->type == IntType)
		return (long) a->d._long;
	else if(a->type == DoubleType)
		return (long) a->d._double;
	else if(a->type == CharType)
		return (long) a->d._char;
	else if(a->type == BoolType)
		return (long) a->d._bool;
}

double Atom_as_double(Atom * a) {
	if(a->type == IntType)
		return (double) a->d._long;
	else if(a->type == DoubleType)
		return (double) a->d._double;
	else if(a->type == CharType)
		return (double) a->d._char;
	else if(a->type == BoolType)
		return (double) a->d._bool;
}

char Atom_as_char(Atom * a) {
	if(a->type == IntType)
		return (char) a->d._long;
	else if(a->type == DoubleType)
		return (char) a->d._double;
	else if(a->type == CharType)
		return (char) a->d._char;
	else if(a->type == BoolType)
		return (char) a->d._bool;
}

bool Atom_isNil(Atom * a) {
	return (a->type == ConsType && !(a->d._Cons));
}

void Atom_destroy(Atom * a) {
	// do something when incremental GC is implemented
}

void Atom_copy(Atom * dest, const Atom * src) {
	Atom_destroy(dest);
	*dest = *src;
}

void Atom_copynd(Atom * dest, const Atom * src) {
	*dest = *src;
}

void Atom_markGray(Atom * a) {
	if(gc_collectable(a) && a->d._GCObject)
		GCObject_markGray(a->d._GCObject);
}

// convert atom to string for REPLs.
// MUST return a pointer to String. Right now it's a C-string
String * toREPLString(const Atom * what) {
	// enough space for the digits of a 128-bit unsigned integer and NULL character.
	char digits[200]; 

	if(what->type == IntType) {
		//itoa (what->d._long, digits, int bas);
		sprintf(digits, "%ld", what->d._long);
		return Str(digits);
	} else if(what->type == DoubleType) {
		sprintf(digits, "%lf", what->d._double);
		return Str(digits);
	} else if(what->type == BoolType) {
		return what->d._bool ? Str("true") : Str("false");
	} else if(what->type == CharType) {
		digits[0] = '$';
		digits[1] = what->d._char;
		digits[2] = 0;
		return Str(digits);
	} else if(what->type == SymbolType) {
		return Str(what->d._Symbol.s->str);
	} else if(what->type == KeywordType) {
		return String_cat_ss(what->d._Keyword.s->str, ":");
	} else if(what->type == StringType) {
		return String_cat_Ss(String_cat_sS("\"", what->d._String), "\"");
	} else if(what->type == LambdaType) {
		String * n = String_new("<LAMBDA ");
		n = String_cat_Ss(n, (what->d._Lambda->name.s 
			? what->d._Lambda->name.s->str 
			: ""));
		n = String_cat_Ss(n, ">");
		return n;
	} else if(what->type == ContinuationType) {
		return String_new("<CONTINUATION>");
	} else {
		sprintf(digits, "NOT PRINTABLE %p", what->d._Cons);
		return Str(digits);
	}
}

String * toString(Atom * what) {
	// enough space for the digits of a 128-bit unsigned integer and NULL character.
	char digits[900]; 

	if(what->type == IntType) {
		//itoa (what->d._long, digits, int bas);
		sprintf(digits, "%ld", what->d._long);
		return Str(digits);
	} else if(what->type == DoubleType) {
		sprintf(digits, "%lf", what->d._double);
		return Str(digits);
	} else if(what->type == BoolType) {
		return what->d._bool ? Str("true") : Str("false");
	} else if(what->type == CharType) {
		digits[0] = what->d._char;
		digits[1] = 0;
		return Str(digits);
	} else if(what->type == SymbolType) {
		return Str(what->d._Symbol.s->str);
	} else if(what->type == KeywordType) {
		return String_cat_ss(what->d._Keyword.s->str, ":");
	} else if(what->type == StringType) {
		return what->d._String;
	} else return Str("??");
}

void dump(Atom what, int indent) {
	if(what.type == ConsType) {
		times(i, indent) fprintf(stderr, "   ");
		fprintf(stderr, "(\n");
		
		foreach(el, what.d._Cons) {
			dump(el->car, indent + 1);
			
			if(el->cdr.type != ConsType) {
				times(x, indent + 1) fprintf(stderr, "   ");
				fprintf(stderr, " . \n");
				dump(el->cdr, indent + 1);
				break;
			}
		}
		
		times(i, indent) fprintf(stderr, "   ");
		fprintf(stderr, ")\n");
	} else {
		times(i, indent) fprintf(stderr, "   ");
		fprintf(stderr, cstr(toREPLString(&what)));
		fprintf(stderr, "\n");
	}
}
			
			

void dump_inline(Atom what) {
	if(what.type == ConsType) {
		fprintf(stderr, "(");
		
		foreach(el, what.d._Cons) {
			dump_inline(el->car);
			
			if(el->cdr.type == ConsType) {
				if(Cons_next(el)) fprintf(stderr, " ");
			} else {
				fprintf(stderr, " . ");
				dump_inline(el->cdr);
				break;
			}
		}
			
		
		fprintf(stderr, ")");
	} else {
		printf(cstr(toREPLString(&what)));
	}
}

void delete_types() {
	free(IntType);
	free(BoolType);
	free(StringType);
	free(CharType);
	free(DoubleType);
	free(ConsType);
	free(SymbolType);
	free(KeywordType);
	free(LambdaType);
	free(TypeType);
	free(EnvironmentType);
	free(ContinuationType);
	free(ModuleType);
	free(MacroType);
}
