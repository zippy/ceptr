#include "../src/ceptr.h"

void testFlowIf() {
    Tnode *r = _f_new(0);
    int fp[3] = {TREE_PATH_TERMINATOR};
    Flow *f = _t_surface(r);

    //null phase must throw bad phase error
    spec_is_ptr_equal(_f_if(fp,f,r),(char *)-1);

    Tnode *r1 = _f_new(r);

    f->phase = 1;
    spec_is_str_equal(_f_if(fp,f,r),"IF requires Boolean Noun");

    Flow *f1 = _t_surface(r1);
    f1->noun = BOOLEAN_NOUN;

    //eval true branch
    int v = TRUE_VALUE;
    f1->surface = &v;
    spec_is_equal(_t_children(r),1);
    spec_is_ptr_equal(_f_if(fp,f,r),(char *)0);
    spec_is_equal(fp[0],2);
    spec_is_equal(f->phase,2);

    //eval false branch
    v = FALSE_VALUE;
    f->phase = 1;
    fp[0] = TREE_PATH_TERMINATOR;
    spec_is_ptr_equal(_f_if(fp,f,r),(char *)0);
    spec_is_equal(f->phase,3);
    spec_is_equal(_t_children(r),2); // should have made dummy item for true branch

    // return val
    Tnode *r3 = _f_new(r);
    Flow *f3 = _t_surface(r3);
    f3->noun = INTEGER_NOUN;
    v = 99;
    f3->surface = &v;

    spec_is_ptr_equal(_f_if(fp,f,r),(char *)0);
    spec_is_equal(_t_children(r),0); // should have cleared children
    spec_is_equal(f->phase,FLOW_PHASE_COMPLETE);
    spec_is_equal(f->noun,INTEGER_NOUN);
    spec_is_equal(*(int *)f->surface,99);

    _t_free(r);
}

void testFlowDef() {
    Tnode *r = _f_new(0);
    int fp[3] = {TREE_PATH_TERMINATOR};
    Flow *f = _t_surface(r);

    //null phase must throw bad phase error
    spec_is_ptr_equal(_f_def(fp,f,r),(char *)-1);

    f->phase = 1;
    //   spec_is_str_equal(_f_def(fp,f,r),"DEF requires a Def Noun");



}

void testFlow() {
    testFlowIf();
    testFlowDef();
}
