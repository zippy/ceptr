#include "ceptr.h"


void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = addr;

    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).

        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);

            // Output the offset.
            printf("  %04x ", i);
        }

        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);

        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e))
            buff[i % 16] = '.';
        else
            buff[i % 16] = pc[i];
        buff[(i % 16) + 1] = '\0';
    }
}

void dump_children_array(Offset *children, int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %d }(%d)", children[i].noun.key, children[i].noun.noun, children[i].offset);
        i++;
    }
}


void dump_process_array(Process *process, int count) {
    int i = 0;
    while (i < count) {
        if (i != 0) {
            printf(",");
        }
        printf("{ %d, %zu }", process[i].name, (size_t) process[i].function);
        i++;
    }
}

void dump_reps_spec(Receptor *r, void *surface) {
    ElementSurface *rs = (ElementSurface *)surface;

    printf("%s\n", "Array");
    printf("    name: %s(%d)\n", label_for_noun(r, rs->name), rs->name);
    int noun = REPS_GET_NOUN(rs);
    printf("    repsNoun: %s(%d) \n", label_for_noun(r, noun), noun);
    printf("\n");
}

void dump_spec_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *)surface;
    printf("Spec\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    %d processes: ", ps->process_count);
    dump_process_array((Process *)&ps->processes, ps->process_count);
    printf("\n");
}


void dump_pattern_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *)surface;

    printf("Pattern\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    size: %d\n", (int) pattern_get_size(ps));
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
        hexDump("hexDump of surface", surface, pattern_get_size(ps));
    }
}

void dump_array_value(Receptor *r, ElementSurface *rs, void *surface) {
    int count = *(int *) surface;
    surface += sizeof(int);
    Symbol arrayItemType;
    Symbol repsNoun = REPS_GET_NOUN(rs);
    ElementSurface *es = spec_surface_for_noun(r, &arrayItemType, repsNoun);
    Symbol typeTypeNoun = spec_noun_for_noun(r, arrayItemType);
    int size;
    if (typeTypeNoun == r->patternNoun) {
        printf("%s(%d) array of %d %s(%d)s\n", label_for_noun(r, rs->name), rs->name, count, label_for_noun(r, repsNoun), repsNoun);
        size = pattern_get_size(es);
        while (count > 0) {
            printf("    ");
            dump_pattern_value(r, es, surface);
            printf("\n");
            surface += size;
            count--;
        }
    }
    else if (typeTypeNoun == r->arrayNoun) {
        printf("array of %d %s(%d) arrays\n", count, label_for_noun(r, rs->name), rs->name);
        while (count > 0) {
            printf("    ");
            dump_array_value(r, es, surface);
            surface += size_of_named_surface(r, repsNoun, surface);
            count--;
        }
    }
}


void dump_noun(Receptor *r, NounSurface *ns) {
    printf("Noun      { %d, %5d } %s", ns->specXaddr.key, ns->specXaddr.noun, &ns->label);
}

void dump_xaddr(Receptor *r, Xaddr xaddr, int indent_level) {
    Symbol typeNoun = spec_noun_for_xaddr(r, xaddr);

    ElementSurface *es;
    NounSurface *ns;
    void *surface;
    int key = xaddr.key;
    int noun = xaddr.noun;
    if (noun == 0 && key == 16) {
        dump_spec_spec(r, &r->data.cache[key]);
    }
    else if (noun == r->nounNoun) {
        dump_noun(r, (NounSurface *) &r->data.cache[key]);
    } else if (typeNoun == CSPEC_NOUN) {
        dump_spec_spec(r, &r->data.cache[key]);

    } else if (typeNoun == r->patternNoun) {
        dump_pattern_spec(r, &r->data.cache[key]);

    } else if (typeNoun == r->arrayNoun) {
        dump_reps_spec(r, &r->data.cache[key]);

    } else {
        Symbol typeTypeNoun = spec_noun_for_noun(r, typeNoun);
	if (typeTypeNoun == r->nounNoun) {
	    ns = (NounSurface *) &r->data.cache[key];
	    dump_noun(r, ns);
	}
	else {
	    surface = surface_for_xaddr(r, xaddr_for_noun(r, noun));
	    ns = (NounSurface *) surface;
	    printf("%s : ", &ns->label);
	    es = element_surface_for_xaddr(r, ns->specXaddr);
	    if (typeTypeNoun == r->patternNoun) {
		dump_pattern_value(r, es, surface_for_xaddr(r, xaddr));
	    } else if (typeTypeNoun == r->arrayNoun) {
		dump_array_value(r, es, surface_for_xaddr(r, xaddr));
	    }
	}
    }
}

void dump_xaddrs(Receptor *r) {
    int i;
    NounSurface *ns;
    void *surface;
    for (i = 0; i <= r->data.current_xaddr; i++) {
        printf("Xaddr { %5d, %5d } - ", r->data.xaddrs[i].key, r->data.xaddrs[i].noun);
        dump_xaddr(r, r->data.xaddrs[i], 0);
        printf("\n");
    }
}
