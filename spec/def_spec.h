/**
 * @file def_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/semtrex.h"
#include "../src/receptor.h"
#include "spec_utils.h"

void testDefValidate() {
    T *symbols = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,TEST_CONTEXT);
    T *structures = __sem_get_defs(G_sem,SEM_TYPE_STRUCTURE,TEST_CONTEXT);

    Symbol bad_symbol = {1,SEM_TYPE_SYMBOL,99};
    Structure bad_structure = {2,SEM_TYPE_STRUCTURE,22};

    // commented out because for now they raise errors...
    //        __d_validate_structure(G_sem,bad_structure,"test");
    //    __d_validate_symbol(G_sem,bad_symbol,"test");
}

void testDefSymbol() {
    //! [testDefSymbol]
    T *d = __r_make_definitions();
    int ctx = _sem_new_context(G_sem,d);
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,ctx);

    Symbol ss = _d_define_symbol(G_sem,INTEGER,"shoe size",ctx);
    T *def = _sem_get_def(G_sem,ss);

    spec_is_equal(_d_get_def_addr(def),1);
    spec_is_true(is_symbol(ss));
    spec_is_equal(_t_children(defs),ss.id);
    spec_is_ptr_equal(_t_child(defs,ss.id),def);
    spec_is_sem_equal(_t_symbol(_t_child(defs,ss.id)),SYMBOL_DEFINITION);
    spec_is_sem_equal(_t_symbol(_t_child(_t_child(defs,ss.id),SymbolDefStructureIdx)),SYMBOL_STRUCTURE);
    spec_is_sem_equal(_t_symbol(_t_child(_t_child(defs,ss.id),DefLabelIdx)),SYMBOL_LABEL);
    spec_is_str_equal(t2s(defs),"(SYMBOLS (SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:shoe size)) (SYMBOL_STRUCTURE:INTEGER)))");

    ss = _d_define_symbol(G_sem,NULL_STRUCTURE,"street number",ctx); // pre-declared to NULL

    spec_is_equal(ss.id,2);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:street number)) (SYMBOL_STRUCTURE:NULL_STRUCTURE))");
    __d_set_symbol_structure(defs,ss,INTEGER);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:street number)) (SYMBOL_STRUCTURE:INTEGER))");
    spec_is_equal(_d_get_def_addr(_t_child(defs,ss.id)),ss.id);

    _sem_free_context(G_sem,ctx);
    _t_free(d);
    //! [testDefSymbol]
}

void testDefStructure() {
    //! [testDefStructure]
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_STRUCTURE,TEST_CONTEXT);

    Structure st = _d_define_structure_v(G_sem,"boolean pair",TEST_CONTEXT,2,BOOLEAN,BOOLEAN);

    spec_is_str_equal(t2s(_sem_get_def(G_sem,st)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:boolean pair)) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:BOOLEAN) (STRUCTURE_SYMBOL:BOOLEAN)))");

    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    T *symbols = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,SYS_CONTEXT);

    // test a few built-in symbols
    spec_is_structure_equal(0,__d_get_symbol_structure(symbols,STRUCTURE_DEFINITION),TUPLE_OF_STRUCTURE_LABEL_AND_STRUCTURE_DEF);
    spec_is_structure_equal(0,__d_get_symbol_structure(symbols,STRUCTURE_SYMBOL),SYMBOL);

    // test user-defined symbols
    symbols = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,TEST_CONTEXT);

    Symbol s = _d_define_symbol(G_sem,INTEGER,"shoe size",TEST_CONTEXT);
    spec_is_sem_equal(__d_get_symbol_structure(symbols,s),INTEGER);
    //! [testSymbolStructure]
}

void testGetSize() {
    //! [testGetSize]

    // test a few built-in symbols and structures
    spec_is_long_equal(_d_get_structure_size(G_sem,INTEGER,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(G_sem,BIT,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(G_sem,FLOAT,0),sizeof(float));
    spec_is_long_equal(_d_get_structure_size(G_sem,XADDR,0),sizeof(Xaddr));
    spec_is_long_equal(_d_get_symbol_size(G_sem,STRUCTURE_SYMBOL,0),sizeof(Symbol));
    spec_is_long_equal(_d_get_symbol_size(G_sem,ENGLISH_LABEL,"shoe_size"),10);

    // test user-defined symbols and structures
    T *symbols = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,TEST_CONTEXT);
    T *structures = __sem_get_defs(G_sem,SEM_TYPE_STRUCTURE,TEST_CONTEXT);

    Symbol lat = _d_define_symbol(G_sem,FLOAT,"latitude",TEST_CONTEXT);      // symbols are declared, structures are defined
    Symbol lon = _d_define_symbol(G_sem,FLOAT,"longitude",TEST_CONTEXT);     // here we declare two meaningful ways to use the structure float


    // Two symbols, lat and lon are assembled into the structure "latlong"
    Structure latlong = _d_define_structure_v(G_sem,"latlong",TEST_CONTEXT, 2, lat, lon);

    // House location is a meaningful use of the structure latlong
    Symbol house_loc = _d_define_symbol(G_sem,latlong,"house location",TEST_CONTEXT);

    // Here's the surface of the latlong.
    float ll[] = {2.0,90.3};
    spec_is_long_equal(_d_get_symbol_size(G_sem,house_loc,ll),sizeof(ll));
    spec_is_long_equal(_d_get_structure_size(G_sem,latlong,ll),sizeof(ll));

    //structure sizing when structure is a singleton
    Structure latstruct = _d_define_structure_v(G_sem,"latstruct",TEST_CONTEXT, 1, lat);
    float ls[] = {2.0};
    spec_is_long_equal(_d_get_structure_size(G_sem,latstruct,ll),sizeof(ls));

    //@todo what about sizing with optional structural elements...

    //! [testGetSize]

    // In the diagram on 20140806_102233.jpg, "Label" is the symbol, "Struct" is the structure.
    // Use t_dump to display textually the resulting tree structure.  Use _td for a T in the context of a given receptor.
}

void testDefProcess() {
    //! [testDefProcess]
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);
    T *code = _t_new_root(NOOP); // fake code
    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      "exponent",SIGNATURE_STRUCTURE,INTEGER,
                                      NULL);
    T *link = _t_build(G_sem,0,PROCESS_LINK,
                       PROCESS_OF_STRUCTURE,INTEGER,PROCESS_TYPE,OPERATOR,
                       NULL_SYMBOL,NULL_SYMBOL
                       );
    Process p = _d_define_process(G_sem,code,"power","takes the mathematical power of the two params",signature,link,TEST_CONTEXT);

    spec_is_true(is_process(p));
    spec_is_true(!is_symbol(p));
    spec_is_equal(_t_children(defs),p.id);
    T *s = _t_child(defs,p.id);

    spec_is_str_equal(t2s(s),"(PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:power)) (PROCESS_INTENTION:takes the mathematical power of the two params) (process:NOOP) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:result)) (SIGNATURE_SYMBOL:NULL_SYMBOL)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:val)) (SIGNATURE_STRUCTURE:INTEGER)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:exponent)) (SIGNATURE_STRUCTURE:INTEGER))) (PROCESS_LINK (PROCESS_OF_STRUCTURE:INTEGER) (PROCESS_TYPE (OPERATOR))))");

    //! [testDefProcess]
}
void testDefProcessTemplate() {
    //! [testDefProcessTemplate]
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);
    T *code = _t_new_root(NOOP); // some code with a template
    T *t = _t_newr(code,SLOT);
    _t_news(t,GOAL,RESPONSE_HANDLER);
    t = _t_newr(code,SLOT);
    _t_news(t,USAGE,REQUEST_TYPE);
    _t_news(t,SLOT_IS_VALUE_OF,TEST_INT_SYMBOL);
    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,NULL);
    Process p = _d_define_process(G_sem,code,"test_template_proc","long desc..",signature,NULL,TEST_CONTEXT);
    T *def = _sem_get_def(G_sem,p);

    T *sig = _t_child(def,ProcessDefSignatureIdx);
    spec_is_str_equal(t2s(_t_child(sig,2)),"(TEMPLATE_SIGNATURE (EXPECTED_SLOT (GOAL:RESPONSE_HANDLER)) (EXPECTED_SLOT (USAGE:REQUEST_TYPE) (SLOT_IS_VALUE_OF:TEST_INT_SYMBOL)))");

    //! [testDefProcessTemplate]
}



void testDefSemtrex() {
    //! [testDefSemtrex]
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    Symbol SY(r,lat,FLOAT);
    Symbol SY(r,lon,FLOAT);
    Structure ST(r,latlong,2,lat,lon);
    Symbol SY(r,house_loc,latlong);

    T *stx = _d_build_def_semtrex(r->sem,house_loc,0);
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(r->sem,stx,buf),"/house_loc/(lat,lon)");
    __t_dump(r->sem,stx,0,buf);
    spec_is_str_equal(buf,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:house_loc) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lat)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lon))))");

    _t_free(stx);
    _r_free(r);
    //! [testDefSemtrex]
}

void testDefSysDefs() {
    SemanticID vmhr = {SYS_CONTEXT,SEM_TYPE_RECEPTOR,0};
    spec_is_sem_equal(vmhr,SYS_RECEPTOR);

    spec_is_equal(G_sem->contexts,_NUM_DEFAULT_CONTEXTS);
    spec_is_equal(_t_children(G_sem->stores[0].definitions),6);  // should have slots for all the defs
    spec_is_equal(_t_children(_t_child(G_sem->stores[0].definitions,SEM_TYPE_STRUCTURE)),NUM_SYS_STRUCTURES-1);
    spec_is_equal(_t_children(_t_child(G_sem->stores[0].definitions,SEM_TYPE_SYMBOL)),NUM_SYS_SYMBOLS-1);

    dump2json(G_sem,__sem_context(G_sem,SYS_CONTEXT)->definitions,"sysdefs");

    // an example of a defined structure (STRUCTURE_DEF)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,STRUCTURE_DEF)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:STRUCTURE_DEF)) (STRUCTURE_OR (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SEQUENCE) (STRUCTURE_SYMBOL:STRUCTURE_OR) (STRUCTURE_SYMBOL:STRUCTURE_ZERO_OR_MORE) (STRUCTURE_SYMBOL:STRUCTURE_ONE_OR_MORE) (STRUCTURE_SYMBOL:STRUCTURE_ZERO_OR_ONE) (STRUCTURE_SYMBOL:STRUCTURE_STRUCTURE) (STRUCTURE_SYMBOL:STRUCTURE_ANYTHING)))");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,CSTRING)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:CSTRING)) (STRUCTURE_SYMBOL:NULL_SYMBOL))");

    // an example of a defined process (IF)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,IF)),"(PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:IF)) (PROCESS_INTENTION:if) (process:NULL_PROCESS) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:result)) (SIGNATURE_PASSTHRU)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:condition)) (SIGNATURE_PROCESS:BOOLEAN)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:then)) (SIGNATURE_ANY)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:else)) (SIGNATURE_ANY) (SIGNATURE_OPTIONAL))))");

    // an example of a defined linked process (LT_INT)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,LT_INT)),"(PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:LT_INT)) (PROCESS_INTENTION:test less than) (process:NULL_PROCESS) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:result)) (SIGNATURE_SYMBOL:BOOLEAN)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:int1)) (SIGNATURE_STRUCTURE:INTEGER)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:int2)) (SIGNATURE_STRUCTURE:INTEGER))) (PROCESS_LINK (PROCESS_OF_STRUCTURE:INTEGER) (PROCESS_TYPE (OPERATOR))))");

    // an example of a process with code
    spec_is_str_equal(t2s(_sem_get_def(G_sem,respond_with_yup)),"(PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:respond_with_yup)) (PROCESS_INTENTION:respond with yup) (process:RESPOND (SIGNAL_REF:/2/1/4) (YUP)) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:response id)) (SIGNATURE_SYMBOL:SIGNAL_UUID))))");

    // an example of a defined protocol (alive)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,ALIVE)),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL (ENGLISH_LABEL:ALIVE)) (PROTOCOL_SEMANTICS (ROLE:SERVER) (ROLE:CLIENT) (GOAL:HANDLER)) (alive (EXPECT (ROLE:SERVER) (SOURCE (ROLE:CLIENT)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:PING))) (ACTION:respond_with_yup)) (EXPECT (ROLE:CLIENT) (SOURCE (ROLE:SERVER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:YUP))) (SLOT (GOAL:HANDLER) (SLOT_IS_VALUE_OF:ACTION)))))");

    // an example of an a extra label:
    spec_is_str_equal(t2s(_sem_get_def(G_sem,CONTENT_TYPE)),"(SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:CONTENT_TYPE) (HTTP_HEADER_LABEL:Content-Type)) (SYMBOL_STRUCTURE:MEDIA_TYPE))");

}

void testDefReceptor() {
    T *d = __r_make_definitions();
    T *defs;

    SemanticID s = _d_define_receptor(G_sem,"streamscapes",d,TEST_CONTEXT);
    defs = _sem_get_defs(G_sem,s);
    spec_is_str_equal(t2s(defs),"(RECEPTORS (RECEPTOR_DEFINITION (RECEPTOR_LABEL (ENGLISH_LABEL:streamscapes)) (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (RECEPTORS) (PROTOCOLS) (SCAPES))))");

    T *d2 = __sem_get_defs(G_sem,SEM_TYPE_RECEPTOR, _d_get_receptor_context(G_sem,s));

    spec_is_ptr_equal(d2,_t_child(d,SEM_TYPE_RECEPTOR));

    Receptor *r = _r_new(G_sem,s);
    char *n = _sem_get_name(r->sem,s);
    spec_is_str_equal(n,"streamscapes");
    _r_free(r);
    //@todo undefining things??
}


void testDef() {
    testDefSysDefs();
    testDefValidate();
    testDefSymbol();
    testDefStructure();
    testGetSymbolStructure();
    testGetSize();
    testDefProcess();
    testDefProcessTemplate();
    testDefSemtrex();
    testDefReceptor();
}
