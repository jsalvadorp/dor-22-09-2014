#include <stdio.h>
#include <assert.h>
#include "vm.h"
#include "asm.h"

//TODO:
// by reference
// continuations. (lazy? copying? what!!)
// dynamic memory
// accessing members

DEFINE_ARGSIZES

#define FETCH \
		opcode = code[state.regs.pc++]; \
		\
		/*printf("instructions 0x%02x\n", opcode);*/ \
		 \
		/* handle signedness. currently only unsigned*/ \
		\
		if(argsize[opcode] == 2) { \
			uoperand = code[state.regs.pc++] << 8; \
			uoperand |= code[state.regs.pc++]; \
		} else if(argsize[opcode] == 4) { \
			uoperand = code[state.regs.pc++] << 24; \
			uoperand |= code[state.regs.pc++] << 16; \
			uoperand |= code[state.regs.pc++] << 8; \
			uoperand |= code[state.regs.pc++]; \
		} \
		\
		operand = uoperand;

int run(state_t * istate) {
	int32_t operand;
	uint32_t uoperand;
	
	word_t tmp;
	
	state_t state = *istate;
	
	int opcode;
	unsigned char * code = state.regs.proc->code; // to save one dereference every cycle
	word_t * constants = state.regs.proc->constants;
	
	proc_t * f;
	closure_t * c;
	
	JUMP_TABLE_DEF
	
	//while(clock > 0) {
	//for(;;) {
	while(state.running) { // very inefficient. should use threads	
		// http://gcc.gnu.org/onlinedocs/gcc-4.4.6/gcc/Labels-as-Values.html#Labels-as-Values
		// to replace switch (adds dependence on gcc)
		
		
		// fetch
		
		FETCH
		
		// execute
		
		JUMP_TABLE_START(opcode)
		
		#include "instructions.h"
		
		JUMP_TABLE_END
		4+3;
	}
	
	*istate = state;
}

/*
typedef struct {
	uint16_t optop;
	uint16_t pc;
	
	void * proc;
	void * closure; // or environment.
	
	// flags;
} regs_t __attribute__ ((aligned (sizeof(word_t))));

typedef struct {
	regs_t regs;
	
	word_t * args;
	word_t * locals;
	word_t * ops;
	regs_t * regs_space;
	
	void * stack;
} state_t;

typedef struct {
	int code_size;
	int args_size;
	int locals_size;
	int ops_size;
	word_t * constants;
	char * code;
	
} proc_t;
*/
void builtin_printint(state_t * state) {
	printf("%d\n", state->args[0]._int32);
}


void builtin_readint(state_t * state) {
	int x;
	scanf("%i", &x);
	
	state->ops[state->regs.optop++]._int32 = x;
}

proc_t * cproc_new(cproc_t le, int argc, int rets, const char * name) {
	proc_t * p = malloc(sizeof(proc_t));
	
	memset(p, 0, sizeof(proc_t));
	
	p->cproc = le;
	p->args_size = argc;
	p->ret_size = rets;
	p->name = name;
}

state_t * init_vm (size_t words, proc_t * entry) {
	state_t * s = malloc(sizeof(state_t));
	
	s->stack = malloc(words * sizeof(word_t));
	s->args = (word_t *) s->stack;
	s->locals = ((word_t *) s->stack) + entry->args_size;
	s->regs_space = (regs_t *) (s->locals + entry->locals_size);
	s->ops = (word_t *) (s->regs_space + 1);
	
	s->regs.prev_frame = NULL;
	s->regs.optop = 0;
	s->regs.pc = 0;
	s->regs.proc = entry;
	
	return s;
}

int main() {
	//printf("%zu", sizeof(regs_t));
	constpool cp;
	linkinfo li;
	//proc_t * entry = NULL;
	strdict * external = NULL;
	
	li.imports = NULL;
	li.exports = NULL;
	
	//assemble(stdin, &cp, &entry);
	
	char buf[300];
	int count;
	
	/*FILE * test = fopen("test.s", "r");
	
	scantokinst(test, buf, &count);
	puts(buf);
	scantokinst(test, buf, &count);
	puts(buf);
	
	fclose(test);*/
	
	FILE * f = fopen("example.s", "r");
	
	if(!f) return 1;
	
	/*while(scantok(f, buf, &count) != EOF) {
		printf("%s\n", buf);
	}*/
	
	fseek(f, SEEK_SET, 0);
	
	
	assemble(f, &cp, &li);
	
	fclose(f);
	printf("entry pool index is %d\n", cp.entry);
	
	if(!cp.cp[cp.entry]._proc) return -1;
	
	if(cp.cp[cp.entry]._proc) {
		hex(cp.cp[cp.entry]._proc);
	}
	
	cp.cp[strdict_get(li.imports, "builtin::printint")._cp_index]._proc 
		= cproc_new(builtin_printint, 1, 0, "builtin::printint");
	cp.cp[strdict_get(li.imports, "builtin::readint")._cp_index]._proc 
		= cproc_new(builtin_readint, 0, 1, "builtin::readint");
	
	
	state_t * s = init_vm(65536, cp.cp[cp.entry]._proc/*cp.cp[entry]._proc*/);
	s->running = 1;
	puts("RUNNING\n");
	run(s);
	
	return 0;
}
	
