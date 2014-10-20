#include "../src/ceptr.h"

void testVMTransformData() {
    T *i = _t_newi(0,BOOLEAN_NOUN,TRUE_VALUE);
    _t_newi(i,PTR_NOUN,1);
    T *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);

    spec_is_equal(transform(i,r),TRANSFORM_OK);
    spec_is_equal(_t_noun(r),BOOLEAN_NOUN);
    spec_is_equal(*(int *)_t_surface(r),TRUE_VALUE);

    T *t = _t_get_child(r,1);
    spec_is_equal(_t_noun(t),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(t),42);
    _t_free(i);
    _t_free(r);
}

T * _testEQ(void *parent) {
    T *eq = _t_newi(parent,INSTRUCTION_NOUN,I_EQ);
    _t_newi(eq,PTR_NOUN,1);
    _t_newi(eq,PTR_NOUN,2);
    return eq;
}

void testVMTransformEQ() {
    T *i = _testEQ(0);
    T *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,42);

    spec_is_equal(transform(i,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),0);
    spec_is_equal(*(int *)_t_surface(r),TRUE_VALUE);
    spec_is_equal(_t_noun(r),BOOLEAN_NOUN);
    _t_free(r);

    r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,43);
    _t_newi(r,INTEGER_NOUN,42);
    spec_is_equal(transform(i,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),0);
    spec_is_equal(*(int *)_t_surface(r),FALSE_VALUE);
    spec_is_equal(_t_noun(r),BOOLEAN_NOUN);
    _t_free(r);
}

void testVMTransformReturn() {
    T *c = _t_newi(0,INSTRUCTION_NOUN,I_COND);
    T *r = _t_newi(c,INSTRUCTION_NOUN,I_RETURN);
    T *i = _t_newi(r,INTEGER_NOUN,314);
    spec_is_equal(_transform(r),TRANSFORM_OK);
    spec_is_equal(_t_children(c),0);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(c),314);
}

T *_testCP(T *parent) {
    T *cp = _t_newi(parent,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp);
    _t_newi(cp,PTR_NOUN,3);
    return cp;
}

void testVMTransformCondPair() {
    T *t = _testCP(0);
    T *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,43);
    _t_newi(r,INTEGER_NOUN,99);

    spec_is_equal(transform(t,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),2);
    spec_is_equal(_t_noun(r),INSTRUCTION_NOUN);
    spec_is_equal(*(int *)_t_surface(r),I_COND_PAIR);
    _t_free(r);

    r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,99);

    spec_is_equal(transform(t,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),1);
    spec_is_equal(_t_noun(r),INSTRUCTION_NOUN);
    spec_is_equal(*(int *)_t_surface(r),I_RETURN);

    T *cr = _t_get_child(r,1);
    spec_is_equal(_t_children(cr),0);
    spec_is_equal(*(int *)_t_surface(cr),99);
    spec_is_equal(_t_noun(cr),INTEGER_NOUN);
    _t_free(r);
    _t_free(t);
}

void testVMTransformIter() {
    T *i = _t_newi(0,INSTRUCTION_NOUN,I_ITER);
    _testCP(i);
    T *r = _t_newi(i,RUNTREE_NOUN,0);

    T *ir = _t_newi(r,RUNTREE_NOUN,0);
    _t_newi(ir,INTEGER_NOUN,42);
    _t_newi(ir,INTEGER_NOUN,43);
    _t_newi(ir,INTEGER_NOUN,100);

    ir = _t_newi(r,RUNTREE_NOUN,0);
    _t_newi(ir,INTEGER_NOUN,42);
    _t_newi(ir,INTEGER_NOUN,43);
    _t_newi(ir,INTEGER_NOUN,101);

    ir = _t_newi(r,RUNTREE_NOUN,0);
    _t_newi(ir,INTEGER_NOUN,42);
    _t_newi(ir,INTEGER_NOUN,42);
    _t_newi(ir,INTEGER_NOUN,102);

    spec_is_equal(_transform(i),TRANSFORM_OK);
    spec_is_equal(_t_children(i),0);
    spec_is_equal(*(int *)_t_surface(i),102);
    spec_is_equal(_t_noun(i),INTEGER_NOUN);
    _t_free(i);
}


