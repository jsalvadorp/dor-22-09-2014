#include <stdio.h>
#include <ctype.h>
#include <assert.h>
#include "vm.h"
#include "strdict.h"
#include "asm.h"


uint32_t strto4cc(const char * str) {
	return *((uint32_t *) str);
}

void fcctos(uint32_t i, char * str) {
	*((uint32_t *) str) = i;
}

int scantok(FILE * src, char * buf, int * count) { // scans a label, a directive, an opcode, an opcode prefix
	int c;
	*count = 0;
	
	while(c = fgetc(src), isspace(c)) {
		
	}
	
	if(c == EOF) return EOF;
	
	buf[(*count)] = c;
	
	//printf("firstc %c\n", (char)c);
	
	/*
	if(c == '.') c = fscanf(src, "%[^ \t\n]s%n", buf + 1, count); // directive
	else c = fscanf(src, "%[^. \t\n]s%n", buf + 1, count); // nondirective
	*/
	fscanf(src, "%[^ \t\n]s%n", buf + 1, count);
	(*count)++;
	
	//buf[*count] = 0;
	
	//printf("COUNT IS %d TOK IS %s\n", *count, buf);
	
	*count = strlen(buf);
	
	//printf("COUNT IS %d TOK IS %s\n", *count, buf);
	
	return *count;
	
}

int scantokinst(FILE * src, char * buf, int * count) { // scans a label, a directive, an opcode, an opcode prefix
	int c;
	*count = 0;
	
	while(c = fgetc(src), isspace(c)) {
		
	}
	
	if(c == EOF) return EOF;
	
	buf[(*count)] = c;
	
	//printf("firstc %c\n", (char)c);
	
	
	if(c == '.') c = fscanf(src, "%[^ \t\n]s%n", buf + 1, count); // directive
	else {
		c = fscanf(src, "%[^. \t\n]s%n", buf + 1, count); 
		
	}
	//fscanf(src, "%[^ \t\n]s%n", buf + 1, count);
	(*count)++;
	
	//buf[*count] = 0;
	
	//printf("COUNT IS %d TOK IS %s\n", *count, buf);
	
	*count = strlen(buf);
	
	if((c = fgetc(src)) == '.')
		buf[(*count)++] = c;
	else ungetc(c, src);
	
	//printf("COUNT IS %d TOK IS %s\n", *count, buf);
	
	return *count;
	
}

int assemble_cp(FILE * src, constpool * c, strdict ** symbols) {
	int cp_size = 0, cp_pos = 0;
	
	char buf[256];
	int bufsize = 0;
	
	bool rextern = false;
	
	unsigned long unum;
	long num;
	
	if(fscanf(src, "%i", &cp_size) == EOF) return EOF;
	c->cp = malloc(cp_size * sizeof(word_t));
	memset(c->cp, 0, cp_size);
	c->external = malloc(cp_size * sizeof(int));
	memset(c->external, 0, cp_size);
	c->type = malloc(cp_size * sizeof(int));
	memset(c->type, 0, cp_size);
	c->size = cp_size;
	
	while(scantok(src, buf, &bufsize) != EOF) {
		if(!strcmp(".extern", buf)) {
			rextern = true;
			
			/* reading symbol argument else if((r = fscanf(src, "%[^ \t\n]s%n", buf, &bufsize)) > 0) {
			
			} */
		} else if(!strcmp(".end", buf)) {
			break;
		} else if(sscanf(buf, "%lu:", &unum) > 0) {
			cp_pos = unum;
		} else {
			word_t pos;
			pos._cp_index = cp_pos;
			strdict_insert(symbols, buf, pos);
			if(rextern)
				c->external[cp_pos] = 1;
				//strdict_insert(external, buf, pos); 
			printf("CONST %s %d\n", buf, cp_pos);
			
			if(fscanf(src, "%s%n", buf, &bufsize) == EOF) return EOF;
			
			if(!strcmp("w", buf)) { //single word
				c->type[cp_pos] = strto4cc("w   ");
				cp_pos++;
			} if(!strcmp("proc", buf) ) { //single word
				c->type[cp_pos] = strto4cc("proc");
				cp_pos++;
			} else if(!strcmp("l", buf)) { //double word
				c->type[cp_pos] = strto4cc("l   ");
				cp_pos += 2;
			} else if(!strcmp("q", buf)) { //quad word
				c->type[cp_pos] = strto4cc("q   ");
				cp_pos += 4;
			} else if(!strcmp("str", buf)) { //specified string
				c->type[cp_pos] = strto4cc("proc");
				if(!rextern) {
					cp_pos++;
				} else cp_pos++;
			} else if(!strcmp("i4", buf)) { //specified string
				c->type[cp_pos] = strto4cc("i4  ");
				if(!rextern) {
					sscanf(buf, "%li", &num);
					c->cp[cp_pos++]._int32 = num;
				} else cp_pos++;
			} else if(sscanf(buf, "%lu", &unum) > 0) { //double word
				cp_pos += unum;
			}
		}
	}
	
	// TODO: assert cp_pos < cp_size
	
	return cp_size;
}

