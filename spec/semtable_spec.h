/**
 * @file semtable_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/semtable.h"
#include "../src/receptor.h"

void testSemTableCreate() {
    //! [testSemTableCreate]
    SemTable *st = _sem_new();
    spec_is_equal(st->contexts,0);
    int idx = _sem_new_context(st,(T *)1234L);
    spec_is_equal(idx,0);
    spec_is_equal(st->contexts,1);
    Symbol s = {SYS_CONTEXT,0,0};
    spec_is_long_equal((long)_sem_context(st,s)->definitions,1234L);
    _sem_free(st);
    //! [testSemTableCreate]
}

void testSemTableGetName() {
    //! [testSemTableGetName]
    spec_is_str_equal(_sem_get_name(G_sem,TEST_INT_SYMBOL),"TEST_INT_SYMBOL");
    spec_is_str_equal(_sem_get_name(G_sem,NULL_SYMBOL),"NULL_SYMBOL");
    spec_is_str_equal(_sem_get_name(G_sem,NULL_STRUCTURE),"NULL_STRUCTURE");
    spec_is_str_equal(_sem_get_name(G_sem,NULL_PROCESS),"NULL_PROCESS");
    spec_is_str_equal(_sem_get_name(G_sem,DEFINITIONS),"DEFINITIONS");
    spec_is_str_equal(_sem_get_name(G_sem,INTEGER),"INTEGER");
    spec_is_str_equal(_sem_get_name(G_sem,IF),"IF");
    //! [testSemTableGetName]
}

testSemGetSymbolStructure() {
    //! [testSemGetSymbolStructure]
    spec_is_structure_equal(0,_sem_get_symbol_structure(G_sem,STRUCTURE_DEFINITION),TUPLE_OF_STRUCTURE_LABEL_AND_STRUCTURE_DEF);
    spec_is_structure_equal(0,_sem_get_symbol_structure(G_sem,STRUCTURE_SYMBOL),SYMBOL);

    //! [testSemGetSymbolStructure]
}

void testSemTable() {
    testSemTableCreate();
    testSemTableGetName();
    testSemGetSymbolStructure();
}
