#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include "types.h"
#include "gc.h"
#include "dictionary.h"

DictNode ** Environment_evOrAdd(Environment * this, Symbol what);
Atom * Environment_set(Environment * this, Symbol key, Atom val);
const Atom * Environment_ev(Environment * this, Symbol key);
Atom * Environment_setp(Environment * this, Symbol key, Atom val);

struct Environment {
	GCObject_HEAD
	
	DictNode * local;
	Environment * parent;
	
	DictNode * imports; // dict of transparent references to the imported things
};

struct Module {
	Environment _Environment_;
	
	Cons * exported_symbols; // list of symbols...
	DictNode * exports;
};

Environment * Environment_new(DictNode * l, Environment * parent);

Atom * Environment_define(Environment * this, Symbol key, Atom val); //same for both
void Environment_addImport(Environment * this, Module * imp);

void Environment_grayChildren(GCObject * this);
void Environment_destroy(GCObject * this);

Module * Module_new(DictNode * l, Environment * parent, Cons * exports);

Atom * Module_get(Module * this, Symbol key);
void Module_export(Module * this, Cons * exp);
void Module_grayChildren(GCObject * this);
void Module_destroy(GCObject * this);

// void (*addImport)(Environment * env);

/*
struct Environment : GCObject {
private:
	Dict local;
	gc_ptr<Environment> parent;
	List<gc_ptr<Environment> > imports;
	Hash<symbol, Atom *> exports;
public:
	static int count;
	
	bool sealed;
	 
	void grayChildren() {
		if(parent != NULL) parent->markGray();
		local.grayThem();
		// exports.grayThem();
		
		ListNode<gc_ptr<Environment> > * el = imports.first_;
		
		while(el != NULL) {
			if(el->content() != NULL) el->content()->markGray();
			
			el = el->next;
		}
	}
	
	Environment(Dict l, gc_ptr<Environment> p) : GCObject() {
		parent = p;
		local = l;
		imports = List<gc_ptr<Environment> >();
		count++;
	}
	
	Environment(gc_ptr<Environment> p) : GCObject() {
		parent = p;
		local = Dict();
		imports = List<gc_ptr<Environment> >();
		count++;
	}
	
	Environment() : GCObject() {
		local = Dict();
		parent = NULL;
		imports = List<gc_ptr<Environment> >();
		count++;
	}
	
	Environment(gc_ptr<Environment> p, ConsList e) : GCObject() {
		parent = p;
		local = Dict();
		imports = List<gc_ptr<Environment> >();
		
		gc_ptr<Cons> el = e.first_;
		
		while(el != NULL) {
			exports.add(el->content().get<symbol>(), (Atom *) NULL);
			el = el->next();
		}
		
		count++;
	}
	
	// Atom & search(symbol key);
	// HashNode<symbol, Atom> * isearch(symbol key);
	//HashNode<symbol, Atom> * searchd(symbol key);
	//Atom& operator[](symbol key);
	
	// internal to environment
	Atom * define(symbol key, Atom val);
	HashNode<symbol, Atom> * & evOrAdd(symbol what);
	Atom * set(symbol key, Atom val);
	// internal and for children
	const Atom * ev(symbol key);
	Atom * setp(symbol key, Atom val);
	// external
	const Atom * get(symbol key);
	void updateExport(symbol key);
	
	// Atom get(symbol key);
	
	void addImport(gc_ptr<Environment> delegate);
};*/
	
Atom AEnvironment(Environment * env);

#endif
