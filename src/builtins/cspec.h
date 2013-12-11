#include "../ceptr.h"


void cspec_proc_instance_new(Receptor *r) {
printf("cspec_proc_instance_new \n");
    char label[BUFFER_SIZE];
    char ps[BUFFER_SIZE];
    stack_pop(r, CSTRING_NOUN, label);
    memset(ps, 0, BUFFER_SIZE);
    Process *p = 0;
    int processes = 0;
    Process noun_processes[] = {
        {INSTANCE_SIZE, &proc_noun_get_size },
        {INSTANCE_NEW, &proc_noun_instance_new }
    };
    Process pattern_processes[] = {
        {INSTANCE_SIZE, &proc_pattern_get_size },
        {INSTANCE_NEW, &proc_pattern_instance_new }
    };
    Process array_processes[] = {
        {INSTANCE_SIZE, &proc_array_get_size}
    };
    if (strcmp(label, "NOUN") == 0) {
        p = noun_processes;
        processes = 2;
    }
    if (strcmp(label, "PATTERN") == 0) {
        p = pattern_processes;
        processes = 2;
    }
    if (strcmp(label, "ARRAY") == 0) {
        p = array_processes;
        processes = 1;
    }
    Symbol newNoun = preop_new_noun(r, r->cspecXaddr, label);
    ((ElementSurface *) ps)->process_count = 0;
    ((ElementSurface *) ps)->name = newNoun;
    add_processes((ElementSurface *) ps, processes, p);
    stack_push(r, newNoun, ps);
    op_new(r);
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
