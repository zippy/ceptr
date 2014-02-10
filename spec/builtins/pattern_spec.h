#include "../../src/ceptr.h"

void testPatternHelpers() {
    Receptor tr; init(&tr); Receptor *r = &tr;
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    Symbol s;
    spec_is_equal(_pattern_get_size(spec_surface_for_noun(r,&s,MY_INT)),4);
}

//TODO: fill out tests
void testPattern(){
    testPatternHelpers();
}
