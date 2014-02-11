#include "../ceptr.h"


int proc_command_print(Receptor *r, Symbol noun, void *this){
    printf("COMMAND DUMP NOT IMPLEMENTED");
}

Xaddr command_init(Receptor *r) {

    Xaddr CMD_STR = new_noun(r, r->str255PatternSpecXaddr, "CMD_STR");
    Xaddr CMD_ALIAS_STR = new_noun(r, r->str255PatternSpecXaddr, "CMD_ALIAS_STR");

    Process processes[] = {
        {PRINT, (processFn)proc_command_print},
    };

    Xaddr children[2] = {CMD_STR,CMD_ALIAS_STR};
    PatternSpecData psd = {
        "COMMAND", 2, children, 1, processes
    };

    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    Xaddr x;
    stack_pop(r, XADDR_NOUN, &x);
    return x;
}
