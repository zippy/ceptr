#include "ceptr.h"


int proc_array_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
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
            rep_size = proc_array_get_size(r, repsNoun, item_spec_surface, surface);
            size += rep_size;
            surface += rep_size;
        }
    }
        //TODO: handle arrays of strings
    else {raise_error2("illegal noun (%d) as array element type for %d\n", arrayItemType, noun);}
    return size;
}

int proc_noun_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    return sizeof(NounSurface);
}

void proc_noun_instance_new(Receptor *r) {
    char label[255];
    Xaddr spec;
    stack_pop(r,CSTRING_NOUN,label);
    stack_pop(r,XADDR_NOUN,&spec);
    spec.key = preop_new_noun(r,spec,label);
    spec.noun = r->nounNoun;
    stack_push(r,XADDR_NOUN,&spec);
}

int proc_pattern_get_size(Receptor *r, Symbol noun, ElementSurface *spec_surface, void *surface) {
    return pattern_get_size(spec_surface);
}

void proc_pattern_instance_new(Receptor *r) {
    PatternSpecData *d;
    stack_peek(r, PATTERN_SPEC_DATA_NOUN, (void **)&d);
    Xaddr pattern_xaddr = preop_new_pattern(r, d->label, d->child_count, d->children, d->process_count, d->processes);
    stack_push(r, XADDR_NOUN, &pattern_xaddr);
}

int proc_int_inc(Receptor *r, void *this) {
    ++*(int *) (this);
    return 0;
}

int proc_int_add(Receptor *r, void *this) {
    // semCheck please
    int *stackSurface = (int *) &r->valStack[r->valStackPointer - r->semStack[r->semStackPointer].size];
    *stackSurface = *(int *) this + *stackSurface;
    return 0;
}

int proc_int_print(Receptor *r, void *this) {
    printf("%d", *(int *) this);
}

int proc_point_print(Receptor *r, void *this) {
    printf("%d,%d", *(int *) this, *(((int *) this) + 1));
}

int proc_line_print(Receptor *r, void *this) {
    int *surface = (int *) this;
    printf("[%d,%d - %d,%d] ", *surface, *(surface + 1), *(surface + 2), *(surface + 3));
}
