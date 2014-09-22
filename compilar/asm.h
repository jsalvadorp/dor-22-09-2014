#ifndef ASM_H
#define ASM_H


#include "strdict.h"

int consume_whitespace(FILE * in);

int scantok(FILE * src, char * buf, int * count);

/*
int assemble_cp(FILE * src, constpool * c, strdict ** symbols);


// TODO: symbol table (global), label dictionary(local), asm constants
int assemble_f(FILE * src, proc_t * f, strdict * symbols, proc_t ** entry);*/


int assemble(FILE * src, constpool * c, linkinfo * li);

#endif
