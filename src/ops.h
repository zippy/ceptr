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

//  This really should be in ops.h.
//  it's here until we implement fake surfaces for the builtin specs so that we don't refer to the procs by name in op_invoke.
void op_invoke(Receptor *r, Xaddr invokee, FunctionName function) {
    // record call on stack?
    if (invokee.key < 0) {
        raise_error("run op invoke for invokee %d", invokee.key);
    }
    ElementSurface *surface;
    Process *p;
    surface = element_surface_for_xaddr(r, invokee);
    p = getProcess(surface, function);
    if (p) {
        (((YagnArgsProcess *) p)->function)(r);
        return;
    }
    raise_error2("No function %d for key %d\n", function, invokee.key);
}

