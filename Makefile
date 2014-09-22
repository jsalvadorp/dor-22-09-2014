CC=gcc
CFLAGS=-c -std=c99
DFLAGS=-g

comp:
	$(CC) -std=c99 $(LFLAGS) $(DFLAGS) typing.c a3.c read3.c ast.c symbol.c stream.c dorc.c -o dorc
	
clean:
	rm -rf *.o dorc
