#include "../ceptr.h"

int proc_str255_print(Receptor *r, Symbol noun, void *this){
    printf("String is: %s",this);
}

void str255_init(Receptor *r) {
    //TODO: make a helper function like this:    _new_pattern("STR255",256,0,1,PRINT,proc_str255_print);
    Process str_255_processes[] = {
        {PRINT, (processFn)proc_str255_print},
    };

    PatternSpecData psd = {
        "STR255", 256, 0, 1, str_255_processes
    };
    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    stack_pop(r, XADDR_NOUN, &r->str255PatternSpecXaddr);
}
