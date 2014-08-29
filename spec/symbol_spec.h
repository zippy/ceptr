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

void testSymbolDef() {
    //! [testSymbolDef]
    Tnode *defs = _t_new_root(SYMBOLS);

    Tnode *def = _s_def(defs,INTEGER,"shoe size");
    spec_is_equal(_t_children(defs),1);
    spec_is_ptr_equal(_t_child(defs,1),def);
    spec_is_equal(_t_symbol(_t_child(defs,1)),SYMBOL_DEF);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,1),1)),SYMBOL_STRUCTURE);
    spec_is_equal(_t_symbol(_t_child(_t_child(defs,1),2)),SYMBOL_LABEL);

    //! [testSymbolDef]
}

void testSymbol() {
    testSymbolGetName();
    testSymbolDef();
}
