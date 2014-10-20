#include "../src/ceptr.h"

void testFlowIf() {
    T *r = _f_new(0);
    int fp[3] = {TREE_PATH_TERMINATOR};
    Flow *f = _t_surface(r);

    //null phase must throw bad phase error
    spec_is_ptr_equal(_f_if(fp,f,r),(char *)-1);

    T *r1 = _f_new(r);

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
    T *r3 = _f_new(r);
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
    T *r = _f_new(0);
    int fp[3] = {TREE_PATH_TERMINATOR};
    Flow *f = _t_surface(r);

    //null phase must throw bad phase error
    spec_is_ptr_equal(_f_def(fp,f,r),(char *)-1);

    f->phase = 1;
    fp[0] = 1;
    fp[1] = TREE_PATH_TERMINATOR;
    spec_is_str_equal(_f_def(fp,f,r),"DEF requires a CSTRING as 1st child");
    T *r1 = _f_new(r);
    Flow *f1 = _t_surface(r1);
    f1->noun = CSTRING_NOUN;
    f1->surface = "Fish";

    spec_is_ptr_equal(_f_def(fp,f,r),(char *)0);
    spec_is_equal(fp[0],2);
    spec_is_true(f->phase == 2);


    spec_is_str_equal(_f_def(fp,f,r),"DEF requires a NOUNTREE as 2nd child");

    T *r2 = _f_new(r);
    Flow *f2 = _t_surface(r2);
    f2->noun = NOUNTREE_NOUN;

    T *r3 = _f_new(r2);
    Flow *f3 = _t_surface(r3);
    f3->noun = META_NOUN;

    *(int *)&f3->surface = CSTRING_NOUN;

    spec_is_ptr_equal(_f_def(fp,f,r),(char *)0);
    spec_is_true(f->phase == FLOW_PHASE_COMPLETE);

    // The new noun def should be in the flow state as a result
    T *d = __d_get_def(f->noun);
    spec_is_equal(f->noun,G_sys_noun_id);
    char *label = (char *)_t_get_child_surface(d,DEF_LABEL_CHILD);
    spec_is_str_equal(label,"Fish");

    T *noun_tree = _t_parse("(META:Fish (META:CSTRING))");
    spec_is_true(_t_nouns_eq(noun_tree,_t_get_child(d,DEF_NOUNTREE_CHILD)));

}

void testFlow() {
    sys_defs_init();
    testFlowIf();
    testFlowDef();
    sys_defs_free();
}
