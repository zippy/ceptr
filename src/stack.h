#include "ceptr.h"


void stack_peek_unchecked(Receptor *r, Symbol *name, void **surface) {
    assert(r->semStackPointer >= 0);
    SemStackFrame *ssf = &r->semStack[r->semStackPointer];
    *name = ssf->noun;
    *surface = &r->valStack[r->valStackPointer - ssf->size];
}

void stack_pop_unchecked(Receptor *r, Symbol *name, void *surface) {
    SemStackFrame *ssf = &r->semStack[r->semStackPointer];
    *name = ssf->noun;
    memcpy(surface, &r->valStack[r->valStackPointer - ssf->size], ssf->size);
    r->valStackPointer -= ssf->size;
    r->semStackPointer--;
}

void stack_peek(Receptor *r, Symbol expectedNoun, void **surface){
    Symbol actualNoun;
    stack_peek_unchecked(r, &actualNoun, surface);
    assert( actualNoun == expectedNoun );
}

void stack_pop(Receptor *r, Symbol expectedNoun, void *surface) {
    Symbol actualNoun;
    stack_pop_unchecked(r, &actualNoun, surface);
    assert( actualNoun == expectedNoun );
}

void stack_push(Receptor *r, Symbol name, void *surface) {
    SemStackFrame *ssf = &r->semStack[++r->semStackPointer];
    ssf->noun = name;
    ssf->size = size_of_named_surface(r, name, surface);
    memcpy(&r->valStack[r->valStackPointer], surface, ssf->size);
    r->valStackPointer += ssf->size;
}

void stack_dump(Receptor *r) {
    int i, v = 0;
    char *unknown = "<unknown>";
    char *label;
    ElementSurface *ps;
    SemStackFrame *ssf;
    for (i = 0; i <= r->semStackPointer; i++) {
        ssf = &r->semStack[i];
        printf("\nStack frame: %d is a %s(%d) size:%d\n", i, label_for_noun(r, ssf->noun), ssf->noun, (int)ssf->size);
        printf("\n");
    }
}

void stack_init(Receptor *r) {
    r->semStackPointer = -1;
    r->valStackPointer = 0;
}
