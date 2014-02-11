#include "../ceptr.h"


int proc_int_inc(Receptor *r, Symbol noun, void *this) {
    ++*(int *) (this);
    return 0;
}

int proc_int_add(Receptor *r, Symbol noun, void *this) {
    int *i;
    stack_peek(r, noun, (void **)&i);
    *i += *(int *)this;
    return 0;
}

int proc_int_print(Receptor *r, Symbol noun, void *this) {
    printf("%d", *(int *) this);
    return 0;
}

void int_init(Receptor *r) {
    // INT
    Process int_processes[] = {
        {PRINT, (processFn)proc_int_print},
        {INC, (processFn)proc_int_inc},
        {ADD, (processFn)proc_int_add}
    };

    PatternSpecData psd = {
        "INT", sizeof(int), 0, 3, int_processes
    };
    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    stack_pop(r, XADDR_NOUN, &r->intPatternSpecXaddr);
}
