#include <ctype.h>
#include <assert.h>
#include "read.h"
#include "types.h"
#include "cons.h"
#include "string.h"
#include "read2.h"




int match_newline(istream * in, rflags * fl) {
	// reads all whitespace until the next newline. if it encounters a
	// nonwhitespace character, return 0.
	
	size_t inipos = itell(in);
	
	int c = igetc(in);
	if(c == EOF) return -1;
	
	while(c != '\n' && (isspace(c) || c == ';')) {
		if(c == ';') {
			while(c != '\n') {
				c = igetc(in);
				if(c == EOF) return -1;
			}
			
			// handle multiline/inline comments
			
			return 1; //found newline
		}
		
		//ICHECK_EOF
		c = igetc(in);
		if(c == EOF) return -1;
	}
	
	if(c == '\n') return 1;
	
	iseek(in, inipos); 
	return 0; // this has no meaning
}

int match_eof(istream * in, rflags * fl) {
	// matches all whitespace until eof. 
	
	size_t inipos = itell(in);
	
	int c = igetc(in);
	if(c == EOF) return -1;
	
	while(c != EOF && (isspace(c) || c == ';')) {
		if(c == ';') {
			while(c != '\n') {
				c = igetc(in);
				if(c == EOF) return 1;
			}
		}
		
		//ICHECK_EOF
		c = igetc(in);
	}
	
	if(c == EOF) return 1;
	
	iseek(in, inipos); 
	return 0;
}

int eat_spacestabs(istream * in, rflags * fl) { 
	int c = igetc(in);
	
	while(c == '\t' || c == ' ') {
		c = igetc(in);
		// handle inline comments
	}
	
	iseekr(in, -1);
}

int match_indent(istream * in, rflags * fl) { // assumes no eof because nonempty line
	int count = 0, c = igetc(in);
	
	while(c == '\t') {
		count++;
		c = igetc(in);
	}
	
	iseekr(in, -1);
	return count;
}

enum toktype read_token(istream * in, char * buffer, int * s, 
	rflags * fl, bool * line_start, int * indent_level, int *  pending_indents) {
	
	int size;
	
	if(*line_start) { // if line has just started
		//puts ("line begins");
		while(size = match_newline(in, fl)) if(size < 0) return TOK_EOF; // skip to the beginning of the first non empty line. will consume tabs also!
		
		size = match_indent(in, fl); // check indentation level.
		
		*pending_indents = size - *indent_level;
		*line_start = false;
	}
	
	//fprintf(stderr, "pending indents %d", *pending_indents);
	
	if(*pending_indents > 0) {
		(*pending_indents)--;
		(*indent_level)++;
		return TOK_INDENT;
	} else if (*pending_indents < 0) {
		(*pending_indents)++;
		(*indent_level)--;
		return TOK_DEDENT;
	}
	
	eat_spacestabs(in, fl);
	
	//puts("consumi espacios inutiles");
	
	if(size = match_newline(in, fl)) {
		//puts("match de newline!");
		*line_start = true;
		if(size < 0) return TOK_EOF;
		else return TOK_LINE;
	} else if(size = match_exact(in, "(", buffer, fl)) return TOK_LPAREN;
	else if(size = match_exact(in, "[", buffer, fl)) return TOK_LBRACKET;
	else if(size = match_exact(in, "{", buffer, fl)) return TOK_LBRACE;
	else if(size = match_exact(in, ")", buffer, fl)) return TOK_RPAREN;
	else if(size = match_exact(in, "]", buffer, fl)) return TOK_RBRACKET;
	else if(size = match_exact(in, "}", buffer, fl)) return TOK_RBRACE;
	else if(size = match_exact(in, "\\", buffer, fl)) return TOK_BACKSLASH;
	else if(size = match_exact(in, "'", buffer, fl)) return TOK_QUOTE;
	else if(size = match_exact(in, "`", buffer, fl)) return TOK_BACKQUOTE;
	else if(size = match_exact(in, ",@", buffer, fl)) return TOK_COMMAAT;
	else if(size = match_exact(in, ",", buffer, fl)) return TOK_COMMA;
	else if(size = match_char(in, buffer, fl)) return TOK_CHAR;
	else if(size = match_string(in, buffer, fl)) return TOK_STRING;
	else if(size = match_int(in, buffer, fl)) return TOK_INT;
	else if(size = match_double(in, buffer, fl)) return TOK_DOUBLE;
	else if(size = match_exact(in, "::", buffer, fl)) return TOK_DOUBLECOLON;
	else if(size = match_exact(in, ":=", buffer, fl)) return TOK_COLONEQUALS;
	else if(size = match_exact(in, ".", buffer, fl)) return TOK_DOT;
	else if(size = match_symbol_or_keyword(in, buffer, fl)) {
		if(buffer[size - 1] == ':') return TOK_KEYWORD;
		else return TOK_SYMBOL;
	} else if(size = match_eof(in, fl)) return TOK_EOF;
	else return TOK_INVALID;
	
	*s = size;
}



