#include "ceptr.h"


void dump_spec_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *) surface;
    printf("Spec\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    %d processes: ", ps->process_count);
    dump_process_array((Process *) &ps->processes, ps->process_count);
    printf("\n");
}

void dump_xaddr(Receptor *r, Xaddr xaddr, int indent_level) {
    Symbol typeNoun = spec_noun_for_xaddr(r, xaddr);

    ElementSurface *es;
    NounSurface *ns;
    void *surface;
    int key = xaddr.key;
    int noun = xaddr.noun;
    if (noun == 1 && key == 2) {
        dump_spec_spec(r, _data_get(&r->data,key));
    }
    else if (noun == r->nounSpecXaddr.noun) {
        dump_noun(r, (NounSurface *) _data_get(&r->data,key));

    } else if (typeNoun == CSPEC_NOUN) {
        dump_spec_spec(r, _data_get(&r->data,key));

    } else if (typeNoun == r->patternSpecXaddr.noun) {
        dump_pattern_spec(r, _data_get(&r->data,key));

    } else if (typeNoun == r->arraySpecXaddr.noun) {
        dump_reps_spec(r, _data_get(&r->data,key));

    } else {
        Symbol typeTypeNoun = spec_noun_for_noun(r, typeNoun);
        if (typeTypeNoun == r->nounSpecXaddr.noun) {
            ns = (NounSurface *) _data_get(&r->data,key);;
            dump_noun(r, ns);
        }
        else {
            surface = surface_for_xaddr(r, xaddr_for_noun(r, noun));
            ns = (NounSurface *) surface;
            printf("%s : ", &ns->label);
            es = element_surface_for_xaddr(r, ns->specXaddr);
            if (typeTypeNoun == r->patternSpecXaddr.noun) {
                dump_pattern_value(r, es, noun, surface_for_xaddr(r, xaddr));
            } else if (typeTypeNoun == r->arraySpecXaddr.noun) {
                dump_array_value(r, es, surface_for_xaddr(r, xaddr));
            }
        }
    }
}

void dump_xaddrs(Receptor *r) {
    NounSurface *ns;
    void *surface;
    Data *d = &r->data;
    for (int i = 1; i <= _t_children(d->root); i++) {
	Tnode *t = _t_get_child(d->root,i);
	Xaddr x = {i,_t_noun(t)};
	printf("Xaddr { %5d, %5d } - ", x.key, x.noun);
        dump_xaddr(r, x, 0);
        printf("\n");
    }
}
