/**
 * @file symbol_spec.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/symbol.h"

void testSymbolGetName() {
    //! [testSymbolGetName]
    spec_is_str_equal(_s_get_symbol_name(0,TEST_SYMBOL),"TEST_SYMBOL");
    //! [testSymbolGetName]
}

void testStructureGetName() {
    //! [testStructureGetName]
    spec_is_str_equal(_s_get_structure_name(0,INTEGER),"INTEGER");
    //! [testStructureGetName]
}

void testDefSymbol() {
    //! [testDefSymbol]
    Tnode *defs = _t_new_root(SYMBOLS);

    Tnode *def = _s_def_symbol(defs,INTEGER,"shoe size");
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

    Tnode *def = _s_def_structure(defs,"receptor pair",2,RECEPTOR,RECEPTOR);
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

void testSymbol() {
    testSymbolGetName();
    testStructureGetName();
    testDefSymbol();
    testDefStructure();
}
