#define POOL(x) constants[x]
#define ARG(x) state.args[x]
#define CLOS(x) state.regs.closure[x]
#define LOCAL(x) state.locals[x]
#define PUSH(x) (state.ops[state.regs.optop++] = x)
#define POP (state.ops[--state.regs.optop])
#define OPTOP state.regs.optop
#define PC state.regs.pc
#define SAVE_REGS *(state.regs_space) = state.regs
#define TOP (state.ops + state.regs.optop)
#define ISZERO(x) (x._int32 == 0)




CASE_OPC( nop ):
	// nop
	BREAK

CASE_OPC( break ):
	// break
	BREAK

CASE_OPC( ldarg ):
	// ldarg <uarg16>
	PUSH(ARG(uoperand));
	BREAK

CASE_OPC( starg ):
	// starg <uarg16>
	ARG(uoperand) = POP;
	BREAK

CASE_OPC( ldloc):
	// ldloc <uarg16>
	PUSH(LOCAL(uoperand));
	BREAK

CASE_OPC( stloc ):
	// stloc <uarg16>
	LOCAL(uoperand) = POP;
	BREAK

CASE_OPC( ldcl ):
	// ldcl <uarg16>
	PUSH(CLOS(uoperand));
	BREAK

CASE_OPC( stcl ):
	// stcl <uarg16>
	CLOS(uoperand) = POP;
	BREAK

CASE_OPC( ldind ):
	// ldind 
	PUSH(*(POP._ref));
	BREAK

CASE_OPC( stind ):
	// stind 
	tmp = POP; // value
	*(POP._ref) = tmp;
	BREAK

CASE_OPC( ldg ):
	// ldg 
	PUSH(*(POOL(uoperand)._ref));
	BREAK

CASE_OPC( stg ):
	// stg 
	tmp = POP; // value
	*(POOL(uoperand)._ref) = tmp;
	BREAK

CASE_OPC( ldc ):
	// ldc <uarg16>
	PUSH(POOL(uoperand));
	BREAK

CASE_OPC( ldi ):
	// ldi <arg32>
	tmp._int32 = operand;
	PUSH(tmp);
	BREAK

CASE_OPC( add ):
	// add
	tmp._int32 = POP._int32;
	tmp._int32 += POP._int32;
	PUSH(tmp);
	BREAK

CASE_OPC( sub ):
	// sub
	tmp._int32 = POP._int32;
	tmp._int32 = POP._int32 - tmp._int32;
	PUSH(tmp);
	//printf("SUB %d\n", tmp._int32);
	BREAK

CASE_OPC( mul ):
	// mul
	tmp._int32 = POP._int32;
	tmp._int32 *= POP._int32;
	PUSH(tmp);
	//printf("MUL %d\n", tmp._int32);
	BREAK

CASE_OPC( div ):
	// div
	tmp._int32 = POP._int32;
	tmp._int32 = POP._int32 / tmp._int32;
	PUSH(tmp);
	BREAK

CASE_OPC( rem ):
	// rem 
	// define what happens with negatives
	tmp._int32 = POP._int32;
	tmp._int32 %= POP._int32;
	PUSH(tmp);
	BREAK

CASE_OPC( call ):
	// call <uarg16>
	// pool indexed
	// tail calls? recursive tail calls? jmp what?
	// call closure/proc on opstack.

	f = POOL(uoperand)._proc;
	
	*(state.regs_space) = state.regs;
	//goto call;

call:
	printf("calling %s ", f->name);
	if(f->args_size) printf("arg %d ", state.ops[state.regs.optop - f->args_size]._int32);
	
	
	state.regs_space->optop -= f->args_size;
	
	state.args = TOP - f->args_size;
	printf("args pos %p\n", state.args);
	state.locals = TOP;
	state.regs.prev_frame = state.regs_space;
	state.regs_space = (regs_t *)(TOP + f->locals_size);
	
	if(f->cproc) {
		f->cproc(&state);
		goto ret;
	} else {
	
	state.ops = ((word_t *)(state.regs_space + 1));
	
	state.regs.pc = 0;
	state.regs.optop = 0;
	state.regs.proc = f;
	code = f->code;
	constants = f->constants;
	// closure?
	
	//memset locals, 0 importantttt
	
	}
	
	BREAK

CASE_OPC( ret ):
	// ret
ret:	
	
	state.regs_space = state.regs.prev_frame;
	if(state.regs_space == NULL) { state.running = 0; BREAK }
	
	// push return value
	printf("Returning %d words from %s", f->ret_size, f->name);
	if(f->ret_size) printf(": %d\n", state.ops[state.regs.optop - 1]._int32);
	else printf("\n");
	/*
	
	state.args[0] = state.ops[state.regs.optop - 1];
	state.regs_space->optop += 1; // single word
	
	/**/
	// is this too much?
	// instructions take too much data from proc info. too "polymorphic"
	/**/
	
	memmove(state.args, TOP - f->ret_size, f->ret_size * sizeof(word_t));
	state.regs_space->optop += f->ret_size;
	/**/
	
	state.regs = *state.regs_space;
	f = state.regs.proc;
	
	
	state.locals = (((word_t *) state.regs_space) - f->locals_size);
	state.args = (state.locals - f->args_size);
	state.ops = (word_t *)(state.regs_space + 1);
	
	code = f->code;
	constants = f->constants;
	
	BREAK