/*
reader_state * reader_state_new(reader_state * prev) {
	reader_state * n = malloc(sizeof(reader_state));
	memset(n, 0, sizeof(reader_state));
	n->prev_state = prev;
	
	if(prev) {
		
	}
};*/

Atom readatom(char * buffer, int size,  enum toktype type) {
	Atom a = nil;
	
	switch(type) {
	case TOK_CHAR:
		process_char(buffer, &size);
		a = Achar(buffer[0]);
		break;
	case TOK_STRING:
		process_string(buffer, &size);
		a = AString(String_newn(buffer, size));
		break;
	case TOK_INT:
		3;
		int sign = 1;
		int base = 10;
		char * numpos = buffer, * numpos2 = NULL;
		
		if(buffer[0] == '+') numpos++;
		else if(buffer[0] == '-') {
			sign = -1;
			numpos++;
		}
		
		if(numpos2 = strpbrk(numpos, "r")) {
			*numpos2 = '\0';
			sscanf(numpos, "%d", &base);
			numpos = numpos2 + 1;
		}
		
		a = Along(sign * strtol(numpos, &numpos2, base));
		break;
	case TOK_DOUBLE:
		3;
		double dv;
		//fprintf(stderr, "%s\n", buffer);
		sscanf(buffer, "%lf", &dv);
		a = Adouble(dv);
		break;
	case TOK_COLONEQUALS:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		a = ASymbol(symbol(":="));
		break;
	case TOK_DOUBLECOLON:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		a = ASymbol(symbol("::"));
		break;
	case TOK_KEYWORD:
	case TOK_SYMBOL:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		if(!strcmp(buffer, ":")) a = Asymbol(":");
		else if(!strcmp(buffer, ":=")) a = Asymbol(":=");
		else if(buffer[size - 1] == ':') {
			buffer[size - 1] = '\0';
			a = AKeyword(keyword(buffer));
		} else {
			if(!strcmp(buffer, "true")) a = Abool(true);
			else if(!strcmp(buffer, "false")) a = Abool(false);
			else if(!strcmp(buffer, "nil")) a = nil;
			else a = ASymbol(symbol(buffer));
		}	
		break;
	default:
		puts("illegal atom");
	}
	
	return a;
}

#define APPLY_MODS_D

#ifdef APPLY_MODS_D
void apply_mods_dcons(Atom * what, Cons * mods, Cons *** dcons) {
	if(mods) {
		Cons * l = NULL;
		Cons_add(&l, mods->car);
		apply_mods_dcons(&(Cons_add(&l, * what)->car),
			Cons_next(mods), dcons);
		*what = ACons(l);
	} else {
		*dcons = &(what->d._Cons);
	}
}
#endif