/*

void testVMTransformRecursive() {
    T *cp = _t_newi(0,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,2,2);
    T *cp1 = _t_newi(cp,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp1,2,2);
    T *i = _t_newi(cp1,INTEGER_NOUN,99);

    spec_is_equal(transform(cp),TRANSFORM_OK);
    spec_is_equal(*(int *)_t_surface(cp),99);
    spec_is_equal(_t_noun(cp),INTEGER_NOUN);
    _t_free(cp);

    T *c = _testC(0);

    cp = _t_newi(c,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,4,4);
    T *c2 = _testC(cp);
    _testCP(c2,3,3,33);

    spec_is_equal(transform(c),TRANSFORM_OK);
    spec_is_equal(*(int *)_t_surface(c),33);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    _t_free(c);

    }*/

void testVMCreateContext() {
    T *c = _context_create(2);
    spec_is_equal(_t_noun(c),CONTEXT_TREE_NOUN);
    T *fp = _t_get_child(c,C_FLOW_PATH);
    spec_is_equal(_t_noun(fp),PATH_NOUN);
    spec_is_long_equal(fp->size,sizeof(int)*3);
    int *p = (int *)_t_surface(fp);
    spec_is_ptr_equal(p,__f_cur_flow_path(c));
    spec_is_equal(p[0],TREE_PATH_TERMINATOR);
    T *rt = _t_get_child(c,C_RUNTREE);
    spec_is_ptr_equal(rt,NULL);
    _t_free(c);
}

void testVMCycleLoad() {
    T *c = _context_create(2);
    T *t = _t_newi(0,FLOW_NOUN,F_IF);
    T *t1 = _t_newi(t,BOOLEAN_NOUN,FALSE_VALUE);
    spec_is_ptr_equal(__run_tree(c),NULL);

    // on first call allocates top node in run tree
    T *rtn = _vm_cycle_load(c,t);
    T *rt = __run_tree(c);
    int *fp = __f_cur_flow_path(c);
    // fp points to correct place on source and run trees and
    // run-tree has a node in initial state
    spec_is_ptr_equal(_t_get(rt,fp),rtn);
    T *s = _t_get(t,fp);

    spec_is_ptr_equal(s,t);  // should be pointing to root
    spec_is_equal(_t_noun(rtn),FLOW_STATE_NOUN);
    Flow *f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,FLOW_PHASE_NULL);

    // works the same on second call (i.e. without allocating run node)
    T *rtn2 = _vm_cycle_load(c,t);
    spec_is_ptr_equal(rtn2,rtn);

    // after descend creates a child
    _vm_cycle_descend(fp,rtn,t);
    rtn2 = _vm_cycle_load(c,t);
    spec_is_ptr_equal(_t_get(rt,fp),rtn2);
    spec_is_true(_t_parent(rtn2)==rtn);
    _t_free(t);_t_free(c);
}

void testVMCycleDescend() {
    T *c = _context_create(2);
    T *t = _t_newi(0,FLOW_NOUN,F_IF);
    T *t1 = _t_newi(t,BOOLEAN_NOUN,FALSE_VALUE);

    T *rtn = _vm_cycle_load(c,t);
    int *fp = __f_cur_flow_path(c);
    spec_is_equal(fp[0],TREE_PATH_TERMINATOR);
    spec_is_true(_vm_cycle_descend(fp,rtn,t));
    spec_is_equal(fp[0],1);spec_is_equal(fp[1],TREE_PATH_TERMINATOR);
    Flow *f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,1);
    spec_is_false(_vm_cycle_descend(fp,rtn,t));
    _t_free(t);_t_free(c);
}

