#include "../ceptr.h"


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
