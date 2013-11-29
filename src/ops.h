#include "ceptr.h"

// OPS. one level lower than PROCS which live on specs and are invoked there, ops are globally defined
//  operations that manipulate the stack and/or interface with the data engine in some way.


// given:  surface on stack
// result: new xaddr on stack
void op_new(Receptor *r) {
    Symbol noun;
    void *surface;
    stack_peek_unchecked(r, &noun, &surface);
    Xaddr new_xaddr = data_new(r, noun, surface, size_of_named_surface(r, noun, surface));
    stack_push(r,XADDR_NOUN,&new_xaddr);
}


void op_invoke(Receptor *r, Xaddr invokee, FunctionName function);