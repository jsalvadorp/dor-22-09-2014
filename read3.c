#include <ctype.h>
#include <assert.h>
#include "read3.h"

#define APPENDNEXTC {nextlex[nextlexpos++] = nextchar(in); nextlex[nextlexpos] = 0;}
#define SCANASSERT(x) if(!(x)) return TOK_INVALID;

#define TOK_IS_OP(x) ((x) < TOK_EOF)
#define TOK_IS_END(x) ((x) >= TOK_EOF && (x) <= TOK_LINE)
#define TOK_IS_EXPR(x) ((x) >= TOK_LPAREN && (x) <= TOK_SYMBOL)


#define DBPRINT(msg, ...) if(1){for(int i = 0; i < depth; i++) fprintf(stderr, "\t"); fprintf(stderr, msg, __VA_ARGS__);}

int nexttok;
int nexttokline;
char *nextlex;
int nextlexpos;

int tok;
int tokline;
char *lex;
int lexpos;

int nextc;
char buffer1[1024], buffer2[1024];

int isdelim(int c) {
	return c == ' ' || c == '\t' || c == '\n' || c == '\r'
		|| c == '(' || c == ')' || c == '\\'
		|| c == '[' || c == ']'
		|| c == '{' || c == '}' || c == '.' || c == EOF
		|| c == ',' || c == '\'' || c == '\"' || c == '$';
}

