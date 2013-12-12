#include "../ceptr.h"

size_t noun_get_size(Receptor *r, Symbol noun, void *surface) {
    return sizeof(NounSurface);
}

size_t get_noun_spec_size(void *es) {
    return element_header_size((ElementSurface *) es);
}

void proc_noun_instance_new(Receptor *r) {
    char label[255];
    Xaddr spec;
    stack_pop(r,CSTRING_NOUN,label);
    stack_pop(r,XADDR_NOUN,&spec);
    spec.key = preop_new_noun(r,spec,label);
    spec.noun = r->nounNoun;
    size_table_set(spec.key, noun_get_size);
    stack_push(r,XADDR_NOUN,&spec);
}
