#include "ceptr.h"


Symbol preop_new_noun(Receptor *r, Xaddr xaddr, char *label) {

    size_t current_index = r->data.cache_index;
    NounSurface *ns = (NounSurface *)&r->data.cache[current_index];
    ns->specXaddr.key = xaddr.key;
    ns->specXaddr.noun = xaddr.noun;
    memcpy(&ns->label,label,strlen(label)+1);
    r->data.cache_index += get_noun_size(ns);

    _record_existence(r, current_index, NOUN_NOUN);

    return current_index;
}

void preop_set(Receptor *r, Xaddr xaddr, void *value) {
    void *surface = &r->data.cache[xaddr.key];
    if (!sem_check(r, xaddr)) {
        raise_error("I do not think that word (%d) means what you think it means!\n", xaddr.noun);
    }
    size_t size = size_of_named_surface(r, xaddr.noun, value);

    memcpy(surface, value, size);
}




Xaddr preop_new(Receptor *r, Symbol noun, void *surface) {
    size_t current_index = r->data.cache_index;
    r->data.cache_index += size_of_named_surface(r, noun, surface);
    _record_existence(r, current_index, noun);
    Xaddr new_xaddr = {current_index, noun};
    preop_set(r, new_xaddr, surface);
    return new_xaddr;
}

Xaddr preop_new_pattern(Receptor *r, char *label, int child_count_or_size, Xaddr *children, int processCount, Process *processes) {
    char ps[BUFFER_SIZE];
    memset(ps, 0, BUFFER_SIZE);
    int i;
    Symbol newNoun = _init_element(r, label, r->patternSpecXaddr, (ElementSurface *) ps, processCount, processes);

    if (children == 0) {
        pattern_set_size(ps, child_count_or_size);
    } else {
        NounSurface *noun;
        ElementSurface *child_pattern_surface;
        int size = 0;
        Offset *pschildren = PATTERN_GET_CHILDREN(ps);

        for (i = 0; i < child_count_or_size; i++) {
            if (children[i].noun == NOUN_NOUN) {
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


void *preop_get_array_nth(Receptor *r, int index, Xaddr rX) {
    void *surface = surface_for_xaddr(r, rX);
    int length = _preop_get_array_length(surface);
    if (index >= length) {
        raise_error2("index %d into array %d greater than length\n", rX.key, index);
    }
    surface += sizeof(int);
    if (index > 0) {

        Symbol nounSpecType;
        ElementSurface *rs = spec_surface_for_noun(r, &nounSpecType, rX.noun);
        Symbol typeTypeNoun = spec_noun_for_noun(r, nounSpecType);
        if (typeTypeNoun != r->arrayNoun) {
            raise_error2("xaddr points to a %d, expected array(%d)\n", nounSpecType, r->arrayNoun);
        }
        Symbol repsNoun = REPS_GET_NOUN(rs);
        Symbol arrayItemType;
        ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
        typeTypeNoun = spec_noun_for_noun(r, nounSpecType);
        int size;
        if (typeTypeNoun == r->patternNoun) {
            size = pattern_get_size(es);
            surface += size * index;
        }
        else if (typeTypeNoun == r->arrayNoun) {
            while (index--) {
                surface += size_of_named_surface(r, repsNoun, surface);
            }
        }
        else {raise_error("bad array item type %d\n", arrayItemType);}
    }
    return surface;
}

Xaddr preop_new_rep(Receptor *r, Xaddr rep_type, Symbol rep_type_noun, char *label, Symbol repsNoun, int processCount, Process *processes) {
    char rs[BUFFER_SIZE];
    memset(rs, 0, BUFFER_SIZE);
    Symbol new_noun = _init_element(r, label, rep_type, (ElementSurface *) rs, processCount, processes);
    REPS_SET_NOUN(rs, repsNoun);
    return preop_new(r, new_noun, rs);
}

Xaddr preop_new_array(Receptor *r, char *label, Symbol repsNoun, int processCount, Process *processes) {
    return preop_new_rep(r, r->arraySpecXaddr, r->arrayNoun, label, repsNoun, processCount, processes);
}
