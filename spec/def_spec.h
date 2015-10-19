/**
 * @file def_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/def.h"
#include "../src/semtrex.h"

void testSymbolGetName() {
    //! [testSymbolGetName]
    spec_is_str_equal(_d_get_symbol_name(0,TEST_INT_SYMBOL),"TEST_INT_SYMBOL");
    spec_is_str_equal(_d_get_symbol_name(0,NULL_SYMBOL),"NULL_SYMBOL");
    spec_is_str_equal(_d_get_symbol_name(0,DEFINITIONS),"DEFINITIONS");
    //! [testSymbolGetName]
}

void testStructureGetName() {
    //! [testStructureGetName]
    spec_is_str_equal(_d_get_structure_name(0,INTEGER),"INTEGER");
    spec_is_str_equal(_d_get_structure_name(0,NULL_STRUCTURE),"NULL_STRUCTURE");
    //! [testStructureGetName]
}

void testProcessGetName() {
    //! [testProcessGetName]
    spec_is_str_equal(_d_get_process_name(0,IF),"IF");
    spec_is_str_equal(_d_get_process_name(0,NULL_PROCESS),"NULL_PROCESS");
    //! [testProcessGetName]
}

void testDefValidate() {
    T *symbols = _t_new_root(SYMBOLS);
    Symbol bad_symbol = {1,SEM_TYPE_SYMBOL,99};
    T *structures = _t_new_root(STRUCTURES);
    Structure bad_structure = {2,SEM_TYPE_STRUCTURE,22};

    // commented out because for now they raise errors...
    //    __d_validate_structure(structures,bad_structure,"test");
    //    __d_validate_symbol(symbols,bad_symbol,"test");
    _t_free(symbols);
    _t_free(structures);
}

void testDefSymbol() {
    //! [testDefSymbol]
    T *defs = _t_new_root(SYMBOLS);

    T *def = __d_declare_symbol(defs,INTEGER,"shoe size");
    Symbol ss = {RECEPTOR_CONTEXT,SEM_TYPE_SYMBOL,1};

    spec_is_true(is_symbol(ss));
    spec_is_equal(_t_children(defs),ss.id);
    spec_is_ptr_equal(_t_child(defs,ss.id),def);
    spec_is_sem_equal(_t_symbol(_t_child(defs,ss.id)),SYMBOL_DECLARATION);
    spec_is_sem_equal(_t_symbol(_t_child(_t_child(defs,ss.id),2)),SYMBOL_STRUCTURE);
    spec_is_sem_equal(_t_symbol(_t_child(_t_child(defs,ss.id),1)),SYMBOL_LABEL);
    spec_is_str_equal(_d_get_symbol_name(defs,ss),"shoe size");
    spec_is_str_equal(t2s(defs),"(SYMBOLS (SYMBOL_DECLARATION (SYMBOL_LABEL:shoe size) (SYMBOL_STRUCTURE:INTEGER)))");

    ss = _d_declare_symbol(defs,0,NULL_STRUCTURE,"street number",RECEPTOR_CONTEXT); // pre-declared to NULL
    spec_is_equal(ss.id,2);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DECLARATION (SYMBOL_LABEL:street number) (SYMBOL_STRUCTURE:NULL_STRUCTURE))");
    __d_set_symbol_structure(defs,ss,INTEGER);
    spec_is_str_equal(t2s(_t_child(defs,ss.id)),"(SYMBOL_DECLARATION (SYMBOL_LABEL:street number) (SYMBOL_STRUCTURE:INTEGER))");

    _t_free(defs);
    //! [testDefSymbol]
}

void testDefStructure() {
    //! [testDefStructure]
    T *defs = _t_new_root(STRUCTURES);

    Structure st = _d_define_structure(0,defs,"boolean pair",RECEPTOR_CONTEXT,2,BOOLEAN,BOOLEAN);
    spec_is_equal(_t_children(defs),st.id);
    T *s = _t_child(defs,st.id);
    spec_is_sem_equal(_t_symbol(s),STRUCTURE_DEFINITION);
    spec_is_equal(_t_children(s),2);
    T *l = _t_child(s,1);
    T *p = _t_child(s,2);
    spec_is_symbol_equal(0,_t_symbol(l),STRUCTURE_LABEL);
    spec_is_str_equal((char *)_t_surface(l),"boolean pair");
    spec_is_symbol_equal(0,_t_symbol(p),STRUCTURE_SEQUENCE);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_SYMBOL);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,2)),STRUCTURE_SYMBOL);
    Structure x = {RECEPTOR_CONTEXT,SEM_TYPE_STRUCTURE,1};
    spec_is_str_equal(_d_get_structure_name(defs,x),"boolean pair");

    spec_is_str_equal(t2s(defs),"(STRUCTURES (STRUCTURE_DEFINITION (STRUCTURE_LABEL:boolean pair) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:BOOLEAN) (STRUCTURE_SYMBOL:BOOLEAN))))");

    _t_free(defs);
    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    // test a few built-in symbols
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_DEFINITION),TUPLE_OF_STRUCTURE_LABEL_AND_STRUCTURE_DEF);
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_SYMBOL),SYMBOL);

    // test user-defined symbols
    T *symbols = _t_new_root(SYMBOLS);
    T *structures = _t_new_root(STRUCTURES);

    Symbol s = _d_declare_symbol(symbols,structures,INTEGER,"shoe size",RECEPTOR_CONTEXT);
    spec_is_sem_equal(_d_get_symbol_structure(symbols,s),INTEGER);
    _t_free(symbols);
    _t_free(structures);
    //! [testSymbolStructure]
}

void testGetSize() {
    //! [testGetSize]

    // test a few built-in symbols and structures
    spec_is_long_equal(_d_get_structure_size(0,0,INTEGER,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,BIT,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,FLOAT,0),sizeof(float));
    spec_is_long_equal(_d_get_structure_size(0,0,XADDR,0),sizeof(Xaddr));
    spec_is_long_equal(_d_get_symbol_size(0,0,STRUCTURE_SYMBOL,0),sizeof(Symbol));
    spec_is_long_equal(_d_get_symbol_size(0,0,SYMBOL_LABEL,"shoe_size"),10);

    // test user-defined symbols and structures
    T *structures = _t_new_root(STRUCTURES);                    // create tree that holds structures
    T *symbols = _t_new_root(SYMBOLS);                          // create tree that holds symbols
    Symbol lat = _d_declare_symbol(symbols,structures,FLOAT,"latitude",RECEPTOR_CONTEXT);      // symbols are declared, structures are defined
    Symbol lon = _d_declare_symbol(symbols,structures,FLOAT,"longitude",RECEPTOR_CONTEXT);     // here we declare two meaningful ways to use the structure float

    Defs defs = {structures, symbols, 0, 0};

    // Two symbols, lat and lon are assembled into the structure "latlong"
    Structure latlong = _d_define_structure(symbols,structures,"latlong",RECEPTOR_CONTEXT, 2, lat, lon);

    // House location is a meaningful use of the structure latlong
    Symbol house_loc = _d_declare_symbol(symbols,structures, latlong, "house location",RECEPTOR_CONTEXT);

    // Here's the surface of the latlong.
    float ll[] = {2.0,90.3};
    spec_is_long_equal(_d_get_symbol_size(symbols,structures,house_loc,ll),sizeof(ll));
    spec_is_long_equal(_d_get_structure_size(symbols,structures,latlong,ll),sizeof(ll));

    //structure sizing when structure is a singleton
    Structure latstruct = _d_define_structure(symbols,structures,"latstruct",RECEPTOR_CONTEXT, 1, lat);
    float ls[] = {2.0};
    spec_is_long_equal(_d_get_structure_size(symbols,structures,latstruct,ll),sizeof(ls));

    //@todo what about sizing with optional structural elements...

    _t_free(symbols);
    _t_free(structures);
    //! [testGetSize]

    // In the diagram on 20140806_102233.jpg, "Label" is the symbol, "Struct" is the structure.
    // Use t_dump to display textually the resulting tree structure.  Use _td for a T in the context of a given receptor.
}

void testCodeProcess() {
    //! [testCodeProcess]
    T *defs = _t_new_root(PROCESSES);
    T *code = _t_new_root(ACTION);
    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      "exponent",SIGNATURE_STRUCTURE,INTEGER,
                                      NULL);
    Process p = _d_code_process(defs,code,"power","takes the mathematical power of the two params",signature,RECEPTOR_CONTEXT);

    spec_is_true(is_process(p));
    spec_is_true(!is_symbol(p));
    spec_is_equal(_t_children(defs),p.id);
    T *s = _t_child(defs,p.id);

    spec_is_sem_equal(_t_symbol(s),PROCESS_CODING);

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

    spec_is_str_equal(_d_get_process_name(defs,p),"power");

    _t_free(defs);

    //! [testCodeProcess]
}

void testDefSemtrex() {
    //! [testDefSemtrex]
    Defs d = {_t_newr(0,STRUCTURES),_t_newr(0,SYMBOLS),0,0};
    Symbol SY(d,lat,FLOAT);
    Symbol SY(d,lon,FLOAT);
    Structure ST(d,latlong,2,lat,lon);
    Symbol SY(d,house_loc,latlong);

    T *stx = _d_build_def_semtrex(d,house_loc,0);
    char buf[2000];
    spec_is_str_equal(_dump_semtrex(&d,stx,buf),"/house_loc/(lat,lon)");
    __t_dump(&d,stx,0,buf);
    spec_is_str_equal(buf,"(SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:house_loc) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lat)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lon))))");

    _t_free(stx);
    _t_free(d.structures);
    _t_free(d.symbols);
    //! [testDefSemtrex]
}

void testDefSysDefs() {
    //    spec_is_str_equal(t2s(_t_child(G_contexts[SYS_CONTEXT].root,1)),"");
    dump2json(&G_contexts[SYS_CONTEXT].defs,G_contexts[SYS_CONTEXT].root,"sysdefs");
    spec_is_str_equal(t2s(_t_child(_t_child(G_contexts[SYS_CONTEXT].root,1),STRUCTURE_DEF.id)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL:STRUCTURE_DEF) (STRUCTURE_SYMBOL_SET (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL) (STRUCTURE_SYMBOL:STRUCTURE_SYMBOL)))");
    /* puts("Structures:");puts(t2sp(_t_child(G_contexts[SYS_CONTEXT].root,1))); */
    /* puts("Symbols:");puts(t2sp(_t_child(G_contexts[SYS_CONTEXT].root,2))); */
    /* puts("Processes:");puts(t2sp(_t_child(G_contexts[SYS_CONTEXT].root,3))); */
    //  spec_is_str_equal(t2s(_t_child(G_contexts[SYS_CONTEXT].root,1)),"(STRUCTURES (STRUCTURE_DEFINITION (STRUCTURE_LABEL:BIT) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:INTEGER) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:FLOAT) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:CHAR) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:CSTRING) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SYMBOL) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:ENUM) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE_PATH) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:XADDR) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:LIST) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SURFACE) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:RECEPTOR) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROCESS) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:STRUCTURE) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SCAPE) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SEMTREX) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:STREAM) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:UUID) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:STRUCTURE_DEF) (STRUCTURE_SYMBOL:NULL_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SIGNATURE_PART) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:SIGNATURE_LABEL) (STRUCTURE_SYMBOL:SIGNATURE_TYPE))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SEQUENCE) (STRUCTURE_SYMBOL:STEP_SYMBOL)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROTOCOL_STEP) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:EXPECTATION) (STRUCTURE_SYMBOL:PARAMS) (STRUCTURE_SYMBOL:ACTION))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROTOCOL) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:STEPS) (STRUCTURE_SYMBOL:SEQUENCES))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:DATE) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:YEAR) (STRUCTURE_SYMBOL:MONTH) (STRUCTURE_SYMBOL:DAY))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TIME) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:HOUR) (STRUCTURE_SYMBOL:MINUTE) (STRUCTURE_SYMBOL:SECOND))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TIMESTAMP) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:TODAY) (STRUCTURE_SYMBOL:NOW))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:REDUCTION_ERROR) (STRUCTURE_SYMBOL:ERROR_LOCATION)) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE_DELTA) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:TREE_DELTA_PATH) (STRUCTURE_SYMBOL:TREE_DELTA_VALUE) (STRUCTURE_SYMBOL:TREE_DELTA_COUNT))))");

    // an example of a defined process (if)
    spec_is_str_equal(t2s(_t_child(_t_child(G_contexts[SYS_CONTEXT].root,3),2)),"(PROCESS_CODING (PROCESS_NAME:IF) (PROCESS_INTENTION:if) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL:result) (SIGNATURE_STRUCTURE:TREE)) (INPUT_SIGNATURE (SIGNATURE_LABEL:condition) (SIGNATURE_PROCESS:BOOLEAN)) (INPUT_SIGNATURE (SIGNATURE_LABEL:then) (SIGNATURE_STRUCTURE:TREE)) (INPUT_SIGNATURE (SIGNATURE_LABEL:else) (SIGNATURE_STRUCTURE:TREE))))");
}

void testDef() {
    testDefSysDefs();
    testSymbolGetName();
    testStructureGetName();
    testDefValidate();
    testProcessGetName();
    testDefSymbol();
    testDefStructure();
    testGetSymbolStructure();
    testGetSize();
    testCodeProcess();
    testDefSemtrex();
}
