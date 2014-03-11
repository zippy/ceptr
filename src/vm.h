#ifndef _VM_H
#define _VM_H

#include "tree.h"

enum {RUNTREE_NOUN=-199,INSTRUCTION_NOUN=-200,INTEGER_NOUN=-201,BOOLEAN_NOUN=-202};
enum {I_RETURN,I_ITER,I_COND,I_COND_PAIR,I_EQ};
enum {TRANSFORM_ERR = -1,TRANSFORM_OK};
enum {FALSE_VALUE = 0,TRUE_VALUE = 1};

int t(Tnode *i,Tnode *r)  {
    Tnode *t;
    if (_t_noun(i) != INSTRUCTION_NOUN) {
	t = _t_build(0,i,r);
	_t_become(r,t);
	return TRANSFORM_OK;
    }
    TreeWalker w;
    _t_init_walk(i,&w,WALK_DEPTH_FIRST);
    while(t = _t_walk(t,&w)) {

    }
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
    Tnode *t = _t_buildx(0,i,r);
    printf("OLD:\n");
    _t_dump(t);
    t = _t_build(0,i,r);
    printf("NEW:\n");    _t_dump(t);
    //  if (_t_noun(i) == INSTRUCTION_NOUN) {
    _transform(t);
    // }
    _t_become(r,t);
    return TRANSFORM_OK;
}

#endif
