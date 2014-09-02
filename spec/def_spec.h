/**
 * @file symbol_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/def.h"

void testSymbolGetName() {
    //! [testSymbolGetName]
    spec_is_str_equal(_d_get_symbol_name(0,TEST_SYMBOL),"TEST_SYMBOL");
    //! [testSymbolGetName]
}

void testStructureGetName() {
    //! [testStructureGetName]
    spec_is_str_equal(_d_get_structure_name(0,INTEGER),"INTEGER");
    //! [testStructureGetName]
}

void testDefSymbol() {
    //! [testDefSymbol]
    Tnode *defs = _t_new_root(SYMBOLS);

    Tnode *def = _d_def_symbol(defs,INTEGER,"shoe size");
    spec_is_equal(_t_children(defs),1);
    spec_is_ptr_equal(_t_child(defs,1),def);
    spec_is_equal(_t_symbol(_t_child(defs,1)),SYMBOL_DEF);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,1),1)),SYMBOL_STRUCTURE);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,1),2)),SYMBOL_LABEL);

    _t_free(defs);
    //! [testDefSymbol]
}

void testDefStructure() {
    //! [testDefStructure]
    Tnode *defs = _t_new_root(STRUCTURES);

    Tnode *def = _d_def_structure(defs,"receptor pair",2,RECEPTOR,RECEPTOR);
    spec_is_equal(_t_children(defs),1);
    spec_is_ptr_equal(_t_child(defs,1),def);
    Tnode *s = _t_child(defs,1);
    spec_is_equal(_t_symbol(s),STRUCTURE_DEF);
    spec_is_str_equal((char *)_t_surface(s),"receptor pair")
    spec_is_equal(_t_children(s),2);
    spec_is_equal(_t_symbol(_t_child(s,1)),STRUCTURE_PART);
    spec_is_equal(_t_symbol(_t_child(s,1)),STRUCTURE_PART);

    _t_free(defs);
    //! [testDefStructure]
}

void testGetSymbolStructure() {
    //! [testSymbolStructure]
    // test built-in symbols
    spec_is_equal(_d_get_symbol_structure(0,STRUCTURE_DEF),CSTRING);
    spec_is_equal(_d_get_symbol_structure(0,STRUCTURE_PART),INTEGER);

    // test user-defined symbols
    Tnode *defs = _t_new_root(SYMBOLS);
    Tnode *def = _d_def_symbol(defs,INTEGER,"shoe size");
    spec_is_equal(_d_get_symbol_structure(defs,1),INTEGER);
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
    Tnode *structures = _t_new_root(STRUCTURES);
    Tnode *symbols = _t_new_root(SYMBOLS);
    _d_def_symbol(symbols,FLOAT,"latitude");
    _d_def_symbol(symbols,FLOAT,"longitude");
    _d_def_structure(structures,"latlong",2,1,2);
    _d_def_symbol(symbols,1,"house location");

    float ll[] = {2.0,90.3};
    spec_is_long_equal(_d_get_symbol_size(symbols,structures,3,ll),sizeof(ll));
    spec_is_long_equal(_d_get_structure_size(symbols,structures,1,ll),sizeof(ll));

    _t_free(symbols);
    _t_free(structures);
    //! [testGetSize]
}

void testDef() {
    testSymbolGetName();
    testStructureGetName();
    testDefSymbol();
    testDefStructure();
    testGetSymbolStructure();
    testGetSize();
}
