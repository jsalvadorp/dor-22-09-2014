#include "types.h"
#include "string.h"
#include "cons.h"
#include "stream.h"
#include "read.h"
#include "infix.h"
#include "environment.h"
#include "eval.h"
#include "std.h"
#include "read2.h"


//#include "dorc.h"


#include <unistd.h>
#include <getopt.h>
#include <stdio.h>
#include <curses.h>
#include <readline/readline.h>
#include <readline/history.h>

// case classes y typeclasses

// considerar la licencia GPL de readline

// checar si la tail call optimization aplica al evaluar argumentos...

// GC ROOT

Cons * program = NULL;
Atom line;
Atom ret;
Environment * global = NULL;
Evaluator evaluator;
String * input = NULL;

void Root_grayChildren(GCRoot * this) {
	//r->i->markGray();
	
	//if(ec != NULL) ec->markGray();
	
	Atom_markGray(&(line));
	Atom_markGray(&(ret));
	Evaluator_markGray(&evaluator);
	
	if(global) GCObject_markGray((GCObject *) global);
	if(std) GCObject_markGray((GCObject *) std);
	if(program) GCObject_markGray((GCObject *) program);
	
	if(input) GCObject_markGray((GCObject *) input);
	
}

void Root_destroy(GCRoot * this) {
	
}

GCRoot _GCRoot_ = {Root_grayChildren, Root_destroy};

int main(int argc,char *argv[]) {
	void * p = EVSE;
	//printf("pointer : %p\n", (instr_t)EVSE);
	FILE *f;

	int opt = 0;
	
	bool debugmode = false, 
		interactive = false, 
		printdump = false,
		compiled = false, 
		prettysyntax = false;

	while((opt = getopt(argc, argv, "ndpic")) != -1) {
		switch (opt) {
			case 'n':
				prettysyntax = true;
				break;
			case 'd': //debug mode enabled
				debugmode = true;
				break;
			case 'i':
				interactive = true;
				break;
			case 'p':
				printdump = true;
				break;
			case 'c':
				compiled = true;
				break;
			default:
				puts("wtf");
		}
	}
	
	GC_setup(&_GCRoot_);
	init_types();
	init_symbols();
	init_operators();
	init_bfs();
	
	line = nil;
	ret = nil;
	evaluator = Evaluator_make(false);
	
	init_std(&evaluator);
	
	global = Environment_new(NULL, NULL);
	Environment_addImport(global, std);
	
	GC_mark();
	GC_sweep();
	
	//if(!global_buffer) global_buffer = malloc(sizeof(char) * READ_BUFFER_SIZE);
	
	
	//Cons * readlines(istream * in, char * buffer, int * size, rflags * fl, reader_state * rs, enum toktype endtoken);
	
	/*FILE * prueba = fopen("newsyntax", "r");
	rflags rf;
	rflags_reset(&rf);
	ifstream ifs = ifstream_make(prueba);
	reader_state rs = {true, 0, 0};
	
	dump(ACons(
	readlines((istream *)&ifs, global_buffer, &global_buffer_size,
		&rf, &rs, TOK_EOF, false)
	), 0);*/
	
	/*isstream iss = isstream_make(
		"12 (print \"Hello ma dear\" $A isa 'isa isa: op::m n.s 123 123e3 "
		"12.34e-3 +23 -16rDEADBEEF `(expr ,@(cdr e) ,e) true false nil)"
	);
	
	char * buffer = malloc(sizeof(char) * READ_BUFFER_SIZE);
	
	rflags rf;
	rflags_reset(&rf);
	
	program = ACons(global_infix_transform(readl((istream *)&iss, buffer, &rf)));
	
	
	
	
	*/
	
	evaluator.debug = debugmode;
	
	rl_bind_key('\t',rl_abort);
	
	
	
	if (optind < argc) {
		if(debugmode) 
			puts("READING FILE:::::::::::::::::::::::::::::::::::::::::::::::::");
		
		/*if(f = fopen( argv[optind], "r" )) root_->program = interpret(f, false);
		fclose(f);*/
		
		FILE* fs = fopen(argv[optind], "r");
		
		ifstream ifs = ifstream_make(fs);
		
		rflags rf;
		rflags_reset(&rf);
		
		optind++;
		
		
		reader_state rs = {true, 0, 0};
		
		program = prettysyntax
		? global_infix_transform(
			readlines((istream *)&ifs, global_buffer, &global_buffer_size,
			&rf, &rs, TOK_EOF, false))
		: global_infix_transform(readl((istream *)&ifs, global_buffer, &rf));
		
		assert(rf.depth == 0);
		
		if(printdump) {
			puts("START DUMP:::::::::::::::::::::::::::::::::::::::::::::::::::");
			dump(ACons(program), 0);
			puts("END DUMP:::::::::::::::::::::::::::::::::::::::::::::::::::::");
		}
		
		if(debugmode) 
			puts("EVALING FILE:::::::::::::::::::::::::::::::::::::::::::::::::");
		
		/*if(compiled)
			compile(program);
		else*/
			Evaluator_eval_seq(&evaluator, program, (Environment *) global);
		
		// STORE PROGRAM ARGUMENTS IN SOME VARIABLE
	} else interactive = true;
	
	if(interactive) {
		printf("Dor\n\n");
		
		if(debugmode) 
			puts("REPL:::::::::::::::::::::::::::::::::::::::::::::::::::::::::");
		
		while(true) {
			input = String_new(readline(" > "));
			
			isstream iss = isstream_make(cstr(input));
			
			rflags rf;
			rflags_reset(&rf);
			
			program = readl((istream *)&iss, global_buffer, &rf);
			
			while(rf.depth > 0) {
				input = String_cat_Ss(input, "\n");
				printf("%d", rf.depth);
				input = String_cat_Ss(input, readline("* "));
				
				rflags_reset(&rf);
				
				isstream iss = isstream_make(cstr(input));
				program = readl((istream *)&iss, global_buffer, &rf);
			}
			
			program = global_infix_transform(program);
			
			if(strcmp("", cstr(input))) add_history(cstr(input));
			
			//cout << "HAS: " << fs.str() << endl;
			
			//puts("START DUMP:::::::::::::::::::::::::::::::::::::::::::::::::::");
			if(printdump) dump(ACons(program), 0);
			//puts("END DUMP:::::::::::::::::::::::::::::::::::::::::::::::::::::");
		
			/*gc_ptr<Cons> el = program.getList().first_;
			while(el != NULL) {
				r = eval(el->content(), ec);
				el = el->next();
			}*/
			
			
			
			ret = Evaluator_eval_seq(&evaluator, program, (Environment *) global);
			
			free(input);
			
			if(evaluator.quit) {
				puts("quitting");
				break;
			}
			
			//printf("=> %s\n", cstr(toREPLString(&ret)));
			printf("=> ");
			dump_inline(ret);
			printf("\n");
			
			
		}
	}
	
	
	
	
	
	
	
	//if(global_buffer) free(global_buffer);
	
	GC_shutdown();
	delete_types();
	delete_symbols();
	delete_operators();
	delete_bfs();
}
