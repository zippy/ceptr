#include "../../src/ceptr.h"

void testGetSymbol() {
    Receptor tr;
    Receptor *r = &tr;

    stack_init(r);
    data_init(r);
    init_elements(r);
    noun_init(r);
    pattern_init(r);

    spec_is_true( getSymbol(r, "NOUN") == 0);
    spec_is_true( getSymbol(r, "PATTERN") == 40);
}

void testNoun() {
    testGetSymbol();
}
