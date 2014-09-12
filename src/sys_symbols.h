/**
 * @file sys_symbols.h
 * @brief Defines hard-coded system defined symbols.
 *
 * Symbols are a numeric semantic indicator (like a type) for tree nodes.
 *
 * This file defines the hard-coded (negative value) system defined symbols
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SYS_SYMBOL_H
#define _CEPTR_SYS_SYMBOL_H
#include "sys_structures.h"

/// test symbols.  These are defined only for the test suite.
enum TestSymbol {TEST_INT_SYMBOL= -9999,TEST_INT_SYMBOL2,TEST_STR_SYMBOL,TEST_TREE_SYMBOL,TEST_TREE_SYMBOL2,
		 TEST_NAME_SYMBOL,TEST_FIRST_NAME_SYMBOL,TEST_RECEPTOR_SYMBOL,
		 _LAST_TEST_SYMBOL
};
static char *G_test_symbol_names[] = {
    "TEST_INT_SYMBOL",
    "TEST_INT_SYMBOL2",
    "TEST_STR_SYMBOL",
    "TEST_TREE_SYMBOL",
    "TEST_TREE_SYMBOL2",
    "TEST_NAME_SYMBOL",
    "TEST_FIRST_NAME_SYMBOL",
    "TEST_RECEPTOR_SYMBOL"
};

static Structure G_test_symbol_structures[] = {
    INTEGER, //"TEST_INT_SYMBOL",
    INTEGER, //"TEST_INT_SYMBOL2",
    CSTRING, //"TEST_STR_SYMBOL",
    TREE,    //"TEST_TREE_SYMBOL",
    TREE,    //"TEST_TREE_SYMBOL2",
    TREE,    //"TEST_NAME_SYMBOL",
    CSTRING, //"TEST_FIRST_NAME_SYMBOL",
    RECEPTOR,//"TEST_RECEPTOR_SYMBOL"
};

/// System defined symbols.
enum SystemSymbol
{
    //-----  Basic symbols for underlying data types
    NULL_SYMBOL = -999,

    //-----  Symbols for the different semantic parts of semtrexes
    SEMTREX_MATCHED_PATH,              ///< path to symbol matched by semtrex
    SEMTREX_SYMBOL_LITERAL,            ///< This system symbol matches on the semantic type.         Ex: /TEST_SYMBOL
    SEMTREX_SEQUENCE,                  ///< Match on a sequence of child nodes which are any valid semtrex's.  Ex: comma separated nodes
    SEMTREX_OR,                        ///< Logical OR between two Semtrex expressions.      Ex: |
    SEMTREX_SYMBOL_ANY,                ///< Match any symbol or value of the node.           Ex: .
    SEMTREX_ZERO_OR_MORE,              ///< Requires one child Semtrex and matches on zero or more of that Semtrex.  Ex: /0/TestSemtrex*
    SEMTREX_ONE_OR_MORE,               ///< Requires one child Semtrex and matches on one or more of that Semtrex.   Ex: /0/TestSemtrex+
    SEMTREX_ZERO_OR_ONE,               ///< Requires one child Semtrex and matches on zero or one of that Semtrex.   Ex: /0/TestSemtrex?
    SEMTREX_VALUE_LITERAL,	       ///< Matches on the semantic type and the data value.
    SEMTREX_GROUP,                     ///< Grouping                                 Ex: (...)operator
    SEMTREX_MATCH,                     ///< Returns result and sibling count.        Ex: {name:expr} (verify this is what it's supposed to do)
    SEMTREX_MATCH_RESULTS,             ///< In the FSA, keeps track of which part matches so it can be referenced
    SEMTREX_MATCH_SIBLINGS_COUNT,      ///< In the FSA, it's the length of the match

    //-----  Symbols for receptors
    RECEPTOR_XADDR,                    ///< An Xaddr that points to a receptor
    FLUX,                              ///< tree to hold all incoming and in process signals on the various aspects
    STRUCTURES,
    STRUCTURE_DEFINITION,
    STRUCTURE_LABEL,
    STRUCTURE_PARTS,
    STRUCTURE_PART,
    SYMBOLS,
    SYMBOL_DECLARATION,
    SYMBOL_STRUCTURE,
    SYMBOL_LABEL,
    ASPECT,
    SIGNALS,                           ///< list of signals on an aspect in the flux
    SIGNAL,                            ///< a signal on the flux.  It's first child is the contents of the signal
    LISTENERS,                         ///< list of carrier/expectation/action tress that "listen" to changes on the flux
    LISTENER,                          ///< surface of the listener is the carrier symbol, and it has two children, expectation semtrex and action code tree
    EXPECTATION,                       ///< expectation is a semtrex (thus has one child which is the first part of the semtrex)
    ACTION,                            ///< code tree, which specifies the action to perform when an expectation's semtrex matches
    INTERPOLATE_SYMBOL,                ///< a place holder to indicate which symbol to insert into this part of the three
    PROCESSES,
    PROCESS_CODING,
    PROCESS_NAME,
    PROCESS_INTENTION,
    INPUT_SIGNATURE,
    OUTPUT_SIGNATURE,
    RUN_TREE,
    PARAMS,

    //-----  Symbols for the virtual machine host
    VM_HOST_RECEPTOR,
    COMPOSITORY,                      ///< receptor that holds available receptor packages for installation
    MANIFEST,                         ///< configuration template to be filled out for the installation of a receptor
    MANIFEST_PAIR,
    MANIFEST_LABEL,                   ///< a label in the manifest to identify a binding
    MANIFEST_SPEC,                    ///< a symbol to specify what type of data must be provided for a given manifest label
    RECEPTOR_PACKAGE,                 ///< a manifest, a symbol declaration tree, a structure definition tree, and an identifier
    RECEPTOR_IDENTIFIER,              ///< uuid that identifies receptors
    INSTALLED_RECEPTOR,               ///< contains the installed receptor as well as state information (enabled,disabled, etc..)
    ACTIVE_RECEPTORS,                 ///< list of currently active INSTALLED_RECEPTORS
    BINDINGS,                         ///< specifics that match a MANIFEST and allow a receptor to be installed
    BINDING_PAIR,                     ///< a pair that matches a MANIFEST_LABEL with a given binding
    _LAST_SYS_SYMBOL
};
static char *G_sys_symbol_names[] = {
    "NULL_SYMBOL",
    "SEMTREX_MATCHED_PATH",
    "SEMTREX_SYMBOL_LITERAL",
    "SEMTREX_SEQUENCE",
    "SEMTREX_OR",
    "SEMTREX_SYMBOL_ANY",
    "SEMTREX_ZERO_OR_MORE",
    "SEMTREX_ONE_OR_MORE",
    "SEMTREX_ZERO_OR_ONE",
    "SEMTREX_VALUE_LITERAL",
    "SEMTREX_GROUP",
    "SEMTREX_MATCH",
    "SEMTREX_MATCH_RESULTS",
    "SEMTREX_MATCH_SIBLINGS_COUNT",
    "RECEPTOR_XADDR",
    "FLUX",
    "STRUCTURES",
    "STRUCTURE_DEFINITION",
    "STRUCTURE_LABEL",
    "STRUCTURE_PARTS",
    "STRUCTURE_PART",
    "SYMBOLS",
    "SYMBOL_DECLARATION",
    "SYMBOL_STRUCTURE",
    "SYMBOL_LABEL",
    "ASPECT",
    "SIGNALS",
    "SIGNAL",
    "LISTENERS",
    "LISTENER",
    "EXPECTATION",
    "ACTION",
    "INTERPOLATE_SYMBOL",
    "PROCESSES",
    "PROCESS_CODING",
    "PROCESS_NAME",
    "PROCESS_INTENTION",
    "INPUT_SIGNATURE",
    "OUTPUT_SIGNATURE",
    "RUN_TREE",
    "PARAMS",
    "VM_HOST_RECEPTOR",
    "COMPOSITORY",
    "MANIFEST",
    "MANIFEST_PAIR",
    "MANIFEST_LABEL",
    "MANIFEST_SPEC",
    "RECEPTOR_PACKAGE",
    "RECEPTOR_IDENTIFIER",
    "INSTALLED_RECEPTOR",
    "ACTIVE_RECEPTORS",
    "BINDINGS",
    "BINDING_PAIR",

};

static Structure G_sys_symbol_structures[] = {
    NULL_STRUCTURE,      //"NULL_SYMBOL",
    TREE_PATH,           //"SEMTREX_MATCHED_PATH",
    SYMBOL,              //"SEMTREX_SYMBOL_LITERAL",
    LIST,                //"SEMTREX_SEQUENCE",
    NULL_STRUCTURE,      //"SEMTREX_OR",
    NULL_STRUCTURE,      //"SEMTREX_SYMBOL_ANY",
    NULL_STRUCTURE,      //"SEMTREX_ZERO_OR_MORE",
    NULL_STRUCTURE,      //"SEMTREX_ONE_OR_MORE",
    NULL_STRUCTURE,      //"SEMTREX_ZERO_OR_ONE",
    NULL_STRUCTURE,      //"SEMTREX_VALUE_LITERAL",
    SYMBOL,              //"SEMTREX_GROUP",
    SYMBOL,              //"SEMTREX_MATCH",
    NULL_STRUCTURE,      //"SEMTREX_MATCH_RESULTS",
    INTEGER,             //"SEMTREX_MATCH_SIBLINGS_COUNT",
    XADDR,               //"RECEPTOR_XADDR",
    LIST,                //"FLUX",
    LIST,                //"STRUCTURES",
    TREE,                //"STRUCTURE_DEFINITION",
    CSTRING,             //STRUCTURE_LABEL,
    LIST,                //"STRUCTURE_PARTS",
    SYMBOL,              //"STRUCTURE_PART",
    LIST          ,      //"SYMBOLS",
    NULL_STRUCTURE,      //"SYMBOL_DECLARATION",
    NULL_STRUCTURE,      //"SYMBOL_STRUCTURE",
    CSTRING,             //"SYMBOL_LABEL",
    INTEGER,             //"ASPECT",
    LIST,                //"SIGNALS",
    NULL_STRUCTURE,      //"SIGNAL",
    LIST,                //"LISTENERS",
    INTEGER,             //"LISTENER",
    NULL_STRUCTURE,      //"EXPECTATION",
    PROCESS,             //"ACTION",
    NULL_STRUCTURE,      //"RESPOND",
    NULL_STRUCTURE,      //"INTERPOLATE_FROM_MATCH",
    SYMBOL,              //"INTERPOLATE_SYMBOL",
    LIST,                //"PROCESSES"
    TREE,                //"PROCESS_CODING",
    CSTRING,             //"PROCESS_NAME",
    CSTRING,             //"PROCESS_INTENTION",
    TREE,                //"INPUT_SIGNATURE",
    SYMBOL,              //"OUTPUT_SIGNATURE",
    TREE,                //"RUN_TREE",
    LIST,                //"PARAMS",
    RECEPTOR,            //"VM_HOST_RECEPTOR",
    RECEPTOR,            //"COMPOSITORY",
    TREE,                //"MANIFEST",
    LIST,                //"MANIFEST_PAIR",
    CSTRING,             //"MANIFEST_LABEL",
    SYMBOL,              //"MANIFEST_SPEC",
    TREE,                //"RECEPTOR_PACKAGE",
    INTEGER,             //"RECEPTOR_IDENTIFIER",
    TREE,                //"INSTALLED_RECEPTOR,"
    LIST,                //"ACTIVE_RECEPTORS",
    TREE,                //"BINDINGS",
    LIST,                //"BINDING_PAIR"
};

#endif


//	Semantic Type:
//		Structures, like "float", are abstract labels.
//		A semantic type is a label with a particular semantic application or implementation of that labelled structure.
//		"int" is a labelled structure, "age" is a semantic implementation.
//
//	Ex:
//		/TEST_SYMBOL="t"		: the root's semantic type is "semtrex_symbol_literal" and it's literal value = 't'

//  SEMTREX_GROUP
//      Paren-group: A group to which you apply some function.
//      SMTP: mail from         where "from is optional"
//      Ex: /ascii-line/char='M'...etc..., (F R O M)?
//
//  Curly brace is a named (or referenced) group
//      Ex: /ascii-line/char='M'...etc..., (F R O M)?{email-address:.*}
