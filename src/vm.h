#ifndef _VM_H
#define _VM_H

#include "tree.h"

enum {INSTRUCTION_NOUN=-200,INTEGER_NOUN=-201,BOOLEAN_NOUN=-202};
enum {I_COND,I_COND_PAIR,I_EQ};
enum {REDUCE_ERR = -1,REDUCE_OK};
enum {FALSE_VALUE = 0,TRUE_VALUE = 1};

int reduce(Tnode *t) {
    int i,j;
    Tnode *t1;
    Tnode *t2;
    if (_t_noun(t) != INSTRUCTION_NOUN) return REDUCE_ERR;
    switch(t->surface) {
    case I_COND:
	for(i=1;i<=_t_children(t);i++) { //change to iterator when we have a break early one?
	    t1 = _t_get_child(t,i);
	    if (reduce(t1) == REDUCE_OK) {
		for(j=1;j<=_t_children(t);j++) {
		    if (j!=i) _t_free(_t_get_child(t,j));
		}
		_t_become(t,t1);
		return REDUCE_OK;
	    }
	}
	return REDUCE_ERR;
	break;
    case I_COND_PAIR:
	t1 = _t_get_child(t,1);
	if (reduce(t1) == REDUCE_OK) {
	    if (_t_noun(t1) == BOOLEAN_NOUN && *(int *)_t_surface(t1)== TRUE_VALUE) {
		_t_free(t1);
		t2 = _t_get_child(t,2);
		_t_become(t,t2);
		return REDUCE_OK;
	    }
	}
	return REDUCE_ERR;
	break;
    case I_EQ:
	//TODO: should eq evaluate all the children or just assume two?
	if (t->child_count != 2) {
	    return REDUCE_ERR;
	}
	t1 = _t_get_child(t,1);
	t2 = _t_get_child(t,2);
	t->surface = (*(int *)_t_surface(t1)==*(int *)_t_surface(t2)) ? TRUE_VALUE : FALSE_VALUE;
	t->noun = BOOLEAN_NOUN;
	_t_free(t1);
	_t_free(t2);
	t->child_count = 0; //TODO: remove this if t_free ends up handling it in the future
	break;
    default:
	return REDUCE_ERR;
    }
    return REDUCE_OK;
}

#endif
