#include "../ceptr.h"

void cspec_proc_instance_new(Receptor *r) {
printf("cspec_proc_instance_new \n");
    char label[BUFFER_SIZE];
    char ps[BUFFER_SIZE];
    sizeFunction size_function;
    stack_pop(r, CSTRING_NOUN, label);
    memset(ps, 0, BUFFER_SIZE);
    Process *p = 0;
    int processes = 0;
    Process noun_processes[] = {
        {INSTANCE_NEW, &proc_noun_instance_new }
    };
    Process pattern_processes[] = {
        {INSTANCE_NEW, &proc_pattern_instance_new }
    };
    Process array_processes[] = {
    };
    if (strcmp(label, "NOUN") == 0) {
        p = noun_processes;
        processes = 1;
        size_function = noun_get_spec_size;
    }
    if (strcmp(label, "PATTERN") == 0) {
        p = pattern_processes;
        processes = 1;
        size_function = pattern_get_spec_size;
    }
    if (strcmp(label, "ARRAY") == 0) {
        p = array_processes;
        processes = 0;
        size_function = array_get_spec_size;
    }
    Symbol newNoun = preop_new_noun(r, r->cspecXaddr, label);
    ((ElementSurface *) ps)->process_count = 0;
    ((ElementSurface *) ps)->name = newNoun;
    add_processes((ElementSurface *) ps, processes, p);
    size_table_set(newNoun, size_function);
    stack_push(r, newNoun, ps);
    if(size_function == noun_get_spec_size) {
        Symbol noun;
        void *surface;
        stack_peek_unchecked(r, &noun, &surface);
        Xaddr new_xaddr = data_new(r, noun, surface, noun_get_spec_size(r, noun, surface));
        stack_push(r,XADDR_NOUN,&new_xaddr);
    } else {
        op_new(r);
    }
}

//int cspec_proc_instance_size(Receptor *r) {
//    return element_header_size(surface);
//}

void cspec_init(Receptor *r) {
//    Symbol noun = data_new_noun(r, r->rootXaddr, "CSPEC");
//    dump_xaddrs(r);
//    ElementSurface tmp_cspec = {noun, 1, {}};
//    size_t size = element_header_size(&tmp_cspec);
//    ElementSurface *cspec = (ElementSurface *)data_new_uninitialized(r, &r->cspecXaddr, noun, size);
//
//    cspec->processes.name = INSTANCE_NEW;
//    cspec->processes.function = &cspec_proc_instance_new;

//    cspec->processes[1].name = INSTANCE_SIZE;
//    cspec->processes[1].function = &cspec_proc_instance_size;
}
