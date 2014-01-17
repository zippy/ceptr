#include "../src/ceptr.h"

void testReceptorUtil(){
    Receptor tr; Receptor *r = &tr; init(r);

    spec_is_true(!logChange(r));
    data_write_log(NULL, r, CSPEC_NOUN, r, 3);  // what's being written doesn't work
    spec_is_true(logChange(r));


}