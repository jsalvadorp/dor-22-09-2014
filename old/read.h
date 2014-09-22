#ifndef READ_H
#define READ_H

#include <stdio.h>
#include "types.h"
#include "stream.h"

#define READ_BUFFER_SIZE 2000

extern char global_buffer [READ_BUFFER_SIZE] ;
extern int global_buffer_size;

// whitespace	: \s
// comments		: ;.*\n

// literals
// string	: "([^\\"]|\\.)*"
// int		: [+-]?([0-9]+|[0-9]+r[0-9A-Za-z])
// double	: [+-]?([0-9]*\.[0-9]+|[0-9]+e[0-9]+|[0-9]*\.[0-9]+e[0-9]+|[0-9]+E[0-9]+|[0-9]*\.[0-9]+E[0-9]+)
// bool		: true|false
// char		: \$.|\$\\.
// symbols and keywords can contain any of the following. must not start with number
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

// the matching functions. each will leave the stream pointer to the next character
// after what was matched upon success (and return true)
// or leave the stream pointer the way it was before upon failure (and return false)
// consu,e is the same as match but does not read the matched stuff to a buffer.

/*





bool match_exact(istream * in, const char * sequence, char * out);
bool match_any(istream * in, const char * sequence, char * out);

bool consume_exact(istream * in, const char * sequence);
bool consume_any(istream * in, const char * sequence);

// probe is like consume but it always leaves the stream pointer the way it was before it started.



bool probe_exact(istream * in, const char * sequence);

bool probe_any(istream * in, const char * sequence);
*/

typedef struct rflags {
	int depth;
	bool good : 1;
	bool eol : 1;
	
	const char * msg;
} rflags;

void rflags_reset(rflags * this);

Cons * readl(istream * in, char * buffer, rflags * fl);

size_t match_char(istream * in, char * out, rflags * fl);
size_t match_string(istream * in, char * out, rflags * fl);
size_t match_int(istream * in, char * out, rflags * fl);
size_t match_double(istream * in, char * out, rflags * fl);
size_t match_symbol_or_keyword(istream * in, char * out, rflags * fl);
size_t match_exact(istream * in, const char * sequence, char * out, rflags * fl);

void process_string(char * in, int * pos);

void process_char(char * buffer, int * pos);


// it would be better to use regular expressions... maybe
/*

void next_token(istream * source, char * buffer, rflags * fl);

bool match_string(istream * in, char * out, rflags * fl);

bool match_int(istream * in, char * out, rflags * fl);
bool match_double(istream * in, char * out, rflags * fl;
bool match_bool(istream * in, char * out, rflags * fl);
bool match_char(istream * in, char * out, rflags * fl);
bool match_keyword(istream * in, char * out, rflags * fl);
bool match_symbol(istream * in, char * out, rflags * fl);
bool match_exact(istream * in, const char * sequence, char * out, rflags * fl);

void consume_whitespace(istream * in, char * out , rflags * fl);
*/

#endif
