#include "../src/ceptr.h"

void testDefDumpSurface() {
    _d_dump(G_sys_defs);
    T *t = _t_new(0,CSTRING_NOUN,"fish",5);
    _d_dump(t);
}

void testDefSysDefs() {
    T *t = __d_get_def(META_NOUN);
    spec_is_equal(_t_noun(t),DEF_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"META");
    t = __d_get_def(DEF_NOUN);
    spec_is_equal(_t_noun(t),DEF_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"DEF");
    t = __d_get_def(DEF_DUMP_FUNC_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"DEF_DUMP_FUNC");
    t = __d_get_def(CSTRING_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"CSTRING");
    t = __d_get_def(XADDR_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"XADDR");
    t = __d_get_def(NOUN_NOUN);
    spec_is_str_equal((char *)_t_get_child_surface(t,1),"NOUN");
    T *n = _t_get_child(t,2);
    spec_is_equal(_t_noun(n),META_NOUN);
    spec_is_equal(*(int *)_t_surface(n),NOUN_NOUN);
    spec_is_equal(_t_children(n),2);

}

void testDef() {
    sys_defs_init();
    testDefSysDefs();
    //    testDefDumpSurface(); //commented out to not have to see this all the time... should be tested against a string or something
    sys_defs_free();
}
