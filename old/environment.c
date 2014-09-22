#include "environment.h"
#include "cons.h"

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

Atom * Environment_define(Environment * this, Symbol key, Atom value) {
	DictNode ** dn = DictNode_searchToAdd(&(this->local), key);
	if(*dn) (*dn)->value = value;
	else *dn = DictNode_new(key, value);
}

void Environment_addImport(Environment * this, Module * imp) {
	foreach(export, imp->exported_symbols) {
		assert(export->car.type == SymbolType);
		
		DictNode ** a = Environment_evOrAdd(&(imp->_Environment_), export->car.d._Symbol);
		
		if(!a) *a = DictNode_new(export->car.d._Symbol, nil);
		//exported symbol must be already bound to some value. otherwise error.
		//assert(a);
		
		Atom ref;
		ref.d._Atom_transparent = &((*a)->value);
		
		DictNode_add(&(this->imports), export->car.d._Symbol, ref);
	}
}

void Environment_grayChildren(GCObject * this) {
	Environment * env = (Environment *) this;
	
	if(env->parent) GCObject_markGray((GCObject *) env->parent);
	DictNode_map(env->local, DictNode_markGray);
}

void Environment_destroy(GCObject * this) {
	Environment * env = (Environment *) this;
	
	if(env->local) {
		DictNode_destroy(env->local);
		free(env->local);
	}
	
	if(env->imports) {
		DictNode_destroy(env->imports);
		free(env->imports);
	}
}

DictNode ** Environment_evOrAdd(Environment * this, Symbol what) {
	return DictNode_searchToAdd(&(this->local), what);
	// should also search parents and imports?
}

Atom * Environment_set(Environment * this, Symbol key, Atom val) {
	Atom * a = DictNode_search(this->local, key);
	if(!a) a = DictNode_search(this->imports, key);
	assert(a);
	*a = val;
	return a;
}

const Atom * Environment_ev(Environment * this, Symbol key) {
	// search locally
	const Atom * dn = DictNode_search(this->local, key);
	if(dn) return dn;
	
	// search imports
	dn = DictNode_search(this->imports, key);
	if(dn) return dn->d._Atom_transparent;
	
	// search parent
	if(this->parent && (dn = Environment_ev(this->parent, key))) return dn;
	
	// else 
	
	// throw UndefinedSymbol("Tried to evaluate undefined symbol.");
	return NULL;
}

Atom * Environment_setp(Environment * this, Symbol key, Atom val) {
	Atom * a;
	if(a = DictNode_search(this->local, key));
	else if(a = DictNode_search(this->imports, key));
	else if(this->parent) return Environment_setp(this->parent, key, val);
	
	assert(a);
	*a = val;
	return a;
}

struct GCObject_vtbl_t Environment_vtbl = {
	Environment_grayChildren, Environment_destroy
};

Environment * Environment_new(DictNode * local, Environment * parent) {
	Environment * ne = malloc(sizeof(Environment));
	//printf("%p\n", ne);
	ne->_GCObject_.vptr = (struct GCObject_vtbl_t *) &Environment_vtbl;
	GCObject_INIT(ne)
	
	ne->local = local;
	ne->parent = parent;
	ne->imports = NULL;
	
	return ne;
}

Atom * Module_get(Module * this, Symbol key) {
	//return this->exports
	
	if(this->exports) { // if the module has explicitly exported symbols
		Atom * dn = DictNode_search(this->exports, key);
		if(dn) return dn->d._Atom_transparent;
		else return NULL;
	} else {
		Atom * dn = DictNode_search(this->_Environment_.local, key);
		if(dn) return dn;
		else return NULL;
	}
}

void Module_export(Module * this, Cons * exp) {
	Cons_append(&(this->exported_symbols), exp);
	
	foreach(export, exp) {
		assert(export->car.type == SymbolType);
		
		DictNode ** a = Environment_evOrAdd(&(this->_Environment_), export->car.d._Symbol);
		
		if(!a) *a = DictNode_new(export->car.d._Symbol, nil);
		//exported symbol must be already bound to some value. otherwise error.
		//assert(a);
		
		Atom ref;
		ref.d._Atom_transparent = &((*a)->value);
		
		DictNode_add(&(this->exports), export->car.d._Symbol, ref);
	}
}

void Module_grayChildren(GCObject * this) {
	Module * env = (Module *) this;
	//printf("pointers %p %p %p", this, env,  &(env->_Environment_));
	
	Environment_grayChildren((GCObject *) &(env->_Environment_));
	if(env->exported_symbols)  GCObject_markGray((GCObject *) env->exported_symbols);
}

void Module_destroy(GCObject * this) {
	Module * env = (Module *) this;
	
	Environment_destroy((GCObject *) &(env->_Environment_));
	
	if(env->exports) {
		DictNode_destroy(env->exports);
		free(env->exports);
	}
}

struct GCObject_vtbl_t Module_vtbl = {
	Module_grayChildren, Module_destroy
};

Module * Module_new(DictNode * local, Environment * parent, Cons * exported_symbols) {
	
	Module * ne = malloc(sizeof(Module));
	//printf("%p\n", ne);
	Environment * env = (Environment *) ne;
	env->_GCObject_.vptr = (struct GCObject_vtbl_t *) &Module_vtbl;
	GCObject_INIT(env)
	
	ne->_Environment_.local = local;
	ne->_Environment_.parent = parent;
	
	ne->exported_symbols = NULL;
	ne->_Environment_.imports = NULL;
	ne->exports = NULL;
	
	Module_export(ne, exported_symbols);
	
	return ne;
}

Atom AEnvironment(Environment * env) {
	Atom ae;
	ae.d._Environment = env;
	ae.type = EnvironmentType;
	
	return ae;
}
