/**
 * @file symbol_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/def.h"

void testSymbolGetName() {
    //! [testSymbolGetName]
    spec_is_str_equal(_d_get_symbol_name(0,TEST_INT_SYMBOL),"TEST_INT_SYMBOL");
    //! [testSymbolGetName]
}

void testStructureGetName() {
    //! [testStructureGetName]
    spec_is_str_equal(_d_get_structure_name(0,INTEGER),"INTEGER");
    //! [testStructureGetName]
}

void testProcessGetName() {
    //! [testProcessGetName]
    spec_is_str_equal(_d_get_process_name(0,NULL_PROCESS),"NULL_PROCESS");
    //! [testProcessGetName]
}

void testDefSymbol() {
    //! [testDefSymbol]
    Tnode *defs = _t_new_root(SYMBOLS);

    Tnode *def = __d_declare_symbol(defs,INTEGER,"shoe size");
    Symbol ss = 1;

    spec_is_true(is_symbol(ss));
    spec_is_equal(_t_children(defs),ss);
    spec_is_ptr_equal(_t_child(defs,ss),def);
    spec_is_equal(_t_symbol(_t_child(defs,ss)),SYMBOL_DECLARATION);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,ss),1)),SYMBOL_STRUCTURE);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,ss),2)),SYMBOL_LABEL);
    spec_is_str_equal(_d_get_symbol_name(defs,ss),"shoe size");

    _t_free(defs);
    //! [testDefSymbol]
}

void testDefStructure() {
    //! [testDefStructure]
    Tnode *defs = _t_new_root(STRUCTURES);

    Structure st = _d_define_structure(defs,"receptor pair",2,RECEPTOR,RECEPTOR);
    spec_is_equal(_t_children(defs),st);
    Tnode *s = _t_child(defs,st);
    spec_is_equal(_t_symbol(s),STRUCTURE_DEFINITION);
    spec_is_equal(_t_children(s),2);
    Tnode *l = _t_child(s,1);
    Tnode *p = _t_child(s,2);
    spec_is_symbol_equal(0,_t_symbol(l),STRUCTURE_LABEL);
    spec_is_str_equal((char *)_t_surface(l),"receptor pair");
    spec_is_symbol_equal(0,_t_symbol(p),STRUCTURE_PARTS);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    spec_is_symbol_equal(0,_t_symbol(_t_child(p,1)),STRUCTURE_PART);
    spec_is_str_equal(_d_get_structure_name(defs,1),"receptor pair");

    _t_free(defs);
    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    // test built-in symbols
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_DEFINITION),TREE);
    spec_is_structure_equal(0,_d_get_symbol_structure(0,STRUCTURE_PART),SYMBOL);

    // test user-defined symbols
    Tnode *defs = _t_new_root(SYMBOLS);
    Symbol s = _d_declare_symbol(defs,INTEGER,"shoe size");
    spec_is_equal(_d_get_symbol_structure(defs,s),INTEGER);
    _t_free(defs);
    //! [testSymbolStructure]
}

void testGetSize() {
    //! [testGetSize]

    // test built-in symbols and structures
    spec_is_long_equal(_d_get_structure_size(0,0,INTEGER,0),sizeof(int));
    spec_is_long_equal(_d_get_structure_size(0,0,FLOAT,0),sizeof(float));
    spec_is_long_equal(_d_get_symbol_size(0,0,STRUCTURE_PART,0),sizeof(int));
    spec_is_long_equal(_d_get_symbol_size(0,0,SYMBOL_LABEL,"shoe_size"),10);

    // test user-defined symbols and structures
    Tnode *structures = _t_new_root(STRUCTURES);					// create tree that holds structures
    Tnode *symbols = _t_new_root(SYMBOLS);							// create tree that holds symbols
    Symbol lat = _d_declare_symbol(symbols,FLOAT,"latitude");		// symbols are declared, structures are defined
    Symbol lon = _d_declare_symbol(symbols,FLOAT,"longitude");		// here we declare two meaningful ways to use the structure float

    // Two symbols, lat and lon are assembled into the structure "latlong"
    Structure latlong = _d_define_structure(structures,"latlong", 2, lat, lon);

    // House location is a meaningful use of the structure latlong
    Symbol house_loc = _d_declare_symbol(symbols, latlong, "house location");

    // Here's the surface of the latlong.
    float ll[] = {2.0,90.3};
    spec_is_long_equal(_d_get_symbol_size(symbols,structures,house_loc,ll),sizeof(ll));
    spec_is_long_equal(_d_get_structure_size(symbols,structures,latlong,ll),sizeof(ll));

    _t_free(symbols);
    _t_free(structures);
    //! [testGetSize]

    // In the diagram on 20140806_102233.jpg, "Label" is the symbol, "Struct" is the structure.
    // Use t_dump to display textually the resulting tree structure.  Use _td for a Tnode in the context of a given receptor.
}

void testCodeProcess() {
    //! [testCodeProcess]
    Tnode *defs = _t_new_root(PROCESSES);
    Tnode *code = _t_new_root(ACTION);
    Tnode *input = _t_new_root(INPUT_SIGNATURE);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _d_code_process(defs,code,"power","takes the mathematical power of the two params",input,output);

    spec_is_true(is_process(p));
    spec_is_true(!is_symbol(p));
    spec_is_equal(_t_children(defs),-p);
    Tnode *s = _t_child(defs,-p);

    spec_is_equal(_t_symbol(s),PROCESS_CODING);

    // first child is the name of the process
    Tnode *name = _t_child(s,1);
    spec_is_equal(_t_symbol(name),PROCESS_NAME);
    spec_is_str_equal((char *)_t_surface(name),"power");

    // second child is process intention as text (documentation)
    Tnode *intention = _t_child(s,2);
    spec_is_equal(_t_symbol(intention),PROCESS_INTENTION);
    spec_is_str_equal((char *)_t_surface(intention),"takes the mathematical power of the two params");

    // third child is code itself
    spec_is_ptr_equal(_t_child(s,3),code);

    // fourth child is the input signature
    Tnode *in = _t_child(s,4);
    spec_is_equal(_t_symbol(in),INPUT_SIGNATURE);

    // fifth child is the output signature
    Tnode *out = _t_child(s,5);
    spec_is_equal(_t_symbol(out),OUTPUT_SIGNATURE);

    spec_is_str_equal(_d_get_process_name(defs,p),"power");

    _t_free(defs);

    //! [testCodeProcess]
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
}
