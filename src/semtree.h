#include "ceptr.h"

// util function naming convention:  <returned-thing>_for_<given-thing>

void *surface_for_xaddr(Receptor *r, Xaddr xaddr) {
    return data_get(r, xaddr);
}

Xaddr xaddr_for_noun(Receptor *r, Symbol noun) {
    Xaddr nounXaddr = {noun, r->nounSpecXaddr.noun};
    return nounXaddr;
}

void *surface_for_noun(Receptor *r, Symbol noun) {
    return surface_for_xaddr(r, xaddr_for_noun(r, noun));
}

NounSurface *noun_surface_for_noun(Receptor *r, Symbol noun) {
    return (NounSurface *) surface_for_noun(r, noun);
}

ElementSurface *element_surface_for_xaddr(Receptor *r, Xaddr element) {
    return (ElementSurface *) surface_for_xaddr(r, element);
}

Xaddr spec_xaddr_for_noun(Receptor *r, Symbol noun){
    if (noun == r->cspecXaddr.noun) {
        raise_error0("no spec for the root xaddr\n");
        return r->cspecXaddr;
    } else if (noun == r->nounSpecXaddr.noun) {
        return r->nounSpecXaddr;
    } else if (noun == XADDR_NOUN) {
        return r->cspecXaddr; // FIXME: should be xaddrSpecXaddr
    } else {
        return noun_surface_for_noun(r, noun)->specXaddr;
    }
}

Xaddr spec_xaddr_for_xaddr(Receptor *r, Xaddr element) {
    return spec_xaddr_for_noun(r, element.noun);
}

Symbol spec_noun_for_xaddr(Receptor *r, Xaddr element) {
    return spec_xaddr_for_xaddr(r, element).noun;
}

Symbol spec_noun_for_noun(Receptor *r, Symbol noun) {
    return spec_xaddr_for_noun(r, noun).noun;
}

void *spec_surface_for_noun(Receptor *r, Symbol *nounType, Symbol noun) {
    void *surface = surface_for_xaddr(r, spec_xaddr_for_noun(r, noun));
    *nounType = spec_noun_for_noun(r, noun);
    return surface;
}

char *label_for_noun(Receptor *r, Symbol noun) {
    switch (noun) {
        case XADDR_NOUN:
            return "Xaddr";
        case CSPEC_NOUN:
            return "Cspec";
        case CSPEC:
            return "Root";
        case CSTRING_NOUN:
            return "Cstring";
        default:
            return &noun_surface_for_noun(r, noun)->label;
    }
}


// this should go somewhere once the dependencies are better resolved.  right now it depends on everything.
size_t size_of_named_surface(Receptor *r, Symbol instanceNoun, void *surface) {
    size_t result = 0;
    if (instanceNoun <= 0){
        switch (instanceNoun) {
            case XADDR_NOUN:
                result = sizeof(Xaddr);
                break;
            case CSPEC_NOUN:
                result = (size_t) 0;
                break;
            case CSTRING_NOUN:
                result = strlen((char *) surface) + 1;
                break;
            case PATTERN_SPEC_DATA_NOUN:
                result = sizeof(PatternSpecData);
                break;
            default:
                raise_error("can't get size of instance of %d \n", instanceNoun);
        }
    } else {
        result = (*size_table_get(spec_noun_for_noun(r, instanceNoun)))(r, instanceNoun, surface);
    }
    return result;
}
