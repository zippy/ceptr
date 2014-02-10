#include "../../src/ceptr.h"

void testStr255() {
    Receptor tr;init(&tr);Receptor *r = &tr;
    Symbol S = preop_new_noun(r, r->str255PatternSpecXaddr, "S");
    stack_push(r,S,"fish");
    op_new(r);
    Xaddr my_s;
    stack_pop(r,XADDR_NOUN,&my_s);
    char *s = surface_for_xaddr(r,my_s);
    spec_is_str_equal(s,"fish");
}