char escapechar(char c) {
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

int nextchar(istream * in) {
	int old = nextc;
	
	nextc = igetc(in);
	
	return old;
}


// check eofs!!!!!!!!
// allow _ in numbers
int scannumber(istream * in) {
		while(isdigit(nextc)) {
			APPENDNEXTC;
		}
		
		if(nextc == '.') {
			APPENDNEXTC;
			
			SCANASSERT(isdigit(nextc));
			
			while(isdigit(nextc)) {
				APPENDNEXTC;
			}
			
			if(nextc == 'e' || nextc == 'E') {
				APPENDNEXTC;
				
				SCANASSERT(isdigit(nextc) || nextc == '+' || nextc == '-');
				
				if(nextc == '+' || nextc == '-')
					APPENDNEXTC;
			
				while(isdigit(nextc)) {
					APPENDNEXTC;
				}
			
				
			}
			
			if(isdelim(nextc)) {
				//nextlex[nextlexpos] = 0;
				return TOK_FLOAT;
			} else return TOK_INVALID;
		} else if(nextc == 'e' || nextc == 'E') {
			APPENDNEXTC;
			
			SCANASSERT(isdigit(nextc) || nextc == '+' || nextc == '-');
			
			if(nextc == '+' || nextc == '-')
				APPENDNEXTC;
				
				while(isdigit(nextc)) {
					APPENDNEXTC;
			}
			
			if(isdelim(nextc)) {
				//nextlex[nextlexpos] = 0;
				return TOK_FLOAT;
			} else return TOK_INVALID;
		} else if(nextc == 'r' || nextc == 'R') {
			APPENDNEXTC;
			
			SCANASSERT(isdigit(nextc));
			
			while(isdigit(nextc)) {
				APPENDNEXTC;
			}
			
			if(isdelim(nextc)) {
				//nextlex[nextlexpos] = 0;
				return TOK_FLOAT;
			} else return TOK_INVALID;
		} else if(isdelim(nextc)) {
			//nextlex[nextlexpos] = 0;
			return TOK_INT;
		} else return TOK_INVALID;
}

int scansym(istream * in) {
	while(!isdelim(nextc)) {
		APPENDNEXTC;
	}
		
	SCANASSERT(nextlexpos);
		
	if(!nextlexpos) return TOK_INVALID;
		
	#define CASE_OP(str, tok) if(!strcmp(nextlex, str)) { return OP_##tok; }
	
	CASE_OP("*", MUL)
	CASE_OP("/", DIV)
	CASE_OP("%", MOD)
	CASE_OP("+", ADD)
	CASE_OP("-", SUB)
	CASE_OP("<", LT)
	CASE_OP(">", GT)
	CASE_OP("<=", LE)
	CASE_OP(">=", GE)
	CASE_OP("==", EQ)
	CASE_OP("!=", NE)
	CASE_OP("&&", AND)
	CASE_OP("||", OR)
	CASE_OP(",", COMMA)
	CASE_OP("<<", COMPL)
	CASE_OP(">>", COMPR)
	CASE_OP("->", TFMAP)
	CASE_OP("=>", TPMAP)
	CASE_OP("|", TUNION)
	CASE_OP("=", DEFINE)
	CASE_OP("<-", ASSIGN)
	//CASE_OP("<=", DECL)
	
	return TOK_SYMBOL;
}

bool line_start;
//bool anythingyet;
int pending_indents;
int pending_line;
int indent_level;
int icounter;

int linecount;

void eatblanks(istream * in) { // handle multiline nestable comments...
	int exit = 0;
	//printf("blank ");
	while(!exit) {
		//printf("%c", nextc != '\n' ? nextc : ' ');
		switch(nextc) {
		case EOF:
			exit = 1;
			break;
		case ' ':
			line_start = false;
			nextchar(in);
			break;
		case '\r':
			break; // windows newline: ignore r, only count n
		case '\n':
			pending_line = true;
			//anythingyet = false;
			line_start = true;
			icounter = 0;
			pending_indents = -indent_level;
			nextchar(in);
			
			linecount++;
			
			break;
		case '\t':
			if(line_start) {
				for(icounter = 0; nextc == '\t'; icounter++) nextchar(in);
				
				pending_indents = icounter - indent_level;
				//if(icounter > 
				line_start = false;
				// allow multiple indents?????? NO!
			} else nextchar(in);
			break;
		case ';':
			nextchar(in);
			while(nextc != '\n' && nextc != '\r') {
				nextchar(in);
			}
			break;
		default:
			exit = 1;
		}
	}
	//puts("");
}

int scan(istream * in, int * lcount) {
	eatblanks(in);
	
	//puts("aqui");
	
	if(nextc == EOF) return TOK_EOF;
	
	//fprintf(stderr, "pending indents %d", *pending_indents);
	if(pending_line /*&& anythingyet*/) {
		nextlex[0] = 0;
		pending_line = 0;
		return TOK_LINE;
	} 
	
	if(pending_indents > 0) {
		nextlex[0] = 0;
		pending_indents--;
		indent_level++;
		return TOK_INDENT;
	} else if (pending_indents < 0) {
		nextlex[0] = 0;
		pending_indents++;
		indent_level--;
		return TOK_DEDENT;
	}
	
	//anythingyet = true;
	line_start = false;
	
	int ctok, escapeop = false;
	
	*lcount = linecount;
	
	if(nextc == '#') {
		escapeop = true;
		nextchar(in);
	}
	
	if(isdigit(nextc)) {
		ctok = scannumber(in);
	} else if(nextc == '+' || nextc == '-') {
		APPENDNEXTC;
		if (isdelim(nextc)) {
			ctok = nextlex[0] == '+' ? OP_ADD : OP_SUB;
		} else if(isdigit(nextc)) {
			ctok = scannumber(in);
		} else {
			ctok = scansym(in); //symbol starting in sign, maybe an operator
		}
	} else if(nextc == '.') {
		// does not accept float numbers like ".34", must be "0.34"
		APPENDNEXTC;
		ctok = OP_DOT;
	} else if(nextc == ':') {
		APPENDNEXTC;
		
		if(nextc == ':') {
			APPENDNEXTC;
			
			ctok = OP_TYPE;
		} else if(nextc == '=') {
			APPENDNEXTC;
			
			ctok = OP_TYPEDEF;
		} else ctok = OP_TYPEI;
	} else if(nextc == ',') {
		APPENDNEXTC;
		
		ctok = OP_COMMA;
	} else if(nextc == '(') {
		APPENDNEXTC;
		
		ctok = TOK_LPAREN;
	} else if(nextc == ')') {
		APPENDNEXTC;
		
		ctok = TOK_RPAREN;
	} else if(nextc == '[') {
		APPENDNEXTC;
		
		ctok = TOK_LBRACKET;
	} else if(nextc == ']') {
		APPENDNEXTC;
		
		ctok = TOK_RBRACKET;
	} else if(nextc == '{') {
		APPENDNEXTC;
		
		ctok = TOK_LBRACE;
	} else if(nextc == '}') {
		APPENDNEXTC;
		
		ctok = TOK_RBRACE;
	} else if(nextc == '\\') {
		APPENDNEXTC;
		
		ctok = TOK_BACKSLASH;
	} else if(nextc == '\'') {
		APPENDNEXTC;
		
		ctok = TOK_QUOTE;
	} else if(nextc == '$') {
		APPENDNEXTC;
		
		if(nextc == '\\') APPENDNEXTC;
		
		APPENDNEXTC;
		
		ctok = TOK_CHAR;
	} else if(nextc == '\"') {
		APPENDNEXTC;
		
		while(nextc != '\"') {
			if(nextc == '\\') nextlex[nextlexpos++] = nextchar(in);
			
			APPENDNEXTC;
		}
		
		APPENDNEXTC;
		
		ctok = TOK_STRING;
	} else ctok = scansym(in);
	
	if (TOK_IS_OP(ctok) && escapeop) return TOK_SYMBOL;
	
	return ctok;
}

const char * opnames[] = {
	".",// = TOK_INVALID + 10,
	
	"", // NEW
	
	"*",
	"/",
	"%",
	
	"+",
	"-",
	
	"<",
	">",
	"<=",
	">=",
	"==",
	"!=",
	
	"&&",
	"||",
	
	",",
	
	"<<",
	">>",
	
	":",
	
	//"", // OLD
	
	"->",
	
	"=>",
	
	"|",
	
	"=",
	"<-", // replace with :=
	//"<=", stupid
	
	":=",
	
	"::"
};

int precedence[] =  {
	110,//OP_DOT = TOK_INVALID + 10,
	
	100,//OP_GLUE
	
	90,//OP_MUL,
	90,//OP_DIV,
	90,//OP_MOD,
	
	80,//OP_ADD,
	80,//OP_SUB,
	
	70,//OP_LT,
	70,//OP_GT,
	70,//OP_LE,
	70,//OP_GE,
	70,//OP_EQ,
	70,//OP_NE,
	
	60,//OP_AND,
	60,//OP_OR,
	
	50,//OP_COMMA,
	
	40,//OP_COMPL,
	40,//,//OP_COMPR,
	
	10,//OP_TYPEI,
	
	// 0,//OP_GLUE, // OLD
	
	-10,//,//OP_TFMAP,->
	
	-20,//OP_TPMAP,=>
	
	-30,//OP_TUNION,
	
	-40,//OP_DEFINE,
	-40,//OP_ASSIGN,
	//-40,//OP_DECL,
	
	-50,//OP_TYPEDEF,
	
	-60//OP_TYPE
};

#define LA 0
#define RA 1
#define VA 2
#define NA 3

int associativity[] =  {
	LA,//OP_DOT = TOK_INVALID + 10,
	
	VA,//OP_GLUE,
	
	LA,//OP_MUL,
	LA,//OP_DIV,
	LA,//OP_MOD,
	
	LA,//OP_ADD,
	LA,//OP_SUB,
	
	LA,//OP_LT,
	LA,//OP_GT,
	LA,//OP_LE,
	LA,//OP_GE,
	LA,//OP_EQ,
	LA,//OP_NE,
	
	LA,//OP_AND, or v
	LA,//OP_OR, or v
	
	VA,//OP_COMMA,
	
	LA,//OP_COMPL,
	LA,//,//OP_COMPR,
	
	NA,//OP_TYPEI,
	
	// VA,//OP_GLUE,
	
	RA,//,//OP_TFMAP,->
	
	RA,//OP_TPMAP,=>
	
	VA,//OP_TUNION,
	
	RA,//OP_DEFINE,
	RA,//OP_ASSIGN,
	//RA,//OP_DECL,
	
	RA,//OP_TYPEDEF,
	
	NA//OP_TYPE
};

int depth;

void nexttoken(istream * in) {
	tok = nexttok;
	tokline = nexttokline;
	lexpos = nextlexpos;
	
	//swap buffers
	char * tmp = lex;
	lex = nextlex;
	nextlex = tmp;
	
	nextlexpos = 0;
	nexttok = scan(in, &nexttokline);
	
	DBPRINT("%d -> %s\n", nexttok, nextlex);
}

void match(istream  * in, enum toktype token) {
	assert(nexttok == token);
	nexttoken(in);
}


ASTAtom scanatom(istream * in) {
	ASTAtom a = nil;
	
	DBPRINT("scanning %d %s\n", nexttok, nextlex);
	
	int token = nexttok;
	
	if(TOK_IS_OP(token)) token = TOK_SYMBOL;
	
	char *out = nextlex, *lexeme = nextlex;
	int size = 0;
	
	nexttoken(in);
	
	int previndent = indent_level;
	
	int prevline = nexttokline;
	
	switch(token) {
	case TOK_CHAR:
		lexeme++; // ignore $
		a.type = CharType;
		a.d._char = (*lexeme == '\\') ? escapechar(lexeme[1]) : *lexeme;
		break;
	case TOK_STRING:
		1;
		char * orig = out;
		lexeme++;
		
		while(*lexeme != '"') {
			if(*lexeme == '\\') {
				lexeme++;
				*(out++) = escapechar(*(lexeme++));
			} else *(out++) = *(lexeme++);
			size++;
		}
		
		*(out++) = 0;
		
		Symbol s = symbol(orig);
		
		a = Acstring(Symbol_getstr(s));
		break;
	case TOK_INT:
		3;
		int sign = 1;
		int base = 10;
		char *numpos = lexeme, *numpos2 = NULL;
		
		if(numpos[0] == '+') numpos++;
		else if(numpos[0] == '-') {
			sign = -1;
			numpos++;
		}
		
		if(numpos2 = strpbrk(numpos, "r") ) {
			*numpos2 = '\0';
			sscanf(numpos, "%d", &base);
			numpos = numpos2 + 1;
		}
		
		a = Along(sign * strtol(numpos, &numpos2, base));
		break;
	case TOK_FLOAT:
		3;
		double dv;
		//fprintf(stderr, "%s\n", buffer);
		sscanf(lexeme, "%lf", &dv);
		a = Adouble(dv);
		break;
	case TOK_SYMBOL:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		/*if(lexeme[size - 1] == ':') {
			lexeme[size - 1] = '\0';
			a = AKeyword(keyword(lexeme));
		} else*/ {
			if(!strcmp(lexeme, "true")) a = Abool(true);
			else if(!strcmp(lexeme, "false")) a = Abool(false);
			//else if(!strcmp(lexeme, "nil")) a = nil;
			else a = ASymbol(symbol(lexeme));
		}	
		break;
	/*case TOK_LBRACE:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		a = group(in, false);
		match(in, TOK_RBRACE);
		
		break;*/
	case TOK_LPAREN:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		a = group(in, true);
		match(in, TOK_RPAREN);
		
		break;
	/*case TOK_BACKSLASH:
		//fprintf(stderr, "SYMBOL %s\n", buffer);
		a = group(in, -1, true);
		break;*/
	default:
		//puts("illegal atom");
		assert(0);
	}
	
	a.line = prevline + 1;
	
	return a;
}


int scanop(istream * in) {
	if(TOK_IS_OP(nexttok)) {
		nexttoken(in);
		
		return tok;
	} else if(TOK_IS_EXPR(nexttok)) {
		return OP_GLUE;
	} else return nexttok;
}

#define ASTACK ASTList *
#define AS_TOP(x) (x)->car
#define AS_POP(x) (x) = (x)->next
#define AS_PUSH(x, y) {(x) = ASTList_new(y, (x));}
#define AS_EMPTY(x) ((x) == NULL)
#define AS_NOT_EMPTY(x) (x)
#define AS_NEW NULL

int infix_helper(ASTACK * operators, ASTACK * operands, ASTACK * operand_dests, ASTACK * operand_heads);



ASTAtom group(istream * in, bool canbeempty) {
	
	//assert(nexttok != endtoken);
	DBPRINT("grouping starting at token %d -> %s\n", nexttok, nextlex);
	depth++;
	
	ASTACK operators = AS_NEW;
	ASTACK operands = AS_NEW;
	ASTACK operand_dests = AS_NEW;
	ASTACK operand_heads = AS_NEW;
	
	// if endtoken is })] and read a line... indentlevel++
	
	bool anythingyet = false, hanging = false;
	
	int op = -1, lastop = -1;
	
	if(!TOK_IS_END(nexttok)) {
	
		AS_PUSH(operands, scanatom(in));
		AS_PUSH(operand_dests, nil);
		AS_PUSH(operand_heads, Along(-1));
		
		anythingyet = true;
	
		
	
		while(!TOK_IS_END(nexttok)) {
			op = scanop(in);
		
			DBPRINT("op is %d %s\n", op, lex);
		
			while(AS_NOT_EMPTY(operators) 
				&& precedence[AS_TOP(operators).d._long] 
					>= precedence[op]) {
			
				lastop = infix_helper(&operators, &operands, &operand_dests, &operand_heads);
			}
		
			AS_PUSH(operators, Along(op));
			//assert(!TOK_IS_END(nexttok));
			if(TOK_IS_END(nexttok)) {
				hanging = true;
				break;
			}
		
			// check if nexttok != endtoken. what if operstor is left hanging
			// maybe it should just increase indent level and eat the line.
			// or if endtoken is {} or () give an error...
			assert(TOK_IS_EXPR(nexttok));
			AS_PUSH(operands, scanatom(in));
			AS_PUSH(operand_dests, nil);
			AS_PUSH(operand_heads, Along(-1));
		}
	}
	
	// if hanging expect line indent and absorb dedent...
	//if(hanging) printf("it is hanging \n");
	
	
	//puts("after");
	if(nexttok == TOK_LINE && (nexttoken(in), nexttok == TOK_INDENT)) {
		nexttoken(in);
		
		if(!anythingyet || hanging) {
			AS_PUSH(operands, group(in, false));
			AS_PUSH(operand_dests, nil);
			AS_PUSH(operand_heads, Along(-1));
			anythingyet = true;
		} 
		
		if(!hanging) {
		
			while(!TOK_IS_END(nexttok)) {
				// what if op is left hanging!! 
				while(AS_NOT_EMPTY(operators) 
					&& precedence[AS_TOP(operators).d._long] 
						>= precedence[OP_GLUE]) {
			
					lastop = infix_helper(&operators, &operands, &operand_dests, &operand_heads);
				}
			
				AS_PUSH(operators, Along(OP_GLUE));
			
				AS_PUSH(operands, group(in, false));
				AS_PUSH(operand_dests, nil);
				AS_PUSH(operand_heads, Along(-1));
				//nexttoken(in);
				// GLUE
			}
			// absorb dedent
			if(nexttok == TOK_DEDENT) nexttoken(in);
			else indent_level--;
		} else {
			if(nexttok == TOK_DEDENT) nexttoken(in);
			else indent_level--;
			assert(TOK_IS_END(nexttok));
			// (if (> (if (> a b) a b) c) (if (> a b) a b) c)
		}
	}
	//puts("after2");
	
	while(AS_NOT_EMPTY(operators)) {
		lastop = infix_helper(&operators, &operands, &operand_dests, &operand_heads);
	}
		
	// dump(operands.top(), "");
	DBPRINT("lstop is %d cbe %d\n", lastop, canbeempty);
	
	
	depth--;
	
	if(!anythingyet)
		return nil;
	
	//if(canbeempty) {
		/*if(!anythingyet)
			return nil;
		else if(lastop != OP_GLUE)
			return AASTList(ASTList_new(AS_TOP(operands), NULL));*/
	//}
	
	return AS_TOP(operands);
}

int infix_helper(ASTACK * operators, ASTACK * operands, ASTACK * operand_dests, ASTACK * operand_heads) {
	int lastop = -1;
	
	ASTAtom operand2 = AS_TOP(*operands);
	AS_POP(*operands);
	AS_POP(*operand_dests);
	AS_POP(*operand_heads);
						
	ASTAtom operand1 = AS_TOP(*operands);
	ASTList * operand_destination = AS_TOP(*operand_dests).d._ASTList;
	enum toktype operand_head = AS_TOP(*operand_heads).d._long;
	AS_POP(*operands);
	AS_POP(*operand_dests);
	AS_POP(*operand_heads);
	
	// MAKE ATOM COPARISON OPS!!!!!!!
	lastop = AS_TOP(*operators).d._long;
	
	if (operand_head != -1 && (operand_head == AS_TOP(*operators).d._long)
		&& (associativity[AS_TOP(*operators).d._long] != LA)) {
		// assert(associativity[AS_TOP(*operators).d._long] != LA);
		
		// variadic associative
		// 4 + 3 + 2 + 1 becomes (+ 4 3 2 1)
		if(associativity[AS_TOP(*operators).d._long] == VA) {
			operand_destination = (operand_destination->next = ASTList_new(operand2, NULL));
			AS_PUSH(*operands, operand1); // modified to include operand2
			AS_PUSH(*operand_dests, AASTList(operand_destination));
			AS_PUSH(*operand_heads, Along(AS_TOP(*operators).d._long));
		}
							
		// right associative
		// 4 + 3 + 2 + 1 becomes (+ 4 (+ 3 (+ 2 1)))
		else if (associativity[AS_TOP(*operators).d._long] == RA) {
			ASTList * sexp = NULL;
			ASTList * tmp = (sexp = ASTList_new(operand2, sexp));
			sexp = ASTList_new(operand_destination->car, sexp);	
			sexp = ASTList_new(Asymbol(opnames[AS_TOP(*operators).d._long]), sexp);
			operand_destination->car = AASTList(sexp);
			
			operand_destination = tmp;	
			
			//operands.push(ASTAtom(sexp));
			
			AS_PUSH(*operands, operand1); // modified to include operand2
			AS_PUSH(*operand_dests, AASTList(operand_destination));
			AS_PUSH(*operand_heads, Along(AS_TOP(*operators).d._long));
		}
		
		AS_POP(*operators);
	}
							
	// left associative
	// 4 + 3 + 2 + 1 becomes (+ (+ (+ 4 3) 2) 1)
	// or first application
	// 5 # 6 becomes (# 5 6) regardless of #'s associativity
	else {
		ASTList * sexp = NULL;
		ASTList * tmp = (sexp = ASTList_new(operand2, sexp));
		sexp = ASTList_new(operand1, sexp);
		if(AS_TOP(*operators).d._long != OP_GLUE) sexp = ASTList_new(Asymbol(opnames[AS_TOP(*operators).d._long]), sexp);
		
		operand_destination = (associativity[AS_TOP(*operators).d._long] == LA)
			? NULL
			: tmp;
		
		
		AS_PUSH(*operands, AASTList(sexp));
		AS_PUSH(*operand_dests, AASTList(operand_destination));
		AS_PUSH(*operand_heads, Along(AS_TOP(*operators).d._long));
		
		AS_POP(*operators);
	}
	
	return lastop;
}

#if 0
int main() {
	init_types();
	
	ifstream ifs = ifstream_make(stdin);
	istream * in = (istream *) &ifs;
	
	nextchar(in);
	
	nextlex = buffer1;
	lex = buffer2;
	
	nexttoken(in);
	
	if(nexttok == TOK_LINE) nexttoken(in);
	
	ASTList * program = NULL, ** dest = &program;
	
	while(nexttok != TOK_EOF) {
		//printf("%d -> %s\n", nexttok, nextlex);
		//nexttoken(in);
		
		*dest = ASTList_new(group(in, false), NULL);
		dest = &((*dest)->next);
	}
	
	dump(AASTList(program), 0);
}



fac n = cond
	n == 0
		println 0
		1
	else
		n * \fac n - 1
		
a = x -> y -> y + x + 3

a <- 3 +
	4 +
	

SYM SYM OP SYM INDENT SYM OP INT INDENT SYM INT LINE INT DEDENT SYM 
	INDENT SYM OP BACKS SYM SYM OP INT EOF

{fac GLUE n = cond GLUE {n == 0 GLUE {1}} GLUE {else GLUE {n * (fac GLUE n - 1)}}}


(fac GLUE 1 GLUE 2 GLUE 3) 	=> (fac 1 2 3)
(fac)						=> (fac)
{fac GLUE 1 GLUE 2 GLUE 3}	=> (fac 1 2 3)
{fac}						=> fac

OLD:::

	TOK_EOF = 0,
	TOK_LINE,
	TOK_INDENT,
	TOK_DEDENT,
	
	TOK_KEYWORD,
	TOK_SYMBOL,
	
	TOK_INVALID

	l,r,v,n - associatiivity left right variadic none

	.				l (or v?)
	* / %			l
	+ -				l
	< > <= >= == !=	l
	|| &&			l
	,				v
	<< >>			
	:				n inline types
	GLUE			v
	->				r (or v? should be v for simplicity...)
	=>				r
	|				v
	= <-			r
	:=				r
	::				n whole line types


{fac GLUE n = cond GLUE {n == 0 GLUE {1}} GLUE {else GLUE {n * (fac GLUE n - 1)}}}


pred <- {x y -> x / y}

map {x -> do
		println x
		Cons x Nil}
	
	list	
	
	
neg n = 0 - n

import std.io

main () = do
	hi = "Hello"
	i <= 0
	
	while i < 10
		print hi
		println i
		
		i <- i + 1

NEW rules
	l,r,v,n - associatiivity left right variadic none

	.				l (or v?)
	GLUE			v
	* / %			l
	+ -				l
	< > <= >= == !=	l
	|| &&			l
	,				v
	<< >>			
	:				n inline types
	->				r (or v? should be v for simplicity...)
	=>				r
	|				v
	= <-			r
	:=				r
	::				n whole line types

#endif


