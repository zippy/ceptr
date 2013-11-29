#include "ceptr.h"


void init_elements(Receptor *r) {

    r->rootXaddr.key = ROOT;
    r->rootXaddr.noun = ROOT;
    r->rootSurface.name = ROOT;
    r->rootSurface.process_count = 0;

    cspec_init(r);

    r->cspecXaddr.key = CSPEC;
    r->cspecXaddr.noun = CSPEC_NOUN;
    stack_push(r, CSTRING_NOUN, &"PATTERN");
    op_invoke(r, r->cspecXaddr, INSTANCE_NEW);
    stack_pop(r, XADDR_NOUN, &r->patternSpecXaddr);
    r->patternNoun = (element_surface_for_xaddr(r, r->patternSpecXaddr))->name;

    stack_push(r, CSTRING_NOUN, &"ARRAY");
    op_invoke(r, r->cspecXaddr, INSTANCE_NEW);
    stack_pop(r,XADDR_NOUN,&r->arraySpecXaddr);
    r->arrayNoun = (element_surface_for_xaddr(r, r->arraySpecXaddr))->name;

//    Symbol *_;
//    stack_pop_named_surface(r, _, &r->patternSpecXaddr);
//    r->patternSpecXaddr = cspec_proc_instance_new(r, "PATTERN");
//    r->arraySpecXaddr = cspec_proc_instance_new(r, "ARRAY");
}


void init_base_types(Receptor *r) {
    // ********************** bootstrap built in types

    // INT
    Process int_processes[] = {
        {PRINT, &proc_int_print},
        {INC, &proc_int_inc},
        {ADD, &proc_int_add}
    };

    PatternSpecData psd = {
        "INT", sizeof(int), 0, 3, int_processes
    };
    stack_push(r, PATTERN_SPEC_DATA_NOUN, &psd);
    op_invoke(r, r->patternSpecXaddr, INSTANCE_NEW);
    stack_pop(r, XADDR_NOUN, &r->intPatternSpecXaddr);

    // POINT
    Symbol X = preop_new_noun(r, r->intPatternSpecXaddr, "X");
    Symbol Y = preop_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process point_processes[] = {
        {PRINT, &proc_point_print}
    };

    // LINE
    Xaddr point_children[2] = {{X, NOUN_NOUN}, {Y, NOUN_NOUN}};
    r->pointPatternSpecXaddr = preop_new_pattern(r, "POINT", 2, point_children, 1, point_processes);

    Symbol A = preop_new_noun(r, r->pointPatternSpecXaddr, "A");
    Symbol B = preop_new_noun(r, r->pointPatternSpecXaddr, "B");


    Xaddr line_children[2] = {{A, NOUN_NOUN}, {B, NOUN_NOUN}};

    Process line_processes[] = {
        {PRINT, &proc_line_print}
    };


    r->linePatternSpecXaddr = preop_new_pattern(r, "LINE", 2, line_children, 1, line_processes);
}

void init(Receptor *r) {
    stack_init(r);
//    init_processing(r);
    data_init(r);
    init_elements(r);
    init_base_types(r);
}

