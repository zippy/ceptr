#include "ceptr.h"


Symbol preop_new_noun(Receptor *r, Xaddr xaddr, char *label) {
    return data_new_noun(r, xaddr, label);
}

void preop_set(Receptor *r, Xaddr xaddr, void *value) {
    size_t size = size_of_named_surface(r, xaddr.noun, value);
    data_set(r, xaddr, value, size);
}

Xaddr preop_new(Receptor *r, Symbol noun, void *surface) {
    size_t size = size_of_named_surface(r, noun, surface);
    return data_new(r, noun, surface, size);
}

