#include "../../src/ceptr.h"

void testReceptor(){
    Receptor tr; init(&tr); Receptor *r = &tr;

    spec_is_equal( r->receptorSpecXaddr.noun, getSymbol(r, "RECEPTOR") );
    spec_is_long_equal( (long)size_table_get(getSymbol(r, "RECEPTOR")), (long)receptor_get_size );
    spec_is_equal( element_surface_for_xaddr(r, r->receptorSpecXaddr)->process_count, 1 );

}