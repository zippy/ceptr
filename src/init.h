#include "ceptr.h"

void init_elements(Receptor *r) {
    r->rootXaddr.key = ROOT;
    r->rootXaddr.noun = ROOT;
    r->rootSurface.name = ROOT;
    r->rootSurface.process_count = 0;

    r->cspecXaddr.key = CSPEC;
    r->cspecXaddr.noun = CSPEC_NOUN;

    r->membraneXaddr.key = MEMBRANE;
    r->membraneXaddr.noun = CSPEC;
    r->xaddrXaddr.key = XADDR_NOUN;
    r->xaddrXaddr.noun = CSPEC;
}

void init_processing(Receptor *r) {
    r->listenerCount = 0;
    r->signalProc = 0;
}

void init_builtins(Receptor *r) {
    noun_init(r);
    pattern_init(r);
    array_init(r);
    int_init(r);
    str255_init(r);
    cfunc_init(r);

    r->charIntNoun = preop_new_noun(r, r->intPatternSpecXaddr, "CHAR_INT");

//    stream_init(r);
//    receptor_init(r);
}

void init(Receptor *r) {
    stack_init(r);
    init_processing(r);
    _data_init(&r->data);
    init_elements(r);
    init_builtins(r);
}
