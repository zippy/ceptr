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
    Tnode *d = __d_get_def(f->noun);
    char *label = (char *)_t_get_child_surface(d,DEF_LABEL_CHILD);
    sprintf(dump_buf,"phase: %d noun: %s",f->phase,label);
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

int __copy_meta(Tnode *ft,Tnode *dt) {
    int c = _t_children(ft);
    for(int i=1;i<=c;i++) {
	Tnode *fn = _t_get_child(ft,i);
	Flow *f = (Flow *)_t_surface(fn);
	int n = _f_noun(f);
	if (n != META_NOUN) return 0;
	Tnode *d = _t_newi(dt,META_NOUN,*(int *)&f->surface);
	if (_t_children(fn) > 0) if (!__copy_meta(fn,d)) return 0;
    }
    return 1;
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
	Tnode *mt = _t_get_child(r,2);
	Flow *m = mt ? (Flow *)_t_surface(mt) : 0;
	if (!m || _f_noun(m) != NOUNTREE_NOUN) {
	    return "DEF requires a NOUNTREE as 2nd child";
	}
	Flow *l = (Flow *)_t_get_child_surface(r,1);
	char *label = (char *)_f_surface(l);
	int def_id = next_id();
	Tnode *d = _d_sys_def(label,def_id,0,0);
	if (!__copy_meta(mt,d)) return "bad meta tree";
	//	_d_dump(mt);
	//_d_dump(d);
	f->noun = def_id;
	f->phase = FLOW_PHASE_COMPLETE;
	return 0;
    }
    return (char *)-1;
}

int meta_eq_mapfn(Tnode *t1,Tnode *t2) {
    if ((_t_noun(t1) != META_NOUN) || (_t_noun(t2) != META_NOUN) ||
	(*(int *)_t_surface(t1) != *(int *)_t_surface(t2))) return 0;
    return 1;
}



#endif
