#include "../ceptr.h"


size_t noun_get_size(Receptor *r, Symbol noun, void *surface) {
    return sizeof(NounSurface);
}

void proc_noun_instance_new(Receptor *r) {
    char label[255];
    Xaddr spec;
    stack_pop(r,CSTRING_NOUN,label);
    stack_pop(r,XADDR_NOUN,&spec);
    spec.key = preop_new_noun(r,spec,label);
    spec.noun = r->nounSpecXaddr.noun;
    size_table_set(spec.key, noun_get_size);
    stack_push(r,XADDR_NOUN,&spec);
}

size_t noun_get_spec_size(Receptor *r, Symbol noun, void *surface) {
    return element_header_size((ElementSurface *) surface);
}


Symbol getSymbol(Receptor *r, char *label) {
    NounSurface *ns;
    Symbol noun;
    int i;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        if (r->data.xaddrs[i].noun == r->nounSpecXaddr.noun) {
            noun = r->data.xaddrs[i].key;
            ns = (NounSurface *) &r->data.cache[noun];
            if (!strcmp(label, &ns->label)) {
                return noun;
            }
        }
    }
    raise_error("couldn't find symbol for label %s\n", label);
    return 0;
}

void noun_init(Receptor *r){
    // we know this is the first Xaddr in system and it will try to look itself up and that
    // breaks if we don't initialize it.
    r->nounSpecXaddr.key = 16;
    r->nounSpecXaddr.noun = 0;

    size_table_set(0, noun_get_size);
    data_new_noun(r, r->cspecXaddr, "NOUN");
    UntypedProcess noun_processes = {INSTANCE_NEW, (voidVoidFn)proc_noun_instance_new };
    ElementSurface nounSpecSurface = {0, 1, noun_processes};
    data_new(r, r->nounSpecXaddr.noun, &nounSpecSurface,  element_header_size((void *)&nounSpecSurface));
}

void dump_noun(Receptor *r, NounSurface *ns) {
    printf("Noun      { %d, %5d } %s", ns->specXaddr.key, ns->specXaddr.noun, &ns->label);
}

