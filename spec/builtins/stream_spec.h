//#include "../../src/ceptr.h"
//
//
//void proc_code_stream_print(Receptor *r, Symbol name, ElementSurface *es) {
//
//}
//
//void testStream() {
//    Receptor tr; init(&tr); Receptor *r = &tr;
//
//    spec_is_equal( r->streamSpecXaddr.noun, getSymbol(r, "STREAM") );
//    spec_is_long_equal( (long)size_table_get(getSymbol(r, "STREAM")), (long)stream_get_size );
//    spec_is_equal( element_surface_for_xaddr(r, r->streamSpecXaddr)->process_count, 1 );
//
//    // make a "code" type of stream
//
////    preop_new_stream(r, "CODE_STREAM", 1)
////    Symbol newNoun = data_new_noun(r, r->streamSpecXaddr, "CODE_STREAM");
////
////    Process int_processes[] = {
////        {PRINT, (processFn)proc_code_stream_print}
////    };
//
//
//
//
//}