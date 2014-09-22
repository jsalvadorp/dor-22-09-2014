#include <ctype.h>
#include <assert.h>
#include "ast.h"
#include "stream.h"

enum toktype {
	OP_DOT,// = TOK_INVALID + 10,
	
	OP_GLUE, // NEW
	
	OP_MUL,
	OP_DIV,
	OP_MOD,
	
	OP_ADD,
	OP_SUB,
	
	OP_LT,
	OP_GT,
	OP_LE,
	OP_GE,
	OP_EQ,
	OP_NE,
	
	OP_AND,
	OP_OR,
	
	OP_COMMA,
	
	OP_COMPL,
	OP_COMPR,
	
	OP_TYPEI,
	
	//OP_GLUE, // OLD
	
	OP_TFMAP,
	
	OP_TPMAP,
	
	OP_TUNION,
	
	OP_DEFINE,
	OP_ASSIGN,
	//OP_DECL,
	
	OP_TYPEDEF,
	
	OP_TYPE,
	
	TOK_EOF,
	TOK_RPAREN,
	TOK_RBRACKET,
	TOK_RBRACE,
	TOK_DEDENT,
	TOK_LINE,
	
	TOK_INDENT,
	TOK_LPAREN,
	TOK_LBRACKET,
	TOK_LBRACE,
	TOK_BACKSLASH,
	TOK_QUOTE,
	
	TOK_CHAR,
	TOK_STRING,
	TOK_INT,
	TOK_FLOAT,
	TOK_KEYWORD,
	TOK_SYMBOL,
	
	TOK_INVALID
};

int nextchar(istream * in);

void nexttoken(istream * in) ;
ASTAtom group(istream * in, bool canbeempty);

extern char * nextlex, buffer1[1024], buffer2[1024], * lex;
extern int nexttok;

