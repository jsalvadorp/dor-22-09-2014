#ifndef FORWARD_H
#define FORWARD_H

typedef int bool;
#define true 1
#define false 0

struct Cons;
struct Atom;
struct Type;
struct GCObject;
struct GCRoot;
struct String;
struct Symbol;
struct Keyword;
struct Environment;
struct Module;
struct Lambda;
struct Evaluator;
struct Continuation;

/*struct Object;
struct Dict;
struct Lambda;
struct Environment;
struct Array;
struct FunctionGeneric;
struct Class;
struct Macro;
struct symbol;
struct keyword;*/

typedef struct Cons Cons;
typedef struct Atom Atom;
typedef struct Type Type;
typedef struct GCObject GCObject;
typedef struct GCRoot GCRoot;
typedef struct String String;
typedef struct Symbol Symbol;
typedef struct Keyword Keyword;
typedef struct Environment Environment;
typedef struct Module Module;
typedef struct Lambda Lambda;
typedef struct Evaluator Evaluator;
typedef struct Continuation Continuation;
/*struct String;
struct Object;
struct Dict;
struct Lambda;
struct Environment;
struct Array;
struct FunctionGeneric;
struct Class;
struct Macro;
struct symbol;
struct keyword;*/

// for the evaluator
typedef void (*instr_t) (Evaluator *);

extern Lambda * listc;

#define nil ACons(NULL)

#define times(i, n) for(size_t i = 0; i < (n); i++)

#endif
