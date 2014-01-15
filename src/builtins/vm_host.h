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


void send_message(VMReceptor *r, Address dest_addr, void *surface, size_t length) {
    Receptor *dest_receptor = resolve(r, dest_addr);
    data_write_log( dest_receptor, surface, length);
}



void vm_host_init(VMReceptor *r){

    init(&r->base);
    init(&r->stdout);

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
