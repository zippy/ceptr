#ifndef _FLOW_H
#define _FLOW_H

#include "tree.h"

enum Symbols {
    CSPEC = -1, CSPEC_NOUN = -2, XADDR_NOUN = -3,
    CSTRING_NOUN = -4, PATTERN_SPEC_DATA_NOUN = -5,
    NOUN_NOUN = -6
};

enum {RUNTREE_NOUN=-199,INSTRUCTION_NOUN=-200,INTEGER_NOUN=-201};
enum {FLOW_NOUN = -31410,PATH_NOUN,CONTEXT_TREE_NOUN,BOOLEAN_NOUN,FLOW_STATE_NOUN};
enum {C_FLOW_PATH = 1,C_RUNTREE};
enum {F_IF};
enum {FLOW_PHASE_NULL,FLOW_PHASE_COMPLETE = -1};

enum {FALSE_VALUE = 0,TRUE_VALUE = 1};

typedef struct {
    int phase;
    Symbol noun;
    void *surface;
} Flow;

Tnode *_f_new(Tnode *parent) {
    Flow f = {FLOW_PHASE_NULL};
    return _t_new(parent,FLOW_STATE_NOUN,&f,sizeof(Flow));
}

Symbol _f_noun(Flow *f) {
    return f->noun;
}

void * _f_surface(Flow *f) {
    return f->surface;
}

int *__f_cur_flow_path(Tnode *c) {
    return (int *)_t_surface(_t_get_child(c,C_FLOW_PATH));
}

Tnode *__f_cur_flow(Tnode *c,Tnode *t) {
    return _t_get(t,__f_cur_flow_path(c));
}

Tnode *__run_tree(Tnode *c) {
    return _t_get_child(c,C_RUNTREE);
}

char *_f_if(int *fp,Flow *f,Tnode *r) {
    if (f->phase == 1) {
	Flow *b = (Flow *)_t_get_child_surface(r,1);
	if (_f_noun(b) != BOOLEAN_NOUN) {
	    return "IF requires Boolean Noun";
	}
	int val = *(int *)_f_surface(b);
	if (val == TRUE_VALUE) {
	    f->phase = 2;
	}
	else {
	    Tnode *dummy = _f_new(r); // make a dummy node for the true value
	    f->phase = 3;
	}
	int d = _t_path_depth(fp);
	fp[d] = f->phase;
	fp[d+1] = TREE_PATH_TERMINATOR;
    }
    else if (f->phase == 2 || f->phase == 3) {
	Flow *c = _t_get_child_surface(r,f->phase);
	f->surface = _f_surface(c);
	f->noun = _f_noun(c);
	f->phase = FLOW_PHASE_COMPLETE;
	__t_free_children(r);
    }
    else return (char *)-1;
    return 0;
}

char *_f_def(int *fp,Flow *f,Tnode *r) {
    return (char *)-1;
}

#endif
