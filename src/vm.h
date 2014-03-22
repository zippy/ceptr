#ifndef _VM_H
#define _VM_H

#include "tree.h"
#include "flow.h"

enum {RUNTREE_NOUN=-199,INSTRUCTION_NOUN=-200,INTEGER_NOUN=-201};
enum {I_RETURN,I_ITER,I_COND,I_COND_PAIR,I_EQ};
enum {TRANSFORM_ERR = -1,TRANSFORM_OK};

/*  VM execution cycle
- load flow at flow ptr from the run tree
- if no flow in run tree, instantiate it from the source tree
- get flow phase
- if flow phase is null, set phase to executing child 1, advance ptr to first child if exists and new cycle,
- load flow function and execute (which updates state)
-
n = _t_get(t,fp);
if (!n) BAD_PATH_FAULT;
ffn = _f_get_flow_fn(n);
f = _f_get_flow(r,fp);
fp = (ffn)(c,t,fp,f);
*/

/*
int t(Tnode *i,Tnode *r)  {
    Tnode *t,*n,*pp;
    int p[20];
    TreeWalker w;
    _t_init_walk(i,&w,WALK_DEPTH_FIRST);
    n = _t_build_one(0,i,r);
    do {
	_t_path_parent(p,w.p);
	Tnode *pp = _t_get(n,p);
	Tnode *c = _t_build_one(pp,i,m);

	if (_t_noun(n) != INSTRUCTION_NOUN) {
	    t = _t_build(i,r);
	    _t_become(r,t);
	    return TRANSFORM_OK;
	}
    } while(_t_walk(i,&w));
}
*/

Tnode *_context_create(int max_depth) {
    Tnode *c = _t_new_root(CONTEXT_TREE_NOUN);
    Tnode *fp = _t_new(c,PATH_NOUN,0,sizeof(int)*(max_depth+1));
    *(int *)fp->surface = TREE_PATH_TERMINATOR;
    return c;
}


Tnode *_vm_cycle_load(Tnode *c,Tnode *s) {
    Tnode *r = __run_tree(c);
    int *fp = __f_cur_flow_path(c);
    Tnode *n = _t_get(r,fp);
    // if current node doesn't exist in the run-tree allocate one
    if (!n) {
	// first find the parent to which to attach the new run tree node
	Tnode *r_parent;
	if (r == NULL) {
	    r_parent = c;
	}
	else {
	    int p[10]; //TODO: dynamic size?
	    _t_path_parent(p,fp);
	    r_parent = _t_get(r,p);
	}
	n = _f_new(r_parent);
    }
    return n;
}

int _vm_cycle_descend(int *fp,Tnode *r,Tnode *s) {
    Flow *f = (Flow *)_t_surface(r);
    if (f->phase == FLOW_PHASE_NULL) {
	int d = _t_path_depth(fp);
	fp[d] = 1;
	fp[d+1] = TREE_PATH_TERMINATOR;
	if (_t_get(s,fp)) {
	    f->phase = 1;
	    return true;
	}
	fp[d] = TREE_PATH_TERMINATOR;
    }
    return false;
}

void _vm_cycle_eval(int *fp,Tnode *r,Tnode *s) {
    Flow *f = (Flow *)_t_surface(r);
    if (_t_noun(s) != FLOW_NOUN) {
	f->phase = FLOW_PHASE_COMPLETE;
	f->surface = _t_surface(s);
	f->noun = _t_noun(s);
	_t_path_parent(fp,fp);
    }
    else {
	char *err = 0;
	switch(*(int *)&s->surface) {
	case F_IF:
	    err = _f_if(fp,f,r);
	    break;
	default:
	    raise_error("Flow %d not implemented\n",*(int *)&s->surface);
	}
	if (err == (char *)-1) {
	    raise_error2("Bad phase %d for instruction %d\n",f->phase,*(int *)&s->surface);
	}
	if (err) {
	    raise_error0(err);
	}
    }
}

int cycle(Tnode *c, Tnode *s) {
    Tnode *rtn = _vm_cycle_load(c,s);
    int *fp = __f_cur_flow_path(c);
    if (((Flow *)_t_surface(rtn))->phase == FLOW_PHASE_COMPLETE && _t_path_depth(fp) == 0)
	return 0;
    if (!_vm_cycle_descend(fp,rtn,s)) {
	Tnode *stn = _t_get(s,fp);
	_vm_cycle_eval(fp,rtn,stn);
    }
    return 1;
}



int transform(Tnode *i,Tnode *r);

int _transform(Tnode *t) {
    int p[3] ={0,0,TREE_PATH_TERMINATOR};
    int i,j;
    Tnode *t1;
    Tnode *t2;
    Tnode *r;
    if (_t_noun(t) == INSTRUCTION_NOUN) {
	switch(*(int *)&t->surface) {
	case I_RETURN:
	    // detach child so it can be used in become;
	    t1 = _t_get_child(t,1);
	    t->child_count = 0;

	    _t_become(_t_parent(t),t1);
	    break;
	case I_ITER:
	    t1 = _t_get_child(t,1);
	    //TODO: semfault if second child isn't a run tree?
	    t2 = _t_get_child(t,2);
	    //TODO: clone the iteration sub run-tree against the current run tree?
	    for(i=1;i<=_t_children(t2);i++) {
		r = _t_get_child(t2,i);
		transform(t1,r);
		if ((_t_noun(r) == INSTRUCTION_NOUN) && (*(int *)_t_surface(r)== I_RETURN))  {
		    _transform(r);
		    _t_become(t,t2);
		    break;
		}
	    }
	    break;
	case I_COND_PAIR:
	    t1 = _t_get_child(t,1);
	    _transform(t1);
	    if (_t_noun(t1) == BOOLEAN_NOUN && *(int *)_t_surface(t1)== TRUE_VALUE) {
		*((int *)_t_surface(t)) = I_RETURN;
		t2 = _t_get_child(t,2);
		_t_become(t1,t2);
		t->child_count = 1;
		return _transform(t1);
	    }
	    break;
	case I_EQ:
	    //TODO: should eq evaluate all the children or just assume two?
	    if (t->child_count != 2) {
		return TRANSFORM_ERR;
	    }
	    t1 = _t_get_child(t,1);
	    t2 = _t_get_child(t,2);
	    (*(int *)&t->surface) = (*(int *)_t_surface(t1)==*(int*)_t_surface(t2)) ? TRUE_VALUE : FALSE_VALUE;
	    t->noun = BOOLEAN_NOUN;
	    _t_free(t1);
	    _t_free(t2);
	    t->child_count = 0; //TODO: remove this if t_free ends up handling it in the future
	    break;
	}
    }
    return TRANSFORM_OK;
}

int transform(Tnode *i,Tnode *r) {
    Tnode *t = _t_build(i,r);
    _transform(t);
    _t_become(r,t);
    return TRANSFORM_OK;
}

#endif
