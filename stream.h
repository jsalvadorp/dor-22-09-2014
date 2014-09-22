#ifndef STREAM_H
#define STREAM_H

#include <stdio.h>

#include "prelude.h"

typedef struct istream {
	struct istream_vtbl_t * vptr;
} istream;

#define istream_HEAD istream _istream_;

struct istream_vtbl_t {
	int (*sgetc)(istream * this);
	void (*sseek)(istream * this, size_t pos);
	void (*sseekr)(istream * this, long pos);
	long (*stell)(istream * this);
	bool (*seof)(istream * this);
	void (*sclose)(istream * this);
};

extern struct istream_vtbl_t ifstream_vtbl;
extern struct istream_vtbl_t isstream_vtbl;

extern struct istream_vtbl_t istream_vtbl;

typedef struct ifstream {
	istream_HEAD
	FILE * fp;
} ifstream;

ifstream ifstream_make(FILE * f);

typedef struct isstream {
	istream_HEAD
	bool last_failed;
	const char * source;
	size_t pointer;
} isstream;

isstream isstream_make(const char * src);

int igetc(istream * this);
void iseek(istream * this, size_t pos);
void iseekr(istream * this, long pos);
long itell(istream * this);
bool ieof(istream * this);
void iclose(istream * this);

#endif
