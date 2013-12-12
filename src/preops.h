#include "ceptr.h"

#define _preop_get_array_length(surface) (*((int*)surface))


void preop_set(Receptor *r, Xaddr xaddr, void *value) {
    size_t size = size_of_named_surface(r, xaddr.noun, value);
    data_set(r, xaddr, value, size);
}

Xaddr preop_new(Receptor *r, Symbol noun, void *surface) {
    size_t size = size_of_named_surface(r, noun, surface);
    return data_new(r, noun, surface, size);
}

Symbol preop_new_noun(Receptor *r, Xaddr xaddr, char *label) {
    return data_new_noun(r, xaddr, label);
}



Xaddr preop_new_pattern(Receptor *r, char *label, int child_count_or_size, Xaddr *children, int processCount, Process *processes) {
    char ps[BUFFER_SIZE];
    memset(ps, 0, BUFFER_SIZE);
    int i;
    Symbol newNoun = init_element(r, label, r->patternSpecXaddr, (ElementSurface *) ps, processCount, processes);

    if (children == 0) {
        pattern_set_size(ps, child_count_or_size);
    } else {
        NounSurface *noun;
        ElementSurface *child_pattern_surface;
        int size = 0;
        Offset *pschildren = PATTERN_GET_CHILDREN(ps);

        for (i = 0; i < child_count_or_size; i++) {
            if (children[i].noun == r->nounNoun) {
                noun = (NounSurface *) surface_for_xaddr(r, children[i]);
                child_pattern_surface = element_surface_for_xaddr(r, noun->specXaddr);
            } else if (children[i].noun == r->patternNoun) {
                child_pattern_surface = element_surface_for_xaddr(r, children[i]);
            } else {
                raise_error("Unknown child element type %d\n", children[i].noun);
            }
            pschildren[i].noun.key = children[i].key;
            pschildren[i].noun.noun = children[i].noun;
            pschildren[i].offset = size;
            size += pattern_get_size(child_pattern_surface);
        }
        pattern_set_size(ps, size);
        PATTERN_SET_CHILDREN_COUNT(ps, child_count_or_size);

    }
    return preop_new(r, newNoun, ps);
}

