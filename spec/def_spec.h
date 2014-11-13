/**
 * @file symbol_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
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

    Structure st = _d_define_structure(defs,"receptor pair",RECEPTOR_CONTEXT,2,RECEPTOR,RECEPTOR);
    spec_is_equal(_t_children(defs),st.id);
    T *s = _t_child(defs,st.id);
    spec_is_sem_equal(_t_symbol(s),STRUCTURE_DEFINITION);
    spec_is_equal(_t_children(s),2);
    T *l = _t_child(s,1);
    T *p = _t_child(s,2);
    spec_is_symbol_equal(0,_t_symbol(l),STRUCTURE_LABEL);
    spec_is_str_equal((char *)_t_surface(l),"receptor pair");
    spec_is_symbol_equal(0,_t_symbol(p),STRUCTURE_PARTS);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    Structure x = {0,0,1};
    spec_is_str_equal(_d_get_structure_name(defs,x),"receptor pair");

    _t_free(defs);
    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    // test built-in symbols
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_DEFINITION),TREE);
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_PART),SYMBOL);

    // test user-defined symbols
    T *defs = _t_new_root(SYMBOLS);
    Symbol s = _d_declare_symbol(defs,INTEGER,"shoe size",RECEPTOR_CONTEXT);
    spec_is_sem_equal(_d_get_symbol_structure(defs,s),INTEGER);
    _t_free(defs);
    //! [testSymbolStructure]
}

void testGetSize() {
    //! [testGetSize]

    // test built-in symbols and structures
    spec_is_long_equal(_d_get_structure_size(0,0,INTEGER,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,BOOLEAN,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,FLOAT,0),sizeof(float));
    spec_is_long_equal(_d_get_structure_size(0,0,XADDR,0),sizeof(Xaddr));
    spec_is_long_equal(_d_get_symbol_size(0,0,STRUCTURE_PART,0),sizeof(Symbol));
    spec_is_long_equal(_d_get_symbol_size(0,0,SYMBOL_LABEL,"shoe_size"),10);

    // test user-defined symbols and structures
    T *structures = _t_new_root(STRUCTURES);					// create tree that holds structures
    T *symbols = _t_new_root(SYMBOLS);							// create tree that holds symbols
    Symbol lat = _d_declare_symbol(symbols,FLOAT,"latitude",RECEPTOR_CONTEXT);		// symbols are declared, structures are defined
    Symbol lon = _d_declare_symbol(symbols,FLOAT,"longitude",RECEPTOR_CONTEXT);		// here we declare two meaningful ways to use the structure float

    Defs defs = {structures, symbols, 0, 0};

    // Two symbols, lat and lon are assembled into the structure "latlong"
    Structure latlong = _d_define_structure(structures,"latlong",RECEPTOR_CONTEXT, 2, lat, lon);

    // House location is a meaningful use of the structure latlong
    Symbol house_loc = _d_declare_symbol(symbols, latlong, "house location",RECEPTOR_CONTEXT);

/*
    char buf[1000];
    __t_dump(&defs, structures, 0, buf);
    puts("\r\n");
    puts(buf);

    __t_dump(&defs, symbols, 0, buf);
    puts("\r\n");
    puts(buf);

    T* latlonDef = _t_child(structures, latlong);
    __t_dump(&defs, latlonDef, 0, buf);
    puts("\r\n");
    puts(buf);
*/
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
    T *input = _t_new_root(INPUT_SIGNATURE);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _d_code_process(defs,code,"power","takes the mathematical power of the two params",input,output,RECEPTOR_CONTEXT);

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

    // fourth child is the input signature
    T *in = _t_child(s,4);
    spec_is_sem_equal(_t_symbol(in),INPUT_SIGNATURE);

    // fifth child is the output signature
    T *out = _t_child(s,5);
    spec_is_sem_equal(_t_symbol(out),OUTPUT_SIGNATURE);

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
    spec_is_str_equal(buf," (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:house_loc) (SEMTREX_SEQUENCE (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lat)) (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:lon))))");

    _t_free(stx);
    _t_free(d.structures);
    _t_free(d.symbols);
    //! [testDefSemtrex]
}

void testDef() {
    testSymbolGetName();
    testStructureGetName();
    testProcessGetName();
    testDefSymbol();
    testDefStructure();
    testGetSymbolStructure();
    testGetSize();
    testCodeProcess();
    testDefSemtrex();
}
