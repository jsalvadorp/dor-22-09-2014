#include <ctype.h>
#include <assert.h>
#include "read.h"
#include "types.h"
#include "cons.h"
#include "string.h"

// whitespace	: \s
// comments		: ;.*\n

// literals
// string	: "([^\\"]|\\.)*"
// int		: [+-]?([0-9]+|[0-9]+r[0-9A-Za-z])
// double	: [+-]?([0-9]*\.[0-9]+|[0-9]+e[0-9]+|[0-9]*\.[0-9]+e[0-9]+|[0-9]+E[0-9]+|[0-9]*\.[0-9]+E[0-9]+)
// bool		: true|false
// char		: \$.|\$\\.
// symbol	: [a-zA-Z0-9+*/?!|=%&~]+
// keyword	: [a-zA-Z0-9+*/?!|=%&~]+:

// other tokens
// (
// )
// \[
// \]
// {
// }
// ,
// ,@
// '
//

//char * global_buffer = NULL;
char global_buffer[READ_BUFFER_SIZE];
int global_buffer_size = 0;

char escape_char(char c) {
	switch(c) {
	case 'n': return '\n'; // no breaks
	case 'r': return '\r';
	case 't': return '\t';
	case 'a': return '\a';
	case 'b': return '\b';
	case 'f': return '\f';
	case 'v': return '\v';
	case 's': return ' ';
	// default (cases \, ", ' or others), c is already the escaped char
	}
	
	return c;			
}

#define CHECK_EOF if(ieof(in)) {fl->good = false; iseek(in, inipos); return 0; }
//#define CHECK_EOF if(c == EOF) {fl->good = false; iseek(in, inipos); return 0; }
#define ICHECK_EOF if(ieof(in)) {fl->good = false; return 0; }

#define OK_IF_EOF if(ieof(in)) {return out_pos; }
#define NOMATCH {iseek(in, inipos); return 0; }

