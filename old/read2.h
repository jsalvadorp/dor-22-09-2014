#ifndef READ2_H
#define READ2_H

enum toktype {
	TOK_EOF = 0,
	TOK_LPAREN,
	TOK_RPAREN,
	TOK_LBRACKET,
	TOK_RBRACKET,
	TOK_LBRACE,
	TOK_RBRACE,
	TOK_BACKSLASH,
	TOK_QUOTE,
	TOK_BACKQUOTE,
	TOK_COMMAAT,
	TOK_COMMA,
	TOK_CHAR,
	TOK_STRING,
	TOK_INT,
	TOK_DOUBLE,
	TOK_DOUBLECOLON,
	TOK_COLONEQUALS,
	TOK_DOT,
	TOK_KEYWORD,
	TOK_SYMBOL,
	TOK_LINE,
	TOK_INDENT,
	TOK_DEDENT,
	TOK_INVALID
};

typedef struct reader_state {
	/*//struct reader_state * prev_state;
	int indent_level;
	Cons ** inspoint; // where the line will be added if it keeps the indentlevel
	
	Cons * prev_line; // previous line's node in the current level.
	Cons ** prev_line_inspoint; // where the previous line is open for insertion
		// null if prev line was single-item
		
	//char c;*/
	
	bool line_start;
	int indent_level;
	int pending_indents;
} reader_state;

Cons * readlines(istream * in, char * buffer, int * size, 
	rflags * fl, reader_state * rs, enum toktype endtoken, bool in_line);


#endif
