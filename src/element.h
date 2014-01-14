#include "ceptr.h"

#define ELEMENT_HEADER_SIZE(h) (sizeof(ElementSurface)-sizeof(Process)+(((ElementSurface *)h)->process_count)*sizeof(Process))
#define SKIP_ELEM_HEADER(h) (ELEMENT_HEADER_SIZE(h) + (void *)h)


size_t element_header_size(void *h) {
    return sizeof(ElementSurface) - sizeof(Process) + (((ElementSurface *) h)->process_count) * sizeof(Process);
}

void * skip_elem_header(void *element_surface) {
    return element_header_size(element_surface) + (void *) element_surface;
}

Process *getProcess(ElementSurface *es, FunctionName name) {
    int i = es->process_count;
    if (i <= 0){
        raise_error2("request for process %d on element with %d process_count\n", name, i);
    }
    Process *p = (Process *)&es->processes;
    while (i--) {
        if (p->name == name) {
            return p;
        }
        p++;
    }
    return 0;
}


void add_processes(ElementSurface *dest_surface, int process_count, Process *source_p) {
//    printf("add_processes count %d\n", process_count);
    Process *dest_p = (Process *)&dest_surface->processes;
    dest_p += dest_surface->process_count;
    dest_surface->process_count += process_count;
    while (process_count-- > 0) {
        dest_p->name = source_p->name;
        dest_p->function = source_p->function;
        source_p++;
        dest_p++;
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

int init_element(Receptor *r, char *label, Xaddr element_spec, ElementSurface *es, int processCount, Process *processes) {
    es->name = preop_new_noun(r, element_spec, label);
    ElementSurface *spec_surface = element_surface_for_xaddr(r,element_spec);

    es->processes = spec_surface->processes;
    es->process_count = 1;
    add_processes(es, processCount, processes);
    return es->name;
}


