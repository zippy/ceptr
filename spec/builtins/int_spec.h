#include "../../src/ceptr.h"

void testInt() {
    Receptor tr; init(&tr); Receptor *r = &tr;

    spec_is_equal( r->intPatternSpecXaddr.noun, getSymbol(r, "INT") );
    spec_is_long_equal( (long)size_table_get(getSymbol(r, "INT")), (long)pattern_get_size );
    spec_is_equal( element_surface_for_xaddr(r, r->intPatternSpecXaddr)->process_count, 4 );


    // FIXME : should be on the stack
    Symbol MY_INT = preop_new_noun(r, r->intPatternSpecXaddr, "MY_INT");
    int val = 7;
    Xaddr my_int_xaddr = preop_new(r, MY_INT, &val);
    int *v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_equal(*v, 7);
    val = 8;
    preop_set(r, my_int_xaddr, &val);
    v = surface_for_xaddr(r, my_int_xaddr);
    spec_is_equal(*v, 8);


    //    testInc();
    //    testAdd();

}


//void testInc() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
//    int val = 7;
//    Xaddr age_xaddr = preop_new(r, AGE, &val);
//    preop_exec(r, age_xaddr, INC);
//    int *v = preop_get(r, age_xaddr);
//    spec_is_true(*v == 8);
//}
//
//void testAdd() {
//    Receptor tr;init(&tr);Receptor *r = &tr;
//    Symbol AGE = preop_new_noun(r, r->intPatternSpecXaddr, "Age");
//    int val = 7;
//    Xaddr age_xaddr = preop_new(r, AGE, &val);
//    val = 3;
//    preop_push_pattern(r, r->intPatternSpecXaddr.key, &val);
//    preop_exec(r, age_xaddr, ADD);
//    spec_is_true(*(int *) (&r->valStack[0]) == 10);
//}
