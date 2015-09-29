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
    spec_is_symbol_equal(0,_t_symbol(p),STRUCTURE_PARTS);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    Structure x = {RECEPTOR_CONTEXT,SEM_TYPE_STRUCTURE,1};
    spec_is_str_equal(_d_get_structure_name(defs,x),"boolean pair");

    _t_free(defs);
    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    // test built-in symbols
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_DEFINITION),TREE);
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_PART),SYMBOL);

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

    // test built-in symbols and structures
    spec_is_long_equal(_d_get_structure_size(0,0,INTEGER,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,BIT,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,FLOAT,0),sizeof(float));
    spec_is_long_equal(_d_get_structure_size(0,0,XADDR,0),sizeof(Xaddr));
    spec_is_long_equal(_d_get_symbol_size(0,0,STRUCTURE_PART,0),sizeof(Symbol));
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
    T *signature = _t_new_root(PROCESS_SIGNATURE);
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

    spec_is_str_equal(t2s(_t_child(G_contexts[SYS_CONTEXT].root,2)),"(STRUCTURES (STRUCTURE_DEFINITION (STRUCTURE_LABEL:BIT) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:INTEGER) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:FLOAT) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:CHAR) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:CSTRING) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SYMBOL) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:ENUM) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE_PATH) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:XADDR) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:LIST) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SURFACE) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:RECEPTOR) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROCESS) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:STRUCTURE) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SCAPE) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SEMTREX) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:STREAM) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:UUID) (STRUCTURE_PARTS (STRUCTURE_PART:NULL_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SIGNATURE_PART) (STRUCTURE_PARTS (STRUCTURE_PART:SIGNATURE_LABEL) (STRUCTURE_PART:SIGNATURE_TYPE))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:SEQUENCE) (STRUCTURE_PARTS (STRUCTURE_PART:STEP_SYMBOL))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROTOCOL_STEP) (STRUCTURE_PARTS (STRUCTURE_PART:EXPECTATION) (STRUCTURE_PART:PARAMS) (STRUCTURE_PART:ACTION))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:PROTOCOL) (STRUCTURE_PARTS (STRUCTURE_PART:STEPS) (STRUCTURE_PART:SEQUENCES))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:DATE) (STRUCTURE_PARTS (STRUCTURE_PART:YEAR) (STRUCTURE_PART:MONTH) (STRUCTURE_PART:DAY))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TIME) (STRUCTURE_PARTS (STRUCTURE_PART:HOUR) (STRUCTURE_PART:MINUTE) (STRUCTURE_PART:SECOND))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TIMESTAMP) (STRUCTURE_PARTS (STRUCTURE_PART:TODAY) (STRUCTURE_PART:NOW))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:REDUCTION_ERROR) (STRUCTURE_PARTS (STRUCTURE_PART:ERROR_LOCATION))) (STRUCTURE_DEFINITION (STRUCTURE_LABEL:TREE_DELTA) (STRUCTURE_PARTS (STRUCTURE_PART:TREE_DELTA_PATH) (STRUCTURE_PART:TREE_DELTA_VALUE) (STRUCTURE_PART:TREE_DELTA_COUNT))))");

    spec_is_str_equal(t2s(_t_child(G_contexts[SYS_CONTEXT].root,3)),"(PROCESSES (PROCESS_CODING (PROCESS_NAME:NOOP) (PROCESS_INTENTION:no-op)) (PROCESS_CODING (PROCESS_NAME:IF) (PROCESS_INTENTION:if)) (PROCESS_CODING (PROCESS_NAME:ADD_INT) (PROCESS_INTENTION:addition)) (PROCESS_CODING (PROCESS_NAME:SUB_INT) (PROCESS_INTENTION:subtract)) (PROCESS_CODING (PROCESS_NAME:MULT_INT) (PROCESS_INTENTION:multiply)) (PROCESS_CODING (PROCESS_NAME:DIV_INT) (PROCESS_INTENTION:divide)) (PROCESS_CODING (PROCESS_NAME:MOD_INT) (PROCESS_INTENTION:modulo)) (PROCESS_CODING (PROCESS_NAME:EQ_INT) (PROCESS_INTENTION:test equality)) (PROCESS_CODING (PROCESS_NAME:LT_INT) (PROCESS_INTENTION:test less than)) (PROCESS_CODING (PROCESS_NAME:GT_INT) (PROCESS_INTENTION:test greater than)) (PROCESS_CODING (PROCESS_NAME:LTE_INT) (PROCESS_INTENTION:test less than or equal)) (PROCESS_CODING (PROCESS_NAME:GTE_INT) (PROCESS_INTENTION:test greater than or equal)) (PROCESS_CODING (PROCESS_NAME:CONCAT_STR) (PROCESS_INTENTION:concatinate strings)) (PROCESS_CODING (PROCESS_NAME:RESPOND) (PROCESS_INTENTION:respond to signal)) (PROCESS_CODING (PROCESS_NAME:SEND) (PROCESS_INTENTION:send a signal)) (PROCESS_CODING (PROCESS_NAME:EXPECT_ACT) (PROCESS_INTENTION:block run tree until expectation met and then act)) (PROCESS_CODING (PROCESS_NAME:QUOTE) (PROCESS_INTENTION:quote a process so that it can be passed as a value)) (PROCESS_CODING (PROCESS_NAME:INTERPOLATE_FROM_MATCH) (PROCESS_INTENTION:interploate match result in place)) (PROCESS_CODING (PROCESS_NAME:RAISE) (PROCESS_INTENTION:raise reduction error)) (PROCESS_CODING (PROCESS_NAME:STREAM_READ) (PROCESS_INTENTION:read from a stream)) (PROCESS_CODING (PROCESS_NAME:STREAM_WRITE) (PROCESS_INTENTION:write to a stream)) (PROCESS_CODING (PROCESS_NAME:STREAM_AVAILABLE) (PROCESS_INTENTION:check a stream for data availability)) (PROCESS_CODING (PROCESS_NAME:REPLICATE) (PROCESS_INTENTION:replicate process)) (PROCESS_CODING (PROCESS_NAME:LISTEN) (PROCESS_INTENTION:plant a listener)))");
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
