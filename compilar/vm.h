#ifndef VM_H
#define VM_H

typedef int bool;
#define false 0
#define true 1

#define LP64

typedef signed char int8_t;
typedef unsigned char uint8_t;

#ifdef LLP64
	typedef signed short int16_t;
	typedef unsigned short uint16_t;
	typedef signed int int32_t;
	typedef unsigned int uint32_t;
	typedef signed long long int64_t;
	typedef unsigned long long uint64_t;
#endif

#ifdef LP64
	typedef signed short int16_t;
	typedef unsigned short uint16_t;
	typedef signed int int32_t;
	typedef unsigned int uint32_t;
	typedef signed long int64_t;
	typedef unsigned long uint64_t;
#endif

typedef uint16_t cp_index; // constant pool index
// should really consider extending to 32bit.
// 65536 constant references isn't a lot...
// plus consider space for flags.

struct word_t;
struct dword_t;
struct proc_t;
struct regs_t;
struct state_t;
struct closure_t;
struct constpool;
struct strdict;
struct linkinfo;
struct tram_pair;

typedef struct word_t word_t;
typedef struct dword_t dword_t;
typedef struct proc_t proc_t;
typedef struct regs_t regs_t;
typedef struct state_t state_t;
typedef struct closure_t closure_t;
typedef struct constpool constpool;
typedef struct strdict strdict;
typedef struct linkinfo linkinfo;
typedef struct tram_pair tram_pair;

struct word_t {
	union {
		int8_t _int8;
		int16_t _int16;
		int32_t _int32;
		uint8_t _uint8;
		uint16_t _uint16;
		uint32_t _uint32;
		float _float;
		void * _ptr;
		word_t * _ref;
		const char * str;
		proc_t * _proc;
		cp_index _cp_index;
		closure_t * _closure;
	};
};

struct dword_t {
	union {
		uint64_t _int64;
		uint64_t _uint64;
		double _double;
	};
};

struct tram_pair {
	uint16_t callsite;
	uint16_t trampoline;
};

typedef void (*cproc_t)(state_t *);

struct proc_t {
	const char * name; // for debugging
	
	int args_size;
	int ret_size;
	int clos_size;
	
	int locals_size;
	int ops_size;
	
	int code_size;
	
	char * code;
	cproc_t cproc;
	
	word_t * constants;
	
	tram_pair * tram_table;
};

struct closure_t {
	proc_t * proc;
	word_t * clos;
};

struct regs_t {
	uint16_t optop;
	uint16_t pc;
	
	regs_t * prev_frame;
	
	proc_t * proc;
	word_t * closure; // or environment.
	
	// flags;
} __attribute__ ((aligned (sizeof(word_t))));;

struct state_t {
	regs_t regs;
	
	word_t * args;
	word_t * locals;
	word_t * ops;
	regs_t * regs_space;
	
	void * stack;
	
	int running;
};

struct constpool {
	int size;
	word_t * cp;
	int * external;
	int * type;
	int entry;
};

struct linkinfo {
	const char * name;
	
	strdict * imports;
	strdict * exports;
	
	int imports_size;
	int exports_size;
};

#include "opcodes.h"

#define COMPUTED_GOTO 1

#if COMPUTED_GOTO
	#define CASE_OPC(opcode) case_##opcode
	
	#define JUMP_TABLE_DEF JUMP_TABLE
	
	#define JUMP_TABLE_START(opcode) goto *jmp_tbl[opcode];
	#define JUMP_TABLE_END jump_table_end:
	#define BREAK goto jump_table_end;
	
#else	
	#define CASE_OPC(opcode) case OPC_##opcode
	#define JUMP_TABLE_DEF ;
	#define JUMP_TABLE_START(opcode) switch(opcode) {
	#define JUMP_TABLE_END }
	#define BREAK break;
#endif

#endif