Cons * readlines(istream * in, char * buffer, int * size, 
	rflags * fl, reader_state * rs, enum toktype endtoken, bool in_line) {
	//reader_state * state = reader_state_new(NULL);
	
	#ifdef APPLY_MODS_D
	Cons * modifiers = NULL;
	Cons ** dcons = NULL;
	#endif
	
	Cons * tmp;
	
	Cons * block = NULL;
	Cons ** block_inspoint = &block;
	
	Atom * prev_line = NULL;
	
	Atom line = nil;
	bool line_empty = true;
	Cons ** line_inspoint = NULL;
	
	//Cons ** block_inspoint = &block;
	
	//Cons * prev_line = NULL;
	//Cons ** prev_line_inspoint = NULL;
	
	
	
	enum toktype tok;
	
	while(tok = read_token(in, buffer, size, fl, &rs->line_start, 
		&rs->indent_level, &rs->pending_indents)) {
		
		switch(tok) {
		#ifdef APPLY_MODS_D
		case TOK_QUOTE:
			Cons_add(&modifiers, ASymbol(s_quote));
			break;
		case TOK_BACKQUOTE:
			Cons_add(&modifiers, ASymbol(s_qquote));
			break;
		case TOK_COMMAAT:
			Cons_add(&modifiers, ASymbol(s_uquotes));
			break;
		case TOK_COMMA:
			Cons_add(&modifiers, ASymbol(s_uquote));
			break;
		#endif
			
		case TOK_LPAREN:
			//puts("(");
			rs->indent_level++;
			if(line_empty) {
				line = ACons(readlines(in, buffer, size, fl, rs, TOK_RPAREN, true));
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&line, modifiers, &dcons);
				modifiers = NULL;
				#endif
				
				line_empty = false;
				line_inspoint = NULL;
			} else if(line_inspoint == NULL) {
				line = ACons(Cons_new(line, NULL));
				line_inspoint = (Cons_nextr(line.d._Cons));
				tmp = Cons_add(line_inspoint, 
					ACons(readlines(in, buffer, size, fl, rs, TOK_RPAREN, true)));
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				modifiers = NULL;
				#endif
				
				line_inspoint = Cons_nextr(*line_inspoint);
				
			} else {
				tmp = Cons_add(line_inspoint,
					ACons(readlines(in, buffer, size, fl, rs, TOK_RPAREN, true)));
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				modifiers = NULL;
				#endif
				
				line_inspoint = Cons_nextr(*line_inspoint);
			}
			
			if(in_line) {
				Cons_add(block_inspoint, line);
				assert(*block_inspoint);
				prev_line = &((*block_inspoint)->car);
				block_inspoint = Cons_nextr(*block_inspoint);
				line_empty = true;
			}
			
			break;
		case TOK_RPAREN:
			//rintf("line\n");
			if(!in_line)Cons_add(block_inspoint, line);
			//prev_line = &((*block_inspoint)->car);
			//block_inspoint = Cons_nextr(*block_inspoint);
			//line_empty = true;
			// line_inspoint wont be updated until first item is added
			
			//puts(")");
			rs->indent_level--;
			assert(endtoken == TOK_RPAREN);
			
			return block;
			break;
		case TOK_LINE:
			//fprintf(stderr, "line\n");
			if(in_line) {
				in_line = false;
				line_empty = true;
				break;
			}
			
			assert(!line_empty);
			//assert(*block_inspoint);
			Cons_add(block_inspoint, line);
			assert(*block_inspoint);
			prev_line = &((*block_inspoint)->car);
			block_inspoint = Cons_nextr(*block_inspoint);
			line_empty = true;
			
			// line_inspoint wont be updated until first item is added
			break;
		case TOK_BACKSLASH:
			in_line = false;
			//puts("\\");
			if(line_empty) {
				line = ACons(NULL);
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&line, modifiers, &dcons);
				line_inspoint = dcons;
				modifiers = NULL;
				#else
				line_inspoint = &(line.d._Cons);
				
				#endif
				
				line_empty = false;
				
				assert(line_inspoint);
			} else if(line_inspoint == NULL) {
				line = ACons(Cons_new(line, NULL));
				line_inspoint = (Cons_nextr(line.d._Cons));
				
				tmp = Cons_add(line_inspoint, ACons(NULL));
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				line_inspoint = dcons;
				modifiers = NULL;
				#else
				line_inspoint = &((*line_inspoint)->car.d._Cons);
				#endif
				
			} else {
				tmp = Cons_add(line_inspoint, ACons(NULL));
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				line_inspoint = dcons;
				modifiers = NULL;
				#else
				line_inspoint = &((*line_inspoint)->car.d._Cons);
				#endif
			}
			
			break;
		case TOK_CHAR:
		case TOK_STRING:
		case TOK_INT:
		case TOK_DOUBLE:
		case TOK_COLONEQUALS:
		case TOK_KEYWORD:
		case TOK_SYMBOL:
			//fprintf(stderr, "someatom\n");
			if(line_empty) {
				line = readatom(buffer, *size, tok);
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&line, modifiers, &dcons);
				modifiers = NULL;
				#endif
				
				line_empty = false;
				line_inspoint = NULL;
			} else if(line_inspoint == NULL) {
				line = ACons(Cons_new(line, NULL));
				line_inspoint = (Cons_nextr(line.d._Cons));
				tmp = Cons_add(line_inspoint, readatom(buffer, *size, tok));
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				modifiers = NULL;
				#endif
				
				line_inspoint = Cons_nextr(*line_inspoint);
			} else {
				tmp = Cons_add(line_inspoint, readatom(buffer, *size, tok));
				line_inspoint = Cons_nextr(tmp);
				
				#ifdef APPLY_MODS_D
				apply_mods_dcons(&(tmp->car), modifiers, &dcons);
				modifiers = NULL;
				#endif
			}
			
			if(in_line) {
				Cons_add(block_inspoint, line);
				assert(*block_inspoint);
				prev_line = &((*block_inspoint)->car);
				block_inspoint = Cons_nextr(*block_inspoint);
				line_empty = true;
			}
			
			break;
		
		case TOK_INDENT:
			//fprintf(stderr, "indent\n");
			if(prev_line) {
				if(!line_inspoint) {
					*prev_line = ACons(Cons_new(*prev_line, NULL));
					assert(prev_line->d._Cons);
					line_inspoint = (Cons_nextr(prev_line->d._Cons));
				}
			}
			
			*line_inspoint = 
				readlines(in, buffer, size, fl, rs, TOK_DEDENT, false);
			break;
		case TOK_DEDENT:
			//fprintf(stderr, "dedent\n");
			assert(endtoken == TOK_DEDENT);
			
			return block;
			break;
		case TOK_EOF:
			//fprintf(stderr, "lparen\n");
			break;
		}
	}
	return block;
	//btfree(state);
}

/*

bool match_int(istream * in, char * out, rflags * fl);
bool match_double(istream * in, char * out, rflags * fl;
bool match_bool(istream * in, char * out, rflags * fl);
bool match_char(istream * in, char * out, rflags * fl);
bool match_keyword(istream * in, char * out, rflags * fl);
bool match_symbol(istream * in, char * out, rflags * fl);
bool match_exact(istream * in, const char * sequence, char * out, rflags * fl);

void consume_whitespace(istream * in, char * out , rflags * fl);*/