int assemble_ie(FILE * src, constpool * c, strdict ** imports) {
	char buf[256];
	int bufsize = 0;
	
	unsigned long unum;
	long num;
	
	//if(fscanf(src, "%i", &cp_size) == EOF) return EOF;
	//cp = malloc(cp_size * sizeof(word_t));
	
	while(scantok(src, buf, &bufsize) != EOF) {
		if(!strcmp(".end", buf)) {
			break;
		} else {
			word_t pos;
			sscanf(buf, "%lu", &unum);
			pos._cp_index = unum;
			
			if(fscanf(src, "%s%n", buf, &bufsize) == EOF) return EOF;
			
			strdict_insert(imports, buf, pos);
			// asser cp_index < c->size
		}
	}
	
	
	return 0;
}


// TODO: symbol table (global), label dictionary(local), asm constants
int assemble_f(FILE * src, proc_t * f, strdict * symbols, /*proc_t ** entry, */const char * name, bool * isentry) { // assembles code for a single function
	// static. do we need thread safety?
	/* static */
	char buf[256];
	int bufsize = 0;
	
	/* static */
	char code[65536];
	int codesize = 0;
	
	
	long num;
	unsigned long unum;
	
	int r;
	
	strdict * labels = NULL;
	
	long fpos = ftell(src);
	long cpos = codesize;
	
	int pass;
	for(pass = 0; pass < 2; pass++) {
	
	codesize = cpos;
	fseek(src, fpos, SEEK_SET);
	
	f->name = strdup(name);
	
	bool tail = false;
	
	while(scantokinst(src, buf, &bufsize) != EOF) {
		//printf("tok %s lastc %c\n", buf, buf[bufsize]);
		
		if(tail) assert(!strcmp("call", buf) || !strcmp("callcl", buf));
		tail = false;
	
		if(!strcmp(".args", buf)) {
			if((r = fscanf(src, "%lu", &unum)) > 0) {
				f->args_size = unum;
			} else return EOF;
			
			/* reading symbol argument else if((r = fscanf(src, "%[^ \t\n]s%n", buf, &bufsize)) > 0) {
			
			} */
		} else if(!strcmp(".locals", buf)) {
			if((r = fscanf(src, "%lu", &unum)) > 0) {
				f->locals_size = unum;
			} else return EOF;
		} else if(!strcmp(".ops", buf)) {
			if((r = fscanf(src, "%lu", &unum)) > 0) {
				f->ops_size = unum;
			} else return EOF;
		} else if(!strcmp(".rets", buf)) {
			if((r = fscanf(src, "%lu", &unum)) > 0) {
				f->ret_size = unum;
			} else return EOF;
		} else if(!strcmp(".clos", buf)) {
			if((r = fscanf(src, "%lu", &unum)) > 0) {
				f->clos_size = unum;
			} else return EOF;
		} else if(!strcmp(".end", buf)) {
			break;
		} else if(!strcmp(".entry", buf)) {
			//printf("Proc %s is entry!\n", name);
			//*entry = f;
			*isentry = true;
		} else if(buf[bufsize - 1] == ':') { // label
		
			buf[bufsize - 1] = '\0';
			if(pass == 0) {
				word_t v;
				v._int32 = codesize;
				strdict_insert(&labels, buf, v);
				
				printf("defined label %s value %d ss %d\n", buf, strdict_get(labels, buf)._int32, codesize);
			}
		} else { // opcode or prefix
			#define  CASE_MNEM(mnem, dowhat) \
				if(!strcmp(#mnem, buf)) { \
					code[codesize++] = OPC_##mnem; \
					dowhat \
				}
			#define  CASE_PREFIX(mnem, dowhat) \
				if(!strcmp(#mnem ".", buf)) { \
					code[codesize++] = OPC_##mnem##_; \
					dowhat \
				}
			
			// FIXME: assumes scanf of argument will succeed.
			
			printf("0x%04x    %-8s ", codesize, buf);
			
			CASE_MNEM(nop,;)
			CASE_MNEM(break,;)
			CASE_MNEM(ldarg,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(starg,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(ldloc,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(stloc,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(ldcl,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(stcl,
				fscanf(src, "%lu", &unum);
				
				printf("%lu", unum);
				
				code[codesize++] = (unum & 0xFF00) >> 8; // msb
				code[codesize++] = unum & 0xFF; // lsb.
			)
			CASE_MNEM(ldc,
				if(fscanf(src, "%s", buf) > 0 && sscanf(buf, "%lu", &unum) < 0) {
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				} else {
					unum = strdict_get(symbols, buf)._cp_index ;
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				}
				
				printf("%lu", unum);
			)
			CASE_MNEM(ldi,
				if(fscanf(src, "%s", buf) > 0 && sscanf(buf, "%lu", &unum) > 0) {
					code[codesize++] = (unum & 0xFF000000) >> 24; // msb
					code[codesize++] = (unum & 0x00FF0000) >> 16; // msb
					code[codesize++] = (unum & 0x0000FF00) >> 8; // msb
					code[codesize++] =  unum & 0x000000FF; // lsb.
				}
				
				printf("%lu", unum);
			)
			CASE_MNEM(add,;)
			CASE_MNEM(sub,;)
			CASE_MNEM(mul,;)
			CASE_MNEM(div,;)
			CASE_MNEM(rem,;)
			CASE_MNEM(call,
				if(fscanf(src, "%s", buf) > 0 && sscanf(buf, "%lu", &unum) > 0) {
				
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				} else {
					unum = strdict_get(symbols, buf)._cp_index;
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				}
				
				printf("%lu", unum);
			)
			CASE_MNEM(ret,;)
			CASE_MNEM(br,
				if(fscanf(src, "%s", buf) > 0 && sscanf(buf, "%lu", &unum) < 0) {
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				} else {
					if(pass == 1) unum = strdict_get(labels, buf)._cp_index ;
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				}
				
				printf("%lu", unum);
			)
			CASE_MNEM(brz,
				if(fscanf(src, "%s", buf) > 0 && sscanf(buf, "%lu", &unum) < 0) {
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				} else {
					if(pass == 1) {
						strdict ** s = strdict_find(&labels, buf);
						//printf("label %s\n", buf);
						assert(*s);
						
						unum = (*s)->value._int32;
					}
					
					code[codesize++] = (unum & 0xFF00) >> 8; // msb
					code[codesize++] = unum & 0xFF; // lsb.
				}
				
				printf("%lu", unum);
			)
			CASE_MNEM(clos,;)
			CASE_MNEM(callcl,;)
			CASE_PREFIX(tail,
				tail = true;
				puts("\nPREFIX");
			)
			
			printf("\n");
			
			#undef CASE_MNEM
			#undef CASE_PREFIX
		}
	}
	
	}
	
	f->code = malloc(codesize);
	f->code_size = codesize;
	
	memcpy(f->code, code, codesize);
	
	return codesize;
	
}

int assemble(FILE * src, constpool * cp, linkinfo * li) {
	int cp_size;
	
	strdict * symbols = NULL;
	
	char buf[256];
	int bufsize = 0;
	
	bool pool_ready = false;
	
	while(scantok(src, buf, &bufsize) != EOF) {
		if(!strcmp(".pool", buf)) {
			assert(!pool_ready);
			printf("Constant Pool\n");
			assemble_cp(src, cp, &symbols);
			pool_ready = true;
		} else if(!strcmp(".proc", buf)) {
			assert(pool_ready);
			scantok(src, buf, &bufsize); // name of proc
			
			
			printf("Proc %s\n", buf);
			
			word_t w;
			proc_t * f = malloc(sizeof(proc_t));
			memset(f, 0, sizeof(proc_t));
			
			int pool_index = strdict_get(symbols, buf)._cp_index;
			bool isentry = false;
			
			assemble_f(src, f, symbols, /*entry, */buf, &isentry);
			if(isentry) {
				printf("Recognized proc %s as entry\n", buf);
				cp->entry = pool_index;
			}
			w._proc = f;
			cp->cp[pool_index] = w;
			f->constants = cp->cp;
			//strdict_insert(symbols, buf, w);
			
		} else if(!strcmp(".object", buf)) {
			assert(!pool_ready);
			scantok(src, buf, &bufsize); // name of obj
			
			li->name = strdup(buf);
		} else if(!strcmp(".import", buf)) {
			assert(pool_ready);
			assemble_ie(src, cp, &li->imports);
			
		} else if(!strcmp(".export", buf)) {
			assert(pool_ready);
			assemble_ie(src, cp, &li->exports);
			
		} else if (buf[0] == '.') {
			while(scantok(src, buf, &bufsize) != EOF && strcmp(".end", buf));
		}
	}
}


void hex (proc_t * f) {
	int i;
	
	for(i = 0; i < f->code_size; i++) {
		printf("%02x\n", f->code[i]);
	}

}
