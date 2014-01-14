#include "ceptr.h"

void init_elements(Receptor *r) {
    r->rootXaddr.key = ROOT;
    r->rootXaddr.noun = ROOT;
    r->rootSurface.name = ROOT;
    r->rootSurface.process_count = 0;

    r->cspecXaddr.key = CSPEC;
    r->cspecXaddr.noun = CSPEC_NOUN;
}

void init_builtins(Receptor *r) {
    noun_init(r);
    pattern_init(r);
    array_init(r);
    int_init(r);
}

void init(Receptor *r) {
    stack_init(r);
    //    init_processing(r);
    data_init(r);
    init_elements(r);
    init_builtins(r);
}
