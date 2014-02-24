#include "../../src/ceptr.h"

void testInt() {
    Receptor tr; init(&tr); Receptor *r = &tr;

    spec_is_equal( r->intPatternSpecXaddr.noun, getSymbol(r, "INT") );
    spec_is_long_equal( (long)size_table_get(getSymbol(r, "INT")), (long)pattern_get_size );
    spec_is_equal( element_surface_for_xaddr(r, r->intPatternSpecXaddr)->process_count, 4 );

    // FIXME : should be on the stack
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    int val = 7;
    Xaddr my_int_xaddr; // = preop_new(r, MY_INT, &val);
    stack_push(r,MY_INT,&val);
    op_new(r);
    stack_pop(r,XADDR_NOUN,&my_int_xaddr);

    int *v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_equal(*v, 7);
    val = 8;
    preop_set(r, my_int_xaddr, &val);
    v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_equal(*v, 8);

    op_invoke(r, my_int_xaddr, INC);
    spec_is_equal(*v, 9);

    val = 99;
    stack_push(r,MY_INT,&val);
    op_invoke(r, my_int_xaddr, ADD);
    stack_pop(r, MY_INT, &val);
    spec_is_equal(val, 108);
}
