#include "../ceptr.h"


typedef struct {
    size_t size;
    int children_count;
    Offset children;
} PatternBody;



#define PATTERN_GET_CHILDREN_COUNT(pat) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count)
#define PATTERN_SET_CHILDREN_COUNT(pat,count) (((PatternBody *)SKIP_ELEM_HEADER(pat))->children_count=count)
#define PATTERN_GET_CHILDREN(pat) (&(((PatternBody *)SKIP_ELEM_HEADER(pat))->children))
//#define PATTERN_GET_SIZE(pat) (((PatternBody *)skip_elem_header(pat))->size)
//#define PATTERN_SET_SIZE(pat,s) (((PatternBody *)skip_elem_header(pat))->size = s)


size_t _pattern_get_size(void *pattern_surface) {
    return ((PatternBody *) skip_elem_header(pattern_surface))->size;
}

size_t pattern_get_size(Receptor *r, Symbol noun, void *pattern_surface) {
    Symbol nounType;
    ElementSurface *spec_surface = spec_surface_for_noun(r, &nounType, noun);
    return _pattern_get_size(spec_surface);
}

void pattern_set_size(void *pattern_surface, size_t size) {
    ((PatternBody *) skip_elem_header(pattern_surface))->size = size;
}

size_t pattern_get_spec_size(Receptor *r, Symbol noun, ElementSurface *es) {
    return ELEMENT_HEADER_SIZE(es) + sizeof(PatternBody) - sizeof(Offset) + sizeof(Offset) * PATTERN_GET_CHILDREN_COUNT(es);
}

Xaddr preop_new_pattern(Receptor *r, char *label, int child_count_or_size, Xaddr *children, int processCount, Process *processes) {
    char ps[BUFFER_SIZE];
    memset(ps, 0, BUFFER_SIZE);
    int i;
    Symbol newNoun = init_element(r, label, r->patternSpecXaddr, (ElementSurface *) ps, processCount, processes);
    size_table_set(newNoun, pattern_get_size);

    if (children == 0) {
        pattern_set_size(ps, child_count_or_size);
    } else {
        NounSurface *noun;
        ElementSurface *child_pattern_surface;
        int size = 0;
        Offset *pschildren = PATTERN_GET_CHILDREN(ps);

        for (i = 0; i < child_count_or_size; i++) {
            if (children[i].noun == r->nounSpecXaddr.noun) {
                noun = (NounSurface *) surface_for_xaddr(r, children[i]);
                child_pattern_surface = element_surface_for_xaddr(r, noun->specXaddr);
            } else if (children[i].noun == r->patternSpecXaddr.noun) {
                child_pattern_surface = element_surface_for_xaddr(r, children[i]);
            } else {
                raise_error("Unknown child element type %d\n", children[i].noun);
            }
            pschildren[i].noun.key = children[i].key;
            pschildren[i].noun.noun = children[i].noun;
            pschildren[i].offset = size;
            size += _pattern_get_size(child_pattern_surface);
        }
        pattern_set_size(ps, size);
        PATTERN_SET_CHILDREN_COUNT(ps, child_count_or_size);

    }
    return preop_new(r, newNoun, ps);
}

void proc_pattern_instance_new(Receptor *r) {
    PatternSpecData *d;
    stack_peek(r, PATTERN_SPEC_DATA_NOUN, (void **)&d);
    Xaddr pattern_xaddr = preop_new_pattern(r, d->label, d->child_count, d->children, d->process_count, d->processes);
    stack_push(r, XADDR_NOUN, &pattern_xaddr);
}

void dump_pattern_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *)surface;

    printf("Pattern\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    size: %d\n", (int) _pattern_get_size(ps));
    int count = PATTERN_GET_CHILDREN_COUNT(ps);
    printf("    %d children: ", count);
    dump_children_array(PATTERN_GET_CHILDREN(ps), count);
    printf("\n    %d processes: ", ps->process_count);
    dump_process_array((Process *)&ps->processes, ps->process_count);
    printf("\n");
}

void dump_pattern_value(Receptor *r, void *pattern_surface, void *surface) {
    ElementSurface *ps = (ElementSurface *)pattern_surface;
    Process *print_proc;
    print_proc = getProcess(ps, PRINT);
    if (print_proc) {
        (((LegacyProcess *)print_proc)->function)(r, surface);
    } else {
        hexDump("hexDump of surface", surface, _pattern_get_size(ps));
    }
}


void pattern_init(Receptor *r){
    Symbol newNoun = data_new_noun(r, r->cspecXaddr, "PATTERN");
    size_table_set(newNoun, (sizeFunction) pattern_get_spec_size);
    UntypedProcess processes = {INSTANCE_NEW, (voidVoidFn)proc_pattern_instance_new };
    ElementSurface specSurface = { newNoun, 1, processes};
    r->patternSpecXaddr = data_new(r, newNoun, &specSurface,  element_header_size((void *)&specSurface));
}
