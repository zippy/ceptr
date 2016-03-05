/**
 * @file semtable_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/semtable.h"
#include "../src/receptor.h"

void testSemTableCreate() {
    //! [testSemTableCreate]
    SemTable *sem = _sem_new();
    spec_is_equal(sem->contexts,0);
    int idx = _sem_new_context(sem,(T *)1234L);
    spec_is_equal(idx,0);
    spec_is_equal(sem->contexts,1);
    Symbol s = {SYS_CONTEXT,0,0};
    spec_is_long_equal((long)_sem_context(sem,s)->definitions,1234L);
    _sem_free(sem);
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

void testSemTableGetLabel() {
    //! [testSemTableGetLabel]
    spec_is_str_equal(t2s(_sem_get_label(G_sem,CONTENT_TYPE,HTTP_HEADER_LABEL)),"(HTTP_HEADER_LABEL:Content-Type)");
    spec_is_str_equal(t2s(_sem_get_label(G_sem,CONTENT_TYPE,TEST_SYMBOL_SYMBOL)),"(ENGLISH_LABEL:CONTENT_TYPE)");
    //! [testSemTableGetLabel]
}

testSemGetSymbolStructure() {
    //! [testSemGetSymbolStructure]
    spec_is_structure_equal(0,_sem_get_symbol_structure(G_sem,STRUCTURE_DEFINITION),TUPLE_OF_STRUCTURE_LABEL_AND_STRUCTURE_DEF);
    spec_is_structure_equal(0,_sem_get_symbol_structure(G_sem,STRUCTURE_SYMBOL),SYMBOL);

    //! [testSemGetSymbolStructure]
}

testSemGetByLabel() {
    //! [testSemGetByLabel]
    SemanticID s = _sem_get_by_label(G_sem,"DEFINITIONS",SYS_CONTEXT);
    spec_is_sem_equal(s,DEFINITIONS);

    //! [testSemGetByLabel]
}

void testSemAddLabel() {
    //! [testSemAddLabel]
    _sem_add_label(G_sem,BIT,ASCII_STR,"one or zero");
    spec_is_str_equal(t2s(_sem_get_def(G_sem,BIT)),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:BIT) (ASCII_STR:one or zero)) (STRUCTURE_SYMBOL:NULL_SYMBOL))");
    //! [testSemAddLabel]
}

void testSemTable() {
    testSemTableCreate();
    testSemTableGetName();
    testSemTableGetLabel();
    testSemGetSymbolStructure();
    testSemGetByLabel();
    testSemAddLabel();
}
