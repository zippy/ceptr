#include "../src/ceptr.h"

Tnode * _testEQ(void *parent,int v1, int v2) {
    Tnode *eq = _t_newi(parent,INSTRUCTION_NOUN,I_EQ);
    _t_newi(eq,INTEGER_NOUN,v1);
    _t_newi(eq,INTEGER_NOUN,v2);
    return eq;
}

void testVMReduceEQ() {
    Tnode *eq = _testEQ(0,2,2);
    spec_is_equal(reduce(eq),0);
    spec_is_equal(_t_children(eq),REDUCE_OK);
    spec_is_equal(*(int *)_t_surface(eq),TRUE_VALUE);
    spec_is_equal(_t_noun(eq),BOOLEAN_NOUN);
    _t_free(eq);

    eq = _testEQ(0,2,1);
    spec_is_equal(reduce(eq),REDUCE_OK);
    spec_is_equal(_t_children(eq),0);
    spec_is_equal(*(int *)_t_surface(eq),FALSE_VALUE);
    spec_is_equal(_t_noun(eq),BOOLEAN_NOUN);
    _t_free(eq);
}

Tnode *_testCP(Tnode *parent,int v1,int v2,int result) {
    Tnode *cp = _t_newi(parent,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,v1,v2);
    Tnode *i = _t_newi(cp,INTEGER_NOUN,result);
    return cp;
}

void testVMReduceCondPair() {
    Tnode *t = _testCP(0,1,2,99);
    spec_is_equal(reduce(t),REDUCE_OK);
    spec_is_equal(_t_children(t),2);
    spec_is_equal(*(int *)_t_surface(t),I_COND_PAIR);
    spec_is_equal(_t_noun(t),INSTRUCTION_NOUN);
    _t_free(t);

    t = _testCP(0,2,2,99);
    spec_is_equal(reduce(t),REDUCE_OK);
    spec_is_equal(_t_children(t),0);
    spec_is_equal(*(int *)_t_surface(t),99);
    spec_is_equal(_t_noun(t),INTEGER_NOUN);
    _t_free(t);
}

Tnode *_testC(Tnode *parent) {
    Tnode *c = _t_newi(parent,INSTRUCTION_NOUN,I_COND);
    _testCP(c,1,2,99);
    _testCP(c,3,2,98);
    return c;
}

void testVMReduceCond() {
    Tnode *c = _testC(0);
    _testCP(c,2,2,22);

    spec_is_equal(reduce(c),REDUCE_OK);

    spec_is_equal(*(int *)_t_surface(c),22);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    spec_is_equal(_t_children(c),0);

    _t_free(c);
}

void testVMReduceRecursive() {
    Tnode *cp = _t_newi(0,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,2,2);
    Tnode *cp1 = _t_newi(cp,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp1,2,2);
    Tnode *i = _t_newi(cp1,INTEGER_NOUN,99);

    spec_is_equal(reduce(cp),REDUCE_OK);
    spec_is_equal(*(int *)_t_surface(cp),99);
    spec_is_equal(_t_noun(cp),INTEGER_NOUN);
    _t_free(cp);

    Tnode *c = _testC(0);

    cp = _t_newi(c,INSTRUCTION_NOUN,I_COND_PAIR);
    _testEQ(cp,4,4);
    Tnode *c2 = _testC(cp);
    _testCP(c2,3,3,33);

    spec_is_equal(reduce(c),REDUCE_OK);
    spec_is_equal(*(int *)_t_surface(c),33);
    spec_is_equal(_t_noun(c),INTEGER_NOUN);
    _t_free(c);

}

void testVM() {
    testVMReduceEQ();
    testVMReduceCondPair();
    testVMReduceCond();
    testVMReduceRecursive();
}
