#include "../ceptr.h"

#define _preop_get_array_length(surface) (*((int*)surface))

size_t array_get_spec_size(void *es) {
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
        if (typeTypeNoun != r->arraySpecXaddr.noun) {
            raise_error2("xaddr points to a %d, expected array(%d)\n", nounSpecType, r->arraySpecXaddr.noun);
        }
        Symbol repsNoun = REPS_GET_NOUN(rs);
        Symbol arrayItemType;
        ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
        typeTypeNoun = spec_noun_for_noun(r, nounSpecType);
        int size;
        if (typeTypeNoun == r->patternSpecXaddr.noun) {
            size = _pattern_get_size(es);
            surface += size * index;
        }
        else if (typeTypeNoun == r->arraySpecXaddr.noun) {
            while (index--) {
                surface += size_of_named_surface(r, repsNoun, surface);
            }
        }
        else {raise_error("bad array item type %d\n", arrayItemType);}
    }
    return surface;
}


int preop_get_array_length(Receptor *r, Xaddr rX) {
    return _preop_get_array_length(surface_for_xaddr(r, rX));
}

size_t _array_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    int length = _preop_get_array_length(surface);
    int size = sizeof(int);
    Symbol arrayItemType;
    int rep_size;

    Symbol repsNoun = REPS_GET_NOUN(spec_surface);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    if (typeTypeNoun == r->patternSpecXaddr.noun) {
        size += length * _pattern_get_size(es);
    }
    else if (typeTypeNoun == r->arraySpecXaddr.noun) {
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

size_t array_get_size(Receptor *r, Symbol noun, void *surface) {
    Symbol nounType;
    ElementSurface *spec_surface = spec_surface_for_noun(r, &nounType, noun);
    assert(nounType == r->arraySpecXaddr.noun);
    return _array_get_size(r, noun, spec_surface, surface);
}

Xaddr preop_new_rep(Receptor *r, Xaddr rep_type, Symbol rep_type_noun, char *label, Symbol repsNoun, int processCount, Process *processes) {
    char rs[BUFFER_SIZE];
    memset(rs, 0, BUFFER_SIZE);
    Symbol new_noun = init_element(r, label, rep_type, (ElementSurface *) rs, processCount, processes);
    REPS_SET_NOUN(rs, repsNoun);
    size_table_set(new_noun, (sizeFunction)array_get_spec_size);
    return preop_new(r, new_noun, rs);
}


Xaddr preop_new_array(Receptor *r, char *label, Symbol repsNoun, int processCount, Process *processes) {
    return preop_new_rep(r, r->arraySpecXaddr, r->arraySpecXaddr.noun, label, repsNoun, processCount, processes);
}
void proc_array_instance_new(Receptor *r) {
    raise_error0("proc_array_instance_new not implemented");
}

void dump_reps_spec(Receptor *r, void *surface) {
    ElementSurface *rs = (ElementSurface *)surface;

    printf("%s\n", "Array");
    printf("    name: %s(%d)\n", label_for_noun(r, rs->name), rs->name);
    int noun = REPS_GET_NOUN(rs);
    printf("    repsNoun: %s(%d) \n", label_for_noun(r, noun), noun);
    printf("\n");
}


void dump_array_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = *(int *) surface;
    surface += sizeof(int);
    Symbol arrayItemType;
    Symbol repsNoun = REPS_GET_NOUN(rs);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    int size;
    if (typeTypeNoun == r->patternSpecXaddr.noun) {
        printf("%s(%d) array of %d %s(%d)s\n", label_for_noun(r, rs->name), rs->name, count, label_for_noun(r, repsNoun), repsNoun);
        size = _pattern_get_size(es);
        while (count > 0) {
            printf("    ");
            dump_pattern_value(r, es, surface);
            printf("\n");
            surface += size;
            count--;
        }
    }
    else if (typeTypeNoun == r->arraySpecXaddr.noun) {
        printf("array of %d %s(%d) arrays\n", count, label_for_noun(r, rs->name), rs->name);
        while (count > 0) {
            printf("    ");
            dump_array_value(r, es, surface);
            surface += size_of_named_surface(r, repsNoun, surface);
            count--;
        }
    }
}


void array_init(Receptor *r){
    Symbol newNoun = data_new_noun(r, r->cspecXaddr, "ARRAY");
    size_table_set(newNoun, (sizeFunction)array_get_spec_size);
    UntypedProcess processes = {INSTANCE_NEW, (voidVoidFn) proc_array_instance_new };
    ElementSurface specSurface = { newNoun, 1, processes};
    r->arraySpecXaddr = data_new(r, newNoun, &specSurface,  element_header_size((void *)&specSurface));
}