// these ALWAYS end a token
#define END_TOKEN_CHAR(c) \
	((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == ',' || (c) == '\'' \
		|| (c) == '`'  || (c) == '"' || (c) == '$' || (c) == '(' \
		|| (c) == ')'  || (c) == '['  || (c) == ']' || (c) == '{'  || (c) == '}'  \
		|| (c) == ';' || (c) == EOF)

// these also include dot and/or colon
#define END_TOKEN_CHAR_DOT_COLON(c) \
	((c) == ' ' || (c) == '\n' || (c) == '\t' || (c) == ',' || (c) == '\'' \
		|| (c) == '`'  || (c) == '"' || (c) == '$' || (c) == '(' \
		|| (c) == ')'  || (c) == '['  || (c) == ']' || (c) == '{'  || (c) == '}'  \
		|| (c) == ';' || (c) == '.' || (c) == ':' || (c) == EOF)


void rflags_reset(rflags * this) {
	this->depth = 0;
	this->good = true;
	this->eol = false;
}

// it would be better to use regular expressions... maybe

// consume whitespace and return whether we can still read
size_t consume_whitespace(istream * in, rflags * fl) {
	//ICHECK_EOF
	//puts("dont call me");
	int c = igetc(in);
	ICHECK_EOF
	
	while(isspace(c) || c == ';') {
		if(c == ';') {
			while(c != '\n') {
				//ICHECK_EOF
				c = igetc(in);
				ICHECK_EOF
			}
		}
		
		//ICHECK_EOF
		c = igetc(in);
		ICHECK_EOF
	}
	
	iseekr(in, -1); // reread first non-ignorable character
	
	return 1; // this has no meaning
}

size_t match_char(istream * in, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	size_t out_pos = 0;
	
	size_t inipos = itell(in);
	
	
	//CHECK_EOF
	int c = igetc(in);
	CHECK_EOF
	
	if(c != '$') NOMATCH
	out[out_pos++] = c; // reads the $
	
	//CHECK_EOF
	c = igetc(in);
	CHECK_EOF
	
	out[out_pos++] = c;
	
	if(c == '\\') {
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		
		out[out_pos++] = c; //escape_char(c);
	}
	
	out[out_pos] = '\0';
	
	return out_pos; // how many characters were read
}

size_t match_string(istream * in, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	size_t out_pos = 0;
	
	size_t inipos = itell(in);
	
	//CHECK_EOF
	int c = igetc(in);
	CHECK_EOF
	
	if(c != '"') NOMATCH
	out[out_pos++] = c; // reads the first quote
	
	//CHECK_EOF
	c = igetc(in);
	CHECK_EOF
	
	while(c != '"') {
		out[out_pos++] = c;
		
		if(c == '\\') {
			//CHECK_EOF
			c = igetc(in);
			CHECK_EOF
			
			out[out_pos++] = c; //escape_char(c);
		}
		
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
	}
	
	out[out_pos++] = c; // last quote
	out[out_pos] = '\0';
	return out_pos; // how many characters were read
}

size_t match_int(istream * in, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	size_t out_pos = 0, npos = 0;
	
	size_t inipos = itell(in);
	
	//CHECK_EOF
	int c = igetc(in);
	CHECK_EOF
	
	if(c == '-' || c == '+') {
		//puts("signed number!");
		out[out_pos++] = c; // read the sign
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
	}
	
	//printf("first digit is %c\n", (char)c);
	
	if(!isdigit(c)) NOMATCH
	out[out_pos++] = c; // reads the first digit
	
	//OK_IF_EOF
	c = igetc(in);
	OK_IF_EOF
	
	while(isdigit(c) /*&& c != EOF*/) {
		out[out_pos++] = c;
		
		//OK_IF_EOF
		c = igetc(in);
		OK_IF_EOF
	}
	
	if(c == 'r') {
		out[out_pos++] = c;
		// check if characters are valid in specified radix...
		npos = out_pos;
		char radix[20];
		int base;
		out[out_pos] = 0;
		sscanf(out, "%d", &base);
		if(base < 0) base = -base;
		
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		
		while((isdigit(c) && (c - '0' < base)) || (isalpha(c) && (10 + tolower(c) - 'a' < base))) {
			out[out_pos++] = c;
		
			//OK_IF_EOF
			c = igetc(in);
			OK_IF_EOF
		}
		
		if(out_pos == npos) // read no number
			NOMATCH
	} 
	
	if(c == '.') {
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		
		if(isdigit(c)) NOMATCH // this is not an int, it's a double!
		else iseekr(in, -1); // go back to reread the thing after the dot.
		c = '.';
	}
	
	if(END_TOKEN_CHAR_DOT_COLON(c) && out_pos != npos) {
		iseekr(in, -1); // will reread termination token
		
		out[out_pos] = '\0';
		return out_pos;
	} else NOMATCH
}

size_t match_double(istream * in, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	//puts("double0");
	size_t out_pos = 0, npos;
	
	size_t inipos = itell(in);
	
	//CHECK_EOF
	int c = igetc(in);
	CHECK_EOF
	//puts("double1");
	
	if(c == '-' || c == '+') {
		out[out_pos++] = c; // read the sign
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
	}
	//puts("double2");
	npos = out_pos;
	
	while(isdigit(c) /*&& c != EOF*/) {
		out[out_pos++] = c;
		
		//OK_IF_EOF
		c = igetc(in);
		OK_IF_EOF
	}
	//puts("double3");
	if(c == '.') {
		out[out_pos++] = c;
		npos = out_pos;
		
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		//puts("double4");
		while(isdigit(c)) {
			out[out_pos++] = c;
		
			//OK_IF_EOF
			c = igetc(in);
			OK_IF_EOF
		}
		//puts("double5");
		if(npos == out_pos) { // number ended in a dot... dot is then dot operator
			// but that is already checked in int... so what
		}
	}
	//puts("double6");
	if(c == 'e' || c == 'E') {
		out[out_pos++] = c;
		
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		
		//puts("double7");
		if(c == '-' || c == '+') {
			out[out_pos++] = c; // read the sign
			
			//CHECK_EOF
			c = igetc(in);
			CHECK_EOF
		}
		//puts("double8");
		npos = out_pos;
		
		while(isdigit(c)) {
			out[out_pos++] = c;
			//puts("double9");
			
			//OK_IF_EOF
			c = igetc(in);
			OK_IF_EOF
		}
		//puts("double10");
	} 
	
	if(END_TOKEN_CHAR_DOT_COLON(c) && out_pos != npos) {
		//puts("matched double");
		//printf("double11%zu\n", out_pos);
		iseekr(in, -1);
		out[out_pos] = '\0';
		return out_pos;
	} else NOMATCH
}

// symbol	: [a-zA-Z0-9+*/?!|=%&~]+
// keyword	: [a-zA-Z0-9+*/?!|=%&~]+:

size_t match_symbol_or_keyword(istream * in, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	//puts("msk0");
	
	size_t out_pos = 0;
	
	size_t inipos = itell(in);
	
	//ICHECK_EOF
	int c = igetc(in);
	ICHECK_EOF
	
	//puts("msk1");
	
	if(isdigit(c)) NOMATCH
	
	//puts("msk2");
	
	//printf("symbolread: %c\n", (char)c);
	
	//while(isalnum(c) || c == ') {
	while(!END_TOKEN_CHAR_DOT_COLON(c)) { //liberal definition of symbol
		out[out_pos++] = c;
		
		//OK_IF_EOF
		c = igetc(in);
		OK_IF_EOF
		//printf("symbolread: %c\n", (char)c);
		
	}
	
	//puts("msk3");
	
	if(c == ':') { //keyword
		//OK_IF_EOF
		c = igetc(in);
		OK_IF_EOF
		/*if(c == '=') {
			if(out_pos == 0 && END_TOKEN_CHAR_DOT_COLON(igetc(in))) {
				out[out_pos++] = ':';
				out[out_pos++] = '=';
			} else NOMATCH
		} else*/ if(c != ':') out[out_pos++] = ':';
		else iseekr(in, -1);
	}
	
	//puts("msk4");
	
	iseekr(in, -1);
	
	out[out_pos] = '\0';
	return out_pos; // how many characters were read
}

size_t match_exact(istream * in, const char * sequence, char * out, rflags * fl) {
	//ICHECK_EOF
	//consume_whitespace(in, fl);
	ICHECK_EOF
	
	size_t len = strlen(sequence);
	int c;
	size_t inipos = itell(in);
	
	size_t i;
	for(i = 0; i < len; i++) {
		//CHECK_EOF
		c = igetc(in);
		CHECK_EOF
		
		if(sequence[i] == c) out[i] = c;
		else NOMATCH
	}
	
	out[i] = '\0';
	return len;
}

void process_string(char * in, int * pos) {
	char * out = in++; // skip opening quote
	*pos = 0;
	
	//printf("string %s\n", in);
	
	while((*out = *(in++)) != '"') {
		//printf("char %c \n", *out);
		if(*out == '\\') {
			*out = escape_char(*(in++));
		}
		(*pos)++;
		out++;
	}
	
	(*out) = '\0';
}

void process_char(char * buffer, int * pos) {
	buffer[0] = buffer[1];
	if(buffer[1] == '\\')
		buffer[0] = escape_char(buffer[2]);
	buffer[1] = '\0';
	
	*pos = 1;
}

Atom reada(istream * in, char * buffer, rflags * fl) {
	int pos;
	Atom a = nil;
	//consume_whitespace(in, fl);
	
	assert(!ieof(in));
	
	if(pos = match_char(in, buffer, fl)) {
		process_char(buffer, &pos);
		a = Achar(buffer[0]);
	} else if(pos = match_string(in, buffer, fl)) {
		process_string(buffer, &pos);
		a = AString(String_newn(buffer, pos));
	} else if(pos = match_int(in, buffer, fl)) {
		buffer[pos] = '\0';
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
	} else if(pos = match_double(in, buffer, fl)) {
		buffer[pos] = '\0';
		double dv;
		//printf("%s\n", buffer);
		sscanf(buffer, "%lf", &dv);
		a = Adouble(dv);
	} else if(pos = match_exact(in, "::", buffer, fl)) {
		a = ASymbol(symbol("::"));
	} else if(pos = match_exact(in, ":=", buffer, fl)) {
		a = ASymbol(symbol(":="));
	} else if(pos = match_exact(in, ".", buffer, fl)) {
		a = ASymbol(symbol("."));
	} else if(pos = match_symbol_or_keyword(in, buffer, fl)) {
		buffer[pos] = '\0';
		if(!strcmp(buffer, ":")) a = Asymbol(":");
		else if(!strcmp(buffer, ":=")) a = Asymbol(":=");
		else if(buffer[pos - 1] == ':') {
			buffer[pos - 1] = '\0';
			a = AKeyword(keyword(buffer));
		} else {
			if(!strcmp(buffer, "true")) a = Abool(true);
			else if(!strcmp(buffer, "false")) a = Abool(false);
			else if(!strcmp(buffer, "nil")) a = nil;
			else a = ASymbol(symbol(buffer));
		}
			
	} else {
		printf("Unrecognized token at %ld, '%d', nl is %d", itell(in), igetc(in), (int)'\n');
		printf("buffer: %s\n", buffer);
		assert(false);
	}
	
	//puts(cstr(toREPLString(&a)));
	consume_whitespace(in, fl);
	
	return a;
}

Atom apply_mods(Atom what, Cons * mods) {
	if(mods) {
		Cons * l = NULL;
		Cons_add(&l, mods->car);
		Cons_add(&l, apply_mods(what, Cons_next(mods)));
		return ACons(l);
	} else return what;
}

Cons * readl(istream * in, char * buffer, rflags * fl) {
	Cons * l = NULL;
	Cons * modifiers = NULL; // CONSIDER MAKING THIS UNGC (destroy on clear)
	
	while(fl->good && !ieof(in) && consume_whitespace(in, fl)) {
		if(match_exact(in, "(", buffer, fl)) {
			fl->depth++;
			Cons_add(&l, apply_mods(ACons(readl(in, buffer, fl)), modifiers));
			modifiers = NULL;
		} else if(match_exact(in, "[", buffer, fl)) {
			fl->depth++;
			Cons * ll = NULL;
			Cons_add(&ll, ASymbol(s_lambda));
			Cons * lm = NULL;
			Cons_add(&lm, ASymbol(symbol("_")));
			Cons_add(&ll, ACons(lm));
			Cons_add(&ll, ACons(readl(in, buffer, fl)));
			Cons_add(&l, apply_mods(ACons(ll), modifiers));
			modifiers = NULL;
		} else if(match_exact(in, "{", buffer, fl)) {
			fl->depth++;
			Cons * ll = NULL;
			Cons_add(&ll, ASymbol(s_expr));
			Cons_append(&ll, readl(in, buffer, fl));
			Cons_add(&l, apply_mods(ACons(ll), modifiers));
			modifiers = NULL;
		} else if (match_exact(in, ")", buffer, fl) || match_exact(in, "]", buffer, fl) || match_exact(in, "}", buffer, fl)) {
			fl->depth--;
			break;
		} else if (match_exact(in, "'", buffer, fl)) {
			Cons_add(&modifiers, ASymbol(s_quote));
		} else if (match_exact(in, "`", buffer, fl)) {
			Cons_add(&modifiers, ASymbol(s_qquote));
		} else if (match_exact(in, ",@", buffer, fl)) {
			Cons_add(&modifiers, ASymbol(s_uquotes));
		} else if (match_exact(in, ",", buffer, fl)) {
			Cons_add(&modifiers, ASymbol(s_uquote));
		} else {
			Cons_add(&l, apply_mods(reada(in, buffer, fl), modifiers));
			modifiers = NULL;
		}
	}
	
	buffer[0] = '\0'; // clean the buffer
	return l;
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