CASE_OPC( br ):
	// br
	
	state.regs.pc = uoperand;
	
	BREAK

CASE_OPC( brz ):
	// brz
	
	if(ISZERO(POP)) {
		//puts("JUMP TAKEN");
		state.regs.pc = uoperand;
	}
	
	BREAK

CASE_OPC( clos ):
	// clos
	
	tmp = POP;
	
	c = malloc(sizeof(closure_t));
	assert(c);
	c->proc = tmp._proc;
	c->clos = malloc(tmp._proc->clos_size * sizeof(word_t));
	memcpy(c->clos, state.ops + state.regs.optop 
		- tmp._proc->clos_size, tmp._proc->clos_size * sizeof(word_t));
	state.regs.optop -= tmp._proc->clos_size;
	tmp._closure = c;
	PUSH(tmp);
	
	BREAK

CASE_OPC( callcl ):
	// callcl
	
	*(state.regs_space) = state.regs;
	
	tmp = POP;
	
	c = tmp._closure;
	//printf("closure is %p\n", c);
	
	
	f = c->proc;
	state.regs.closure = c->clos;
	
	goto call;
	
	

CASE_OPC( dup ):
	// dup
	
	tmp = POP;
	
	PUSH(tmp);
	PUSH(tmp);
	
	BREAK

CASE_OPC( pop ):
	// dup
	
	POP;
	
	BREAK

CASE_OPC( tail_ ): // incomplete return. for tail calls
	// removes stackframe but will continue execution
	// in current method for _just one instruction_
	// which must be a call. (verify this please)
	// verification in the assembler or compiler
	// tail. destroys current frame.
	
	printf("tail.");
	
	FETCH
	
	switch(opcode) {
	case OPC_call:
		f = POOL(uoperand)._proc;
		break;
	case OPC_callcl:
		tmp = POP;
	
		c = tmp._closure;
		f = c->proc;
		state.regs.closure = c->clos;
		
		break;
	default:
		assert(0);
	}
	
	//assert(0);
	state.regs_space = state.regs.prev_frame;
	if(state.regs_space == NULL) { state.running = 0; BREAK }
	
	/*// push return value
	state.args[0] = state.ops[state.regs.optop - 1];
	state.regs_space->optop += 1; // single word
	
	state.regs = *state.regs_space;
	f = state.regs.proc;
	
	
	state.locals = (((word_t *) state.regs_space) - f->locals_size);
	state.args = (state.locals - f->args_size);
	state.ops = (word_t *)(state.regs_space + 1);
	
	code = f->code;
	constants = f->constants;
	*/
	
	// 		WHAT IF THE AREA OVERLAPS? HANDLE THAT!!! done, memmove
	
	memmove(state.args, TOP - f->args_size, f->args_size * sizeof(word_t));
	
	printf("copied args ");
	int i = 0;
	for(i = 0; i < f->args_size; i++)
		printf("%d ", state.args[i]._int32);
	
	state.ops = (word_t *)(state.regs_space + 1);
	state.regs.optop = state.regs_space->optop + f->args_size;
	printf(" args vs top %p %p \n", state.args, state.ops + state.regs.optop);
	state.regs_space->optop = state.regs.optop;
	
	goto call;
	/*
	state.regs_space->optop -= f->args_size;
	
	state.args = TOP - f->args_size;
	state.locals = TOP;
	state.regs.prev_frame = state.regs_space;
	state.regs_space = (regs_t *)(TOP + f->locals_size);
	
	if(f->cproc) {
		f->cproc(&state);
		goto ret;
	} else {
	
	state.ops = ((word_t *)(state.regs_space + 1));
	
	state.regs.pc = 0;
	state.regs.optop = 0;
	state.regs.proc = f;
	code = f->code;
	constants = f->constants;
	// closure?
	
	//memset locals, 0 importantttt
	
	}
	
	BREAK
*/
	BREAK
	
	
	
		
	/*
	
	frame:
	args
	locals
	regs
		proc id or pointer
		flags // has it been captured?
		pc
		op
		closure ptr ??
	operands
	
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
	regs_t * regs_space;
	word_t * ops;
	
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






/*
ldarg
starg
ldloc
stloc
ldcl
stcl
ldarga
ldloca
ldcla
ldind
stind

call
callcl

compare
cgt
c.gt

bz
bp
bn
bgt
blt
bge
ble
beq
bne


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



	
	call
		-- suport tail calling. either recursive (simply jmp, recall, recur)
		-- or to other procs (by removing the current frame and calling)
	local branching, conditional and nonconditional (goto)
	ret
	dup
	pop
	comparison
	bitwise
	binary logic
	nop
	breakpoint
	ap - apply
	
	? sel
	? call from stack: pop funcpointer from opstack, call it. 
	*/
