#ifndef OBJ_H
#define OBJ_H

#include "stdio.h"
#include "vm.h"
#include "strdict.h"

#define RSRC_REC_SIZE 12
#define CP_REC_SIZE 16

#define HEADER_SIZE 32
#define HEADER_EXTRA_SIZE 32

#define RSRCTBL_AREA (HEADER_SIZE + HEADER_EXTRA_SIZE)
#define DATA_AREA(d) (RSRCTBL_AREA + (d->resource_count + d->reserved_resources)*RSRC_REC_SIZE)



struct drzw;
struct drzw_resource;

typedef struct drzw drzw;
typedef struct drzw_resource drzw_resource;

typedef int (*drzw_rsrc_fwriter) (FILE * dest, drzw * d, drzw_resource * r);

struct drzw {
	//char header[HEADER_SIZE];
	//char header_extra[HEADER_EXTRA_SIZE];
	
	int resource_count;
	int max_offset;
	
	int reserved_resources;
	
	drzw_resource * resources;
	drzw_resource ** resources_last; 
};

struct drzw_resource {
	struct drzw_resource * next;
	
	int type;
	int length;
	int offset;
	
	void * data;
	void * owned_data;
	
	drzw_rsrc_fwriter writer;
};

drzw * drzw_new();
int drzw_add_rsrc(drzw * d, drzw_resource * r, bool reserved);
int drzw_write(FILE * dest, drzw * d);
int drzw_write_rsrc(FILE * dest, drzw * d, drzw_resource * r);
int write_rsrc_proc(FILE * dest, drzw * d, drzw_resource * r);
int add_rsrc_proc(drzw * d, proc_t * p, bool reserved);
int add_rsrc_string(drzw * d, const char * p, bool reserved);

int save_object_file(FILE * dest, int entry, constpool * cp, linkinfo * li);



#endif
