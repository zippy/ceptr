#include "../../src/ceptr.h"

void testGetSymbol() {
    Receptor tr;
    Receptor *r = &tr;

    stack_init(r);
    data_init(r);
    init_elements(r);
    noun_init(r);
    pattern_init(r);

    spec_is_equal( getSymbol(r, "NOUN"),1);
    spec_is_equal( getSymbol(r, "PATTERN"), 3);
}

void testNoun() {
    testGetSymbol();
}
