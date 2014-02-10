#include "../../src/ceptr.h"

void testPatternHelpers() {
    Receptor tr; init(&tr); Receptor *r = &tr;
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    spec_is_equal(pattern_get_size(r,MY_INT,0),4); // don't need an actual surface to get a pattern's size
}

//TODO: fill out tests
void testPattern(){
    testPatternHelpers();
}
