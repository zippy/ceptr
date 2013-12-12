#include "../ceptr.h"

size_t get_array_spec_size(void *es) {
    return element_header_size((ElementSurface *) es) + sizeof(RepsBody);
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
    Symbol new_noun = init_element(r, label, rep_type, (ElementSurface *) rs, processCount, processes);
    REPS_SET_NOUN(rs, repsNoun);
    size_table_set(new_noun, array_get_size);
    return preop_new(r, new_noun, rs);
}

Xaddr preop_new_array(Receptor *r, char *label, Symbol repsNoun, int processCount, Process *processes) {
    return preop_new_rep(r, r->arraySpecXaddr, r->arrayNoun, label, repsNoun, processCount, processes);
}


int preop_get_array_length(Receptor *r, Xaddr rX) {
    return _preop_get_array_length(surface_for_xaddr(r, rX));
}

size_t _array_get_size(Receptor *r, Symbol noun, void *surface) {
    Symbol nounType;

    ElementSurface *spec_surface = spec_surface_for_noun(r, &nounType, noun);
    assert(nounType == r->arrayNoun);
    _array_get_size(r, noun, spec_surface, surface);
}

size_t _array_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    int length = _preop_get_array_length(surface);
    int size = sizeof(int);
    Symbol arrayItemType;
    int rep_size;

    Symbol repsNoun = REPS_GET_NOUN(spec_surface);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    if (typeTypeNoun == r->patternNoun) {
        size += length * pattern_get_size(es);
    }
    else if (typeTypeNoun == r->arrayNoun) {
        surface += sizeof(int);
        while (length--) {
            Symbol itemType;
            ElementSurface *item_spec_surface = spec_surface_for_noun(r, &itemType, repsNoun);
            rep_size = _array_get_size(r, repsNoun, item_spec_surface, surface);
            size += rep_size;
            surface += rep_size;
        }
    }
        //TODO: handle arrays of strings
    else {raise_error2("illegal noun (%d) as array element type for %d\n", arrayItemType, noun);}
    return size;
}
