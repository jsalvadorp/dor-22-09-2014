#include <stdlib.h>
#include <assert.h>
#include "typing.h"

#define min(a, b) ((a) < (b) ? (a) : (b))

// memory is never freed. sorry. 

// TOO UGLY!! TOO MANY ** STARS ** 
// LESS INDIRECTION EY

type_t *type_int, *type_bool, *type_void, *type_func;
kind_t _StarKind, _VoidKind;

kind_t *starseq(int order) { // memoize
	if(order > 0) {
		kind_t *newk = malloc(sizeof(kind_t));
		newk->from = StarKind;
		newk->to = starseq(order - 1);
		return newk;
	} else {
		return StarKind;
	}
}

void type_init() {
	type_int = malloc(sizeof(type_t));
	type_int->tag = TYPE_TYPE;
	type_int->kind = StarKind;
	type_int->compactsize = 1;
	
	type_bool = malloc(sizeof(type_t));
	type_int->tag = TYPE_TYPE;
	type_int->kind = StarKind;
	type_int->compactsize = 1;
	
	type_void = malloc(sizeof(type_t));
	type_int->tag = TYPE_TYPE;
	type_int->kind = VoidKind;
	type_int->compactsize = 0;
	
	type_func = malloc(sizeof(type_t));
	type_int->tag = TYPE_TYPE;
	type_int->kind = starseq(2);
	type_int->compactsize = -1;
	
}

// https://en.wikipedia.org/wiki/Disjoint-set_data_structure
tvar_t * tvar_new() {
	tvar_t * newt = malloc(sizeof(tvar_t));
	
	newt->dsf_parent = newt;
	newt->dsf_rank = 0;
	
	newt->t.tag = TYPE_TVAR;
	newt->t.kind = NULL;
}

tvar_t * tvar_getrep(tvar_t * v) {
	if(v->dsf_parent != v)
		v->dsf_parent = tvar_getrep(v->dsf_parent);
	return v->dsf_parent;
}

#define TYFUNC(x) ((x) != VoidKind && (x) != StarKind)

bool makesame(void **p1, void **p2) { // not null!!!s
	*p1 = *p2 = min(*p1, *p2);
	return *p1 != NULL;
}

bool kind_unite(kind_t **lhs, kind_t **rhs) {
	if(!*lhs) {
		*lhs = *rhs;
		
		return true;
	} else if(!*rhs) {
		*rhs = *lhs;
		
		return true;
	} else {
		return (*lhs == *rhs) 
			|| (TYFUNC(*lhs) 
				&& TYFUNC(*rhs) 
				&& kind_unite(&(*lhs)->from, &(*rhs)->from)
				&& kind_unite(&(*lhs)->to, &(*rhs)->to)
				&& makesame((void **)lhs, (void **)rhs)); 
					// last step executed only if kinds are
					// found to match: make them equal so comparing them
					// is cheaper in the future; by keeping only the lower one
	}
}

bool tvar_unite(tvar_t * a, tvar_t * b) {
	tvar_t * arep = tvar_getrep(a), * brep = tvar_getrep(b);
	
	if(arep == brep) return true;
	else {
		// check if constraints and substitutions are equal
		// if they arent, return false!!
		
		/*if(arep->t.kind && brep->t.kind && arep->t.kind != brep->t.kind)
			return false; // differing kinds
		arep->t.kind = arep->t.kind ? arep->t.kind : brep->t.kind;
		brep->t.kind = arep->t.kind;*/
		if(!kind_unite(&arep->t.kind, &brep->t.kind)) // MATCH KINDS!!
			return false;
		
		if(arep->dsf_rank < brep->dsf_rank) {
			arep->dsf_parent = brep;
		} else if(arep->dsf_rank > brep->dsf_rank) {
			brep->dsf_parent = arep;
		} else {
			brep->dsf_parent = arep;
			arep->dsf_rank++;
		}
		
		return true;
	}
}

