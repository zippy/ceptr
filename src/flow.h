#ifndef _FLOW_H
#define _FLOW_H

#include "tree.h"
#include "def.h"

enum {C_FLOW_PATH = 1,C_RUNTREE};

enum {FLOW_PHASE_NULL,FLOW_PHASE_COMPLETE = -1};

typedef struct {
    int phase;
    Symbol noun;
    void *surface;
} Flow;

char *_dump_flow_state(void *surface) {

    Flow *f = (Flow *)surface;
    sprintf(dump_buf,"phase: %d noun: %d",f->phase,f->noun);
    return dump_buf;
}


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
	if (!b || _f_noun(b) != BOOLEAN_NOUN) {
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

int next_id() {
    G_sys_noun_id--;
}

char *_f_def(int *fp,Flow *f,Tnode *r) {
    if (f->phase == 1) {
	Flow *l = (Flow *)_t_get_child_surface(r,1);
	if (!l || _f_noun(l) != CSTRING_NOUN) {
	    return "DEF requires a CSTRING as 1st child";
	}
	f->phase = 2;
	int d = _t_path_depth(fp);
	fp[d-1] = f->phase;
	return 0;
    }
    else if (f->phase == 2) {
	Flow *m = (Flow *)_t_get_child_surface(r,2);
	if (!m || _f_noun(m) != META_NOUN) {
	    return "DEF requires a META tree as 2nd child";
	}
	Flow *l = (Flow *)_t_get_child_surface(r,1);
	char *label = (char *)_f_surface(l);
	int def_id = next_id();
	_d_sys_def(label,def_id,0,0);
	f->noun = def_id;
	f->phase = FLOW_PHASE_COMPLETE;
	return 0;
    }
    return (char *)-1;
}




#endif
