#include "../ceptr.h"


enum { STDOUT };

typedef int Address;

typedef struct {
    Receptor base;
    Receptor stdout;
} VMReceptor;


Receptor *resolve(VMReceptor *r, Address addr) {
    if (addr == STDOUT) {
        return &r->stdout;
    }
    raise_error("whatchu talking about addred %d\n", addr );
    return 0;
}

LogProc getLogProc(Receptor *r) {
    return r->logProc;
}


void data_write_log(Receptor *r, Symbol noun, void *surface, size_t length) {
    memcpy( r->data.lastLogEntry.content, surface, length);
    r->data.lastLogEntry.noun = noun;
}

typedef struct {
    Address destination;
    Xaddr value;
} Packet;

void wakeup(Receptor *r){
    (*getLogProc((void*)r))((void *)r);
}

void _send_message(VMReceptor *r, Packet *p, void *surface, size_t size) {
    Receptor *dest_receptor = resolve(r, p->destination);
    data_write_log( dest_receptor, p->value.noun, surface, size);
    wakeup(dest_receptor);
}


void send_message(Receptor *r, Packet *p) {
    void *surface = surface_for_xaddr(r, p->value);
    size_t size = size_of_named_surface(r, p->value.noun, surface);
    _send_message(r->parent, p, surface, size);
}

void dump_spec_spec(Receptor *r, void *surface) {
    ElementSurface *ps = (ElementSurface *) surface;
    printf("Spec\n");
    printf("    name: %s(%d)\n", label_for_noun(r, ps->name), ps->name);
    printf("    %d processes: ", ps->process_count);
    dump_process_array((Process *) &ps->processes, ps->process_count);
    printf("\n");
}

void dump_named_surface(Receptor *r, Symbol noun, void *surface) {
    if (spec_noun_for_noun(r, noun) == r->patternSpecXaddr.noun) {
        dump_pattern_spec(r, surface);
    } else {
        dump_xaddrs(r);
        raise_error("dunno how to dump named surface %d\n", noun);
    }
}


void stdout_log_proc(void *r_ptr) {
    Receptor *r = (Receptor *) r_ptr;
    dump_named_surface(r, r->data.lastLogEntry.noun, &r->data.lastLogEntry.content );
}


void vm_host_init(VMReceptor *r){

    init(&r->base);
    init(&r->stdout);
    r->stdout.logProc = &stdout_log_proc;
    // add "STDIN", "STDOUT" receptors to Vmhost


//
//    repl {
//        Code c;
//        Instruction *i = &c;
//        while(true) {
//            switch(i.op) {
//                case LISTEN:
//                case INVOKE:
//                case PUSH:
//                case POP:
//            }
//        }
//    }

}