bool occurs(tvar_t * a, type_t * b) {
	return 
		(b->tag == TYPE_TVAR 
			&& tvar_getrep(a) == tvar_getrep((tvar_t*)b)) || 
		(b->tag == TYPE_TAPP
			&& (occurs(a, ((tapp_t *) b)->left)
				||occurs(a, ((tapp_t *) b)->right)));
}

// how to intantiate polymorphic types??

// propagate constraints!!
bool type_unite(type_t **a, type_t **b) {
	type_t **tmp;
	
	if(!*a) {
		*a = *b;
		
		return true;
	} else if(!*b) {
		*b = *a;
		
		return true;
	}
	
	
	type_t *lhs = *a, *rhs = *b;
	
	if((*a)->tag == TYPE_TVAR && ((*a) = tvar_getrep((tvar_t *) (*a))->subst)) {
		(*a) = ((tvar_t *)(*a))->subst;
	} if((*b)->tag == TYPE_TVAR && ((*b) = tvar_getrep((tvar_t *) (*b))->subst)) {
		(*b) = ((tvar_t *)(*b))->subst;
	}	
	
	if((*b)->tag > (*a)->tag) { // tvars first
		tmp = a;
		a = b;
		b = tmp;
	}
	
	if((*a)->tag == TYPE_TVAR) {
		if((*b)->tag == TYPE_TVAR) {
			return tvar_unite((tvar_t *) (*a), (tvar_t *) (*b)) 
				&& makesame((void **)a, (void **)b);
		} else {
			if(kind_unite(&(*a)->kind, &(*b)->kind)) { // kinds match
				((tvar_t *)(*a))->subst = (*b);
			} else return false;
		}
	} else if((*a)->tag == TYPE_TYPE && (*b)->tag == TYPE_TYPE) {
		return (*a) == (*b); // ?
	} else if((*a)->tag == TYPE_TAPP && (*b)->tag == TYPE_TAPP) {
		return kind_unite(&(*a)->kind, &(*b)->kind)
			&& type_unite(&((tapp_t *)(*a))->left, &((tapp_t *)(*b))->left)
			&& type_unite(&((tapp_t *)(*a))->right, &((tapp_t *)(*b))->right);
	} else return false;
}

typedef struct {
	type_t **from, **to;
} functinfo;

bool isfunctype(type_t *ft, functinfo *fi) {
	if (ft->tag == TYPE_TAPP) {
		if(((tapp_t *) ft)->left && ((tapp_t *) ft)->left->tag == TYPE_TAPP) {
			if(((tapp_t *)((tapp_t *) ft)->left)->left == type_func) {
				fi->from = &((tapp_t *)((tapp_t *) ft)->left)->right;
				fi->to = &((tapp_t *) ft)->right;
				return true;
			}
		}
	}
	
	return false;
}

type_t *functype_make(type_t *from, type_t *to) {
	tapp_t *newt = malloc(sizeof(tapp_t));
	newt->t.kind = StarKind;
	newt->t.tag == TYPE_TAPP;
	newt->t.compactsize = 1;
	newt->right = to;
	newt->left = malloc(sizeof(tapp_t));
	newt->left->kind = StarKind;
	newt->left->tag == TYPE_TAPP;
	newt->left->compactsize = 1;
	((tapp_t *)newt->left)->left = type_func;
	((tapp_t *)newt->left)->right = from;
	
	return (type_t *) newt;
}

bool type_unite_app(type_t **fun, type_t **arg, type_t **ret) {
	functinfo fi;
	
	if(*fun && isfunctype(*fun, &fi)) {
		return type_unite(fi.from, arg) && type_unite(fi.to, ret);
	} else { // TVAR!!!!!!!!!!!!
		type_t *newt = (type_t *)functype_make(*arg, *ret);
		return type_unite(&newt, fun);
	}
	
	return false;
}



// a : t1 -> t2
// b : t1
// a b : t2

