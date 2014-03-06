#include "../src/ceptr.h"

void testVMTransformData() {
    Tnode *i = _t_newi(0,BOOLEAN_NOUN,TRUE_VALUE);
    _t_newi(i,PTR_NOUN,1);
    Tnode *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);

    spec_is_equal(transform(i,r),TRANSFORM_OK);
    spec_is_equal(_t_noun(r),BOOLEAN_NOUN);
    spec_is_equal(*(int *)_t_surface(r),TRUE_VALUE);

    Tnode *t = _t_get_child(r,1);
    spec_is_equal(_t_noun(t),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(t),42);
    _t_free(i);
    _t_free(r);
}

Tnode * _testEQ(void *parent) {
    Tnode *eq = _t_newi(parent,INSTRUCTION_NOUN,I_EQ);
    _t_newi(eq,PTR_NOUN,1);
    _t_newi(eq,PTR_NOUN,2);
    return eq;
}

void testVMTransformEQ() {
    Tnode *i = _testEQ(0);
    Tnode *r = _t_newi(0,RUNTREE_NOUN,0);
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
    Tnode *c = _t_newi(0,INSTRUCTION_NOUN,I_COND);
    Tnode *r = _t_newi(c,INSTRUCTION_NOUN,I_RETURN);
    Tnode *i = _t_newi(r,INTEGER_NOUN,314);
    spec_is_equal(_transform(r),TRANSFORM_OK);
    spec_is_equal(_t_children(c),0);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    spec_is_equal(*(int *)_t_surface(c),314);
}

Tnode *_testCP(Tnode *parent) {
    Tnode *cp = _t_newi(parent,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp);
    _t_newi(cp,PTR_NOUN,3);
    return cp;
}

void testVMTransformCondPair() {
    Tnode *t = _testCP(0);
    Tnode *r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,43);
    _t_newi(r,INTEGER_NOUN,99);

    spec_is_equal(transform(t,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),2);
    spec_is_equal(*(int *)_t_surface(r),I_COND_PAIR);
    spec_is_equal(_t_noun(r),INSTRUCTION_NOUN);
    _t_free(r);

    r = _t_newi(0,RUNTREE_NOUN,0);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,42);
    _t_newi(r,INTEGER_NOUN,99);

    spec_is_equal(transform(t,r),TRANSFORM_OK);
    spec_is_equal(_t_children(r),1);
    spec_is_equal(_t_noun(r),INSTRUCTION_NOUN);
    spec_is_equal(*(int *)_t_surface(r),I_RETURN);

    Tnode *cr = _t_get_child(r,1);
    spec_is_equal(_t_children(cr),0);
    spec_is_equal(*(int *)_t_surface(cr),99);
    spec_is_equal(_t_noun(cr),INTEGER_NOUN);
    _t_free(r);
    _t_free(t);
}

void testVMTransformIter() {
    Tnode *i = _t_newi(0,INSTRUCTION_NOUN,I_ITER);
    _testCP(i);
    Tnode *r = _t_newi(i,RUNTREE_NOUN,0);

    Tnode *ir = _t_newi(r,RUNTREE_NOUN,0);
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
    Tnode *cp = _t_newi(0,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,2,2);
    Tnode *cp1 = _t_newi(cp,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp1,2,2);
    Tnode *i = _t_newi(cp1,INTEGER_NOUN,99);

    spec_is_equal(transform(cp),TRANSFORM_OK);
    spec_is_equal(*(int *)_t_surface(cp),99);
    spec_is_equal(_t_noun(cp),INTEGER_NOUN);
    _t_free(cp);

    Tnode *c = _testC(0);

    cp = _t_newi(c,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,4,4);
    Tnode *c2 = _testC(cp);
    _testCP(c2,3,3,33);

    spec_is_equal(transform(c),TRANSFORM_OK);
    spec_is_equal(*(int *)_t_surface(c),33);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    _t_free(c);

    }*/

void testVM() {
    testVMTransformData();
    testVMTransformEQ();
    testVMTransformReturn();
    testVMTransformCondPair();
    testVMTransformIter();
       /*
    testVMTransformRecursive();*/
}
