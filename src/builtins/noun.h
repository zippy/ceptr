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
    Data *d = &r->data;
    //TODO: convert to using itterator when we have short-circuiting
    for (int i = 1; i <= _t_children(d->root); i++) {
	Tnode *t = _t_get_child(d->root,i);
	Xaddr x = {i,_t_noun(t)};
        if (x.noun == r->nounSpecXaddr.noun) {
            noun = x.key;
            ns = (NounSurface *) _data_get(d,noun);
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
    r->nounSpecXaddr.key = 2;
    r->nounSpecXaddr.noun = 1;

    size_table_set(1, noun_get_size);
    data_new_noun(r, r->cspecXaddr, "NOUN");
    UntypedProcess noun_processes = {INSTANCE_NEW, (voidVoidFn)proc_noun_instance_new };
    ElementSurface nounSpecSurface = {1, 1, noun_processes};
    data_new(r, r->nounSpecXaddr.noun, &nounSpecSurface,  element_header_size((void *)&nounSpecSurface));
}

void dump_noun(Receptor *r, NounSurface *ns) {
    printf("Noun      { %d, %5d } %s", ns->specXaddr.key, ns->specXaddr.noun, &ns->label);
}

Xaddr new_noun(Receptor *r,Xaddr type, char * label) {
    stack_push(r, XADDR_NOUN, &type);
    stack_push(r, CSTRING_NOUN, label);
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr x;
    stack_pop(r, XADDR_NOUN, &x);
    return x;
}
