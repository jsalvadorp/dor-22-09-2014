#ifndef GC_H
#define GC_H

#include "forward.h"
#include "stdlib.h"
#include <string.h>

typedef enum GC_color {
	GC_white, GC_gray, GC_black, GC_noncollectable//, free
} GC_color;

struct GCObject_vtbl_t;

struct GCObject {
	// color needs only 2 bits. encode in a pointer? (tagged pointer)?
	// would save a lot of memory. malloc always returns 8-byte aligned addresses after all
	struct GCObject_vtbl_t * vptr;
	GCObject * prev;
	GCObject * next;
	GC_color color;
};

#define GCObject_HEAD GCObject _GCObject_;
#define GCObject_INIT(o) GCObject_init(&(o->_GCObject_));

void GCObject_init(GCObject * this);
void GCObject_markGray(GCObject * this);
void GCObject_markBlack(GCObject * this);

struct GCObject_vtbl_t {
	void (*grayChildren)(GCObject * this);
	void (*destroy)(GCObject * this);
};

struct GCRoot {
	void (*grayChildren)(GCRoot * this);
	void (*destroy)(GCRoot * this);
};

void delete(GCObject * this);

void GC_setup(GCRoot * r);
void GC_mark();
void GC_sweep();
void GC_shutdown();
void GC_remove(GCObject * node);
void GC_consTo(GCObject * what, GCObject ** dest);

extern GCRoot * root;

extern GCObject * whites;
extern GCObject * grays;
extern GCObject * blacks;

/*
class GCObject {
public:
	GCObject * next;
	GCObject * prev;
	color c;
//public:
	GCObject() 
		: c(white), next(NULL), prev(NULL) {
		
		consTo(this, whites);
	}
	
	void addRef() {ref_c++;}
	void remRef() { 
		if (--ref_c == 0) delete this; //this adds the requirement for a virtual destructor.
		//ref_c--;
	}
	
	void markGray() {
		if(c == white) {
			c = gray;
			
			consTo(this, grays);
		}
	}
	
	void markBlack() {
		if(c == gray) {
			c = black;
			
			consTo(this, blacks);
			
			grayChildren();
		}
	}
	
	virtual void grayChildren() = 0;
	
	virtual ~GCObject() = 0;
	
	template<typename T> friend class gc_ptr;
};

class GCObject;

struct RootBase {
	virtual void grayChildren() = 0;
};

void setup(RootBase * r);
void mark();
void sweep();
void shutdown();
void remove(GCObject * node);
void consTo(GCObject * what, GCObject *& dest);

extern RootBase * root;

extern GCObject * whites;
extern GCObject * grays;
extern GCObject * blacks;

//extern * GCObject * free;



*/


#endif
