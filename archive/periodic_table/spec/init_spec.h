#include "../src/ceptr.h"


void testInit() {
    Receptor tr;
    Receptor *r = &tr;

    stack_init(r);
    _data_init(&r->data);
    init_elements(r);

    noun_init(r);
    pattern_init(r);
    array_init(r);

    spec_is_equal( r->cspecXaddr.key, CSPEC );
    spec_is_equal( r->cspecXaddr.noun, CSPEC_NOUN );
    spec_is_equal( r->nounSpecXaddr.key, 2 );
    spec_is_equal( r->nounSpecXaddr.noun, 1 );
    spec_is_equal( getSymbol(r, "NOUN"), 1);
    spec_is_ptr_equal( size_table_get(getSymbol(r, "NOUN")), (void *)noun_get_size );
    spec_is_equal( element_surface_for_xaddr(r, r->nounSpecXaddr)->process_count, 1 );


    spec_is_equal( r->patternSpecXaddr.noun, getSymbol(r, "PATTERN"));
    spec_is_ptr_equal( size_table_get(getSymbol(r, "PATTERN")), (void *)pattern_get_spec_size );
    spec_is_equal( element_surface_for_xaddr(r, r->patternSpecXaddr)->process_count, 1 );


    spec_is_equal( r->arraySpecXaddr.noun, getSymbol(r, "ARRAY") );
    spec_is_ptr_equal( size_table_get(getSymbol(r, "ARRAY")), (void *)array_get_spec_size );
    spec_is_equal( element_surface_for_xaddr(r, r->arraySpecXaddr)->process_count, 1 );

}
