#include "stream.h"

/*struct istream_vtbl_t;

typedef struct istream {
	struct istream_vtbl_t * vptr;
} istream;

#define istream_HEAD istream _istream_;

struct istream_vtbl_t {
	int (*sgetc)(istream * this);
	void (*sseek)(istream * this, size_t pos);
	bool (*seof)(istream * this);
};



typedef struct ifstream {
	istream_HEAD
	FILE * fp;
} ifstream;

ifstream ifstream_make(FILE * f);

typedef struct isstream {
	istream_HEAD
	//size_t size;
	char * source;
} isstream;

isstream isstream_make(char * src);*/



int ifstream_getc(istream * this) {
	return fgetc(((ifstream *)this)->fp);
}

void ifstream_seek(istream * this, size_t pos) {
	fseek(((ifstream *)this)->fp, pos, SEEK_SET);
}

void ifstream_seekr(istream * this, long pos) {
	fseek(((ifstream *)this)->fp, pos, SEEK_CUR);
}

long ifstream_tell(istream * this) {
	return ftell(((ifstream *)this)->fp);
}

bool ifstream_eof(istream * this) {
	return feof(((ifstream *)this)->fp);
}

void ifstream_close(istream * this) {
	fclose(((ifstream *)this)->fp);
}

struct istream_vtbl_t ifstream_vtbl = 
	{ifstream_getc, 
	 ifstream_seek, 
	 ifstream_seekr,
	 ifstream_tell, 
	 ifstream_eof, 
	 ifstream_close};
	 
ifstream ifstream_make(FILE * f) {
	ifstream niss = {{&ifstream_vtbl}, f};
	return niss;
}

int isstream_getc(istream * this) {
	isstream * iss = (isstream *)this;
	
	if(iss->source[iss->pointer] != '\0') {
		
		return iss->source[iss->pointer++];
	} else iss->last_failed = true;
	
	return EOF;
}

void isstream_seek(istream * this, size_t pos) {
	((isstream *)this)->pointer = pos;
}

void isstream_seekr(istream * this, long pos) {
	((isstream *)this)->pointer += pos;
}

long isstream_tell(istream * this) {
	return ((isstream *)this)->pointer;
}

bool isstream_eof(istream * this) {
	isstream * iss = (isstream *)this;
	return iss->last_failed;
}

void isstream_close(istream * this) {
	
}

struct istream_vtbl_t isstream_vtbl = 
	{isstream_getc, 
	 isstream_seek, 
	 isstream_seekr, 
	 isstream_tell, 
	 isstream_eof, 
	 isstream_close};

isstream isstream_make(const char * src) {
	isstream niss = {{&isstream_vtbl}, false, src, 0L};
	return niss;
}

int igetc(istream * this) {
	return this->vptr->sgetc(this);
}

void iseek(istream * this, size_t pos) {
	this->vptr->sseek(this, pos);
}

void iseekr(istream * this, long pos) {
	this->vptr->sseekr(this, pos);
}

long itell(istream * this) {
	return this->vptr->stell(this);
}

bool ieof(istream * this) {
	return this->vptr->seof(this);
}

void iclose(istream * this) {
	this->vptr->sclose(this);
}
