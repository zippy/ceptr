/**
 * @file def_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/semtrex.h"
#include "../src/receptor.h"
#include "spec_utils.h"

void testDefGetName() {
    T *symbols = __sem_get_defs(G_sem,SEM_TYPE_SYMBOL,TEST_CONTEXT);

    spec_is_str_equal(__d_get_sem_name(symbols,TEST_INT_SYMBOL),"TEST_INT_SYMBOL");
}

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
    spec_is_str_equal(__d_get_sem_name(defs,ss),"shoe size");
    spec_is_str_equal(t2s(defs),"(SYMBOLS (SYMBOL_DEFINITION (SYMBOL_LABEL:shoe size) (SYMBOL_STRUCTURE:INTEGER)))");

    ss = _d_define_symbol(G_sem,NULL_STRUCTURE,"street number",ctx); // pre-declared to NULL

    spec_is_equal(ss.id,2);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DEFINITION (SYMBOL_LABEL:street number) (SYMBOL_STRUCTURE:NULL_STRUCTURE))");
    __d_set_symbol_structure(defs,ss,INTEGER);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DEFINITION (SYMBOL_LABEL:street number) (SYMBOL_STRUCTURE:INTEGER))");
    spec_is_equal(_d_get_def_addr(_t_child(defs,ss.id)),ss.id);

    _sem_free_context(G_sem,ctx);
    _t_free(d);
    //! [testDefSymbol]
}

void testDefStructure() {
    //! [testDefStructure]
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_STRUCTURE,TEST_CONTEXT);

    Structure st = _d_define_structure_v(G_sem,"boolean pair",TEST_CONTEXT,2,BOOLEAN,BOOLEAN);
    spec_is_equal(_t_children(defs),st.id);
    T *s = _t_child(defs,st.id);
    spec_is_sem_equal(_t_symbol(s),STRUCTURE_DEFINITION);
    spec_is_equal(_t_children(s),2);
    T *l = _t_child(s,DefLabelIdx);
    T *p = _t_child(s,2);
    spec_is_symbol_equal(0,_t_symbol(l),STRUCTURE_LABEL);
    spec_is_str_equal((char *)_t_surface(l),"boolean pair");
    spec_is_symbol_equal(0,_t_symbol(p),STRUCTURE_SEQUENCE);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_SYMBOL);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,2)),STRUCTURE_SYMBOL);
    spec_is_str_equal(__d_get_sem_name(defs,st),"boolean pair");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,st)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL:boolean pair) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:BOOLEAN) (STRUCTURE_SYMBOL:BOOLEAN)))");

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
    spec_is_long_equal(_d_get_symbol_size(G_sem,SYMBOL_LABEL,"shoe_size"),10);

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

void testCodeProcess() {
    //! [testCodeProcess]
    T *defs = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);
    T *code = _t_new_root(NOOP); // fake code
    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      "exponent",SIGNATURE_STRUCTURE,INTEGER,
                                      NULL);
    Process p = _d_define_process(G_sem,code,"power","takes the mathematical power of the two params",signature,TEST_CONTEXT);

    spec_is_true(is_process(p));
    spec_is_true(!is_symbol(p));
    spec_is_equal(_t_children(defs),p.id);
    T *s = _t_child(defs,p.id);

    spec_is_sem_equal(_t_symbol(s),PROCESS_DEFINITION);

    // first child is the name of the process
    T *name = _t_child(s,1);
    spec_is_sem_equal(_t_symbol(name),PROCESS_NAME);
    spec_is_str_equal((char *)_t_surface(name),"power");

    // second child is process intention as text (documentation)
    T *intention = _t_child(s,2);
    spec_is_sem_equal(_t_symbol(intention),PROCESS_INTENTION);
    spec_is_str_equal((char *)_t_surface(intention),"takes the mathematical power of the two params");

    // third child is code itself
    spec_is_ptr_equal(_t_child(s,3),code);

    // fourth child is the signature
    T *sig = _t_child(s,4);
    spec_is_sem_equal(_t_symbol(sig),PROCESS_SIGNATURE);

    spec_is_str_equal(__d_get_sem_name(defs,p),"power");

    //! [testCodeProcess]
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
    spec_is_str_equal(t2s(_sem_get_def(G_sem,STRUCTURE_DEF)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL:STRUCTURE_DEF) (STRUCTURE_OR (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SEQUENCE) (STRUCTURE_SYMBOL:STRUCTURE_OR) (STRUCTURE_SYMBOL:STRUCTURE_ZERO_OR_MORE) (STRUCTURE_SYMBOL:STRUCTURE_ONE_OR_MORE) (STRUCTURE_SYMBOL:STRUCTURE_ZERO_OR_ONE) (STRUCTURE_SYMBOL:SYMBOL_OF_STRUCTURE) (STRUCTURE_SYMBOL:STRUCTURE_ANYTHING)))");

    spec_is_str_equal(t2s(_sem_get_def(G_sem,CSTRING)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL:CSTRING) (STRUCTURE_SYMBOL:NULL_SYMBOL))");

    // an example of a defined process (IF)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,IF)),"(PROCESS_DEFINITION (PROCESS_NAME:IF) (PROCESS_INTENTION:if) (process:NULL_PROCESS) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:result) (SIGNATURE_PASSTHRU)) (INPUT_SIGNATURE (SIGNATURE_LABEL:condition) (SIGNATURE_PROCESS:BOOLEAN)) (INPUT_SIGNATURE (SIGNATURE_LABEL:then) (SIGNATURE_ANY)) (INPUT_SIGNATURE (SIGNATURE_LABEL:else) (SIGNATURE_ANY) (SIGNATURE_OPTIONAL))))");

    // an example of a process with code
    spec_is_str_equal(t2s(_sem_get_def(G_sem,respond_with_yup)),"(PROCESS_DEFINITION (PROCESS_NAME:respond_with_yup) (PROCESS_INTENTION:respond with yup) (process:RESPOND (SIGNAL_REF:/1/4) (YUP)) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:response id) (SIGNATURE_SYMBOL:SIGNAL_UUID))))");

    // an example of a defined protocol (alive)
    spec_is_str_equal(t2s(_sem_get_def(G_sem,ALIVE)),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL:ALIVE) (PROTOCOL_SEMANTICS (ROLE:SERVER) (ROLE:CLIENT) (GOAL:HANDLER)) (alive (EXPECT (ROLE:SERVER) (SOURCE (ROLE:CLIENT)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:PING))) (ACTION:respond_with_yup)) (EXPECT (ROLE:CLIENT) (SOURCE (ROLE:SERVER)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:YUP))) (GOAL:HANDLER))))");
}

void testDefReceptor() {
    T *d = __r_make_definitions();
    T *defs;

    SemanticID r = _d_define_receptor(G_sem,"streamscapes",d,TEST_CONTEXT);
    defs = _sem_get_defs(G_sem,r);
    spec_is_str_equal(t2s(defs),"(RECEPTORS (RECEPTOR_DEFINITION (RECEPTOR_LABEL:streamscapes) (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (RECEPTORS) (PROTOCOLS) (SCAPES))))");

    T *d2 = __sem_get_defs(G_sem,SEM_TYPE_RECEPTOR, _d_get_receptor_context(G_sem,r));

    spec_is_ptr_equal(d2,_t_child(d,SEM_TYPE_RECEPTOR));

    //@todo undefining things??
}

void testDef() {
    testDefGetName();
    testDefSysDefs();
    testDefValidate();
    testDefSymbol();
    testDefStructure();
    testGetSymbolStructure();
    testGetSize();
    testCodeProcess();
    testDefSemtrex();
    testDefReceptor();
}
