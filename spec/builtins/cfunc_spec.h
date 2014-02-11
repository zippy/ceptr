#include "../../src/ceptr.h"

int func(Receptor *r){
    stack_push(r, CSTRING_NOUN, "fish");
}

void testCfunc() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol F = preop_new_noun(r, r->cfuncPatternSpecXaddr, "F");
    cfuncFn f = func;
    stack_push(r,F,&f);
    op_new(r);
    Xaddr my;
    stack_pop(r,XADDR_NOUN,&my);
    long *s = surface_for_xaddr(r,my);
    spec_is_long_equal(*s,func);
    op_invoke(r,my,RUN);
    char *peek_surface;
    stack_peek(r, CSTRING_NOUN, (void **)&peek_surface);
    spec_is_true( strcmp("fish", peek_surface) == 0);
}
