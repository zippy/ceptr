#include "../ceptr.h"

int proc_cfunc_print(Receptor *r, Symbol noun, void *this){
    printf("address of function: 0x%08x",*(long *)this);
    return 0;
}

typedef int (*cfuncFn)(Receptor *);

int proc_cfunc_run(Receptor *r, Symbol noun, void *this){
    return (*(cfuncFn *)this)(r);
}

void cfunc_init(Receptor *r) {

    Process processes[] = {
        {PRINT, (processFn)proc_cfunc_print},
	{RUN, (processFn)proc_cfunc_run}
    };

    PatternSpecData psd = {
        "CFUNC", sizeof(cfuncFn), 0, 2, processes
    };
    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    stack_pop(r, XADDR_NOUN, &r->cfuncPatternSpecXaddr);
}
