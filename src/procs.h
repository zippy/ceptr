#include "ceptr.h"


void proc_pattern_instance_new(Receptor *r) {
    PatternSpecData *d;
    stack_peek(r, PATTERN_SPEC_DATA_NOUN, &d);
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


void proc_cspec_instance_new(Receptor *r) {
    char label[BUFFER_SIZE];
    char ps[BUFFER_SIZE];
    stack_pop(r, CSTRING_NOUN, label);
    memset(ps, 0, BUFFER_SIZE);
    Process *p = 0;
    int processes = 0;
    Process pattern_processes[] = {
        {INSTANCE_SIZE, &proc_pattern_get_size },
        {INSTANCE_NEW, &proc_pattern_instance_new }
    };
    Process array_processes[] = {
        {INSTANCE_SIZE, &proc_array_get_size}
    };
    if (strcmp(label, "PATTERN") == 0) {
        p = pattern_processes;
        processes = 2;
    }
    if (strcmp(label, "ARRAY") == 0) {
        p = array_processes;
        processes = 1;
    }
    Symbol newNoun = preop_new_noun(r, r->rootXaddr, label);
    ((ElementSurface *) ps)->process_count = 0;
    add_processes((ElementSurface *) ps, processes, p);
    stack_push(r, newNoun, ps);
    op_new(r);
}
