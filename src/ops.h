#include "ceptr.h"

// OPS. one level lower than PROCS which live on specs and are invoked there, ops are globally defined
//  operations that manipulate the stack and/or interface with the data engine in some way.


// given:  surface on stack
// result: new xaddr on stack
void op_new(Receptor *r) {
    Symbol noun;
    void *surface;

    stack_peek_unchecked(r, &noun, &surface);
    size_t current_index = r->data.cache_index;
    r->data.cache_index += size_of_named_surface(r, noun, surface);
    _record_existence(r, current_index, noun);
    Xaddr new_xaddr = {current_index, noun};
    preop_set(r, new_xaddr, surface);
    stack_push(r,XADDR_NOUN,&new_xaddr);
}


void op_invoke(Receptor *r, Xaddr invokee, FunctionName function);