void testVMCycleEval() {
    T *c = _context_create(2);
    T *t = _t_newi(0,FLOW_NOUN,F_IF);
    T *t1 = _t_newi(t,BOOLEAN_NOUN,FALSE_VALUE);
    T *t2 = _t_newi(t,INTEGER_NOUN,1);
    T *t3 = _t_newi(t,INTEGER_NOUN,2);

    // simulate cycling up to the point of first eval
    T *rtn = _vm_cycle_load(c,t);
    int *fp = __f_cur_flow_path(c);
    _vm_cycle_descend(fp,rtn,t);
    rtn = _vm_cycle_load(c,t);
    T *stn = _t_get(t,fp);
    spec_is_ptr_equal(stn,t1);
    _vm_cycle_descend(fp,rtn,t);

    _vm_cycle_eval(fp,rtn,stn);
    Flow *f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,FLOW_PHASE_COMPLETE);
    spec_is_equal(fp[0],TREE_PATH_TERMINATOR);
    spec_is_equal(_f_noun(f),BOOLEAN_NOUN);
    spec_is_equal(*(int *)_f_surface(f),FALSE_VALUE);

    rtn = _vm_cycle_load(c,t);
    spec_is_false(_vm_cycle_descend(fp,rtn,t));
    stn = _t_get(t,fp);
    spec_is_ptr_equal(stn,t);
    _vm_cycle_eval(fp,rtn,stn);
    f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,3);
    spec_is_equal(fp[0],3);spec_is_equal(fp[1],TREE_PATH_TERMINATOR);

    rtn = _vm_cycle_load(c,t);
    spec_is_false(_vm_cycle_descend(fp,rtn,t));
    stn = _t_get(t,fp);
    spec_is_ptr_equal(stn,t3);
    _vm_cycle_eval(fp,rtn,stn);
    f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,FLOW_PHASE_COMPLETE);
    spec_is_equal(fp[0],TREE_PATH_TERMINATOR);
    spec_is_equal(_f_noun(f),INTEGER_NOUN);
    spec_is_equal(*(int *)_f_surface(f),2);

    rtn = _vm_cycle_load(c,t);
    spec_is_false(_vm_cycle_descend(fp,rtn,t));
    stn = _t_get(t,fp);
    spec_is_ptr_equal(stn,t);
    _vm_cycle_eval(fp,rtn,stn);
    f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,FLOW_PHASE_COMPLETE);
    spec_is_equal(_f_noun(f),INTEGER_NOUN);
    spec_is_equal(*(int *)_f_surface(f),2);
    spec_is_equal(_t_children(rtn),0);
    _t_free(t);_t_free(c);

}

void testVMCycle() {
    T *c = _context_create(2);
    T *t = _t_parse("(FLOW:IF (FLOW:IF (BOOLEAN:TRUE) (BOOLEAN:FALSE) (BOOLEAN:TRUE )) (INTEGER:1)(INTEGER:2))");

    T *rtn,*stn;
    int *fp;
    fp = __f_cur_flow_path(c);
    while (cycle(c,t));
    rtn = _t_get(__run_tree(c),fp);
    Flow *f = (Flow *)_t_surface(rtn);
    spec_is_equal(f->phase,FLOW_PHASE_COMPLETE);
    spec_is_equal(_f_noun(f),INTEGER_NOUN);
    spec_is_equal(*(int *)_f_surface(f),2);
    spec_is_equal(_t_children(rtn),0);
    _t_free(t);_t_free(c);
}

void testVM() {
    sys_defs_init();
    testVMCreateContext();
    testVMCycleLoad();
    testVMCycleDescend();
    testVMCycleEval();
    testVMCycle();
    //  testVMTransformData();
    //testVMTransformEQ();
    //testVMTransformReturn();
    // testVMTransformCondPair();
    //testVMTransformIter();
       /*
    testVMTransformRecursive();*/
    sys_defs_free();
}
