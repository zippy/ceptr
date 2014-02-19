#include "ceptr.h"

// OPS. one level lower than PROCS which live on specs and are invoked there, ops are globally defined
//  operations that manipulate the stack and/or interface with the data engine in some way.

void *_pattern_get_child_surface(void *ps, void *surface,Symbol child_noun);

// given:  surface on stack
// result: new xaddr on stack
void op_new(Receptor *r) {
    Symbol noun;
    void *surface;
    stack_peek_unchecked(r, &noun, &surface);
    Xaddr new_xaddr = data_new(r, noun, surface, size_of_named_surface(r, noun, surface));
    //find scapes whose data source is the same noun as the xaddr being set:
    for(int i=1;i<=scape_count(r);i++) {
	Scape *s = get_scape(r,i);
	if (s->data_source == noun) {
	    Symbol nounType;
	    // TODO: This is bogus, not everything is a pattern!!
	    ElementSurface *ps = spec_surface_for_noun(r, &nounType, noun);
	    void *key_surface = _pattern_get_child_surface(ps, surface , s->key_source);
	    size_t key_len = size_of_named_surface(r,s->key_source,key_surface);
	    _add_scape_item(s,key_surface,key_len,new_xaddr);
	}
    }
    stack_push(r,XADDR_NOUN,&new_xaddr);
}

//TODO: fixme!! this is clearly bogus.  We need to differentiate between instances and specs ontologically
int is_spec(Receptor *r, Xaddr x) {
    return util_xaddr_eq(x,r->nounSpecXaddr) || util_xaddr_eq(x,r->patternSpecXaddr);
}

void op_invoke(Receptor *r, Xaddr invokee, FunctionName function) {
    // record call on stack?
    if (invokee.key < 0) {
        raise_error("run op invoke for invokee %d", invokee.key);
    }

    ElementSurface *surface = element_surface_for_xaddr(r, invokee);
    ElementSurface *spec_surface;
    Process *p;
    if (is_spec(r,invokee)) {
	spec_surface = surface;
    }
    else {
	Xaddr spec = spec_xaddr_for_xaddr(r,invokee);
	spec_surface = element_surface_for_xaddr(r, spec);
    }
    p = getProcess(spec_surface, function);

    if (p) {
        ((p)->function)(r,invokee.noun,surface,spec_surface);
        return;
    }
    raise_error2("No function %d for key %d\n", function, invokee.key);

}
