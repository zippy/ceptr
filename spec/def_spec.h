#include "../src/ceptr.h"

void testDefDumpSurface() {
    _d_dump_surface(G_sys_defs);
    Tnode *t = _t_new(0,CSTRING_NOUN,"fish",5);
    _d_dump_surface(t);
}

void testDef() {
    sys_defs_init();
    //        testDefDumpSurface(); commented out to not have to see this all the time... should be tested against a string or something
    sys_defs_free();
}
