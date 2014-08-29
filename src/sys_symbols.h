/**
 * @defgroup symbol Symbols
 * @brief Symbols are the semantic tokens that indicate how to interpret a given structure
 *
 * @{
 * @file sys_symbol.h
 * @brief Defines hard-coded system defined symbols.
 *
 * Symbols are a numeric semantic indicator (like a type) for tree nodes.

 * This file defines the hard-coded (negative value) system defined symbols
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup symbol
 */

#ifndef _CEPTR_SYS_SYMBOL_H
#define _CEPTR_SYS_SYMBOL_H

/// test symbols.  These are defined only for the test suite.
enum TestSymbol {TEST_SYMBOL= -9999,TEST_SYMBOL2,TEST_STR_SYMBOL,TEST_TREE_SYMBOL,
      TEST_NAME_SYMBOL,TEST_FIRST_NAME_SYMBOL,
      _LAST_TEST_SYMBOL
};
static char *G_test_symbol_names[] = {
    "TEST_SYMBOL","TEST_SYMBOL2","TEST_STR_SYMBOL","TEST_TREE_SYMBOL",
    "TEST_NAME_SYMBOL","TEST_FIRST_NAME_SYMBOL"};

/// System defined symbols.
enum SystemSymbol
{
    //-----  Basic symbols for underlying data types
    NULL_SYMBOL = -999,
    TREE_PATH,

    //-----  Symbols for the different semantic parts of semtrexes
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
    RECEPTOR,
    FLUX,                              ///< tree to hold all incoming and in process signals on the various aspects
    STRUCTURES,
    STRUCTURE_DEF,
    STRUCTURE_PART,
    SYMBOLS,
    SYMBOL_DEF,
    SYMBOL_STRUCTURE,
    SYMBOL_LABEL,
    INSTANCE,
    ASPECT,
    SIGNALS,                           ///< list of signals on an aspect in the flux
    SIGNAL,                            ///< a signal on the flux.  It's first child is the contents of the signal
    LISTENERS,                         ///< list of carrier/expectation/action tress that "listen" to changes on the flux
    LISTENER,                          ///< surface of the listener is the carrier symbol, and it has two children, expectation semtrex and action code tree
    EXPECTATION,                       ///< expectation is a semtrex (thus has one child which is the first part of the semtrex)
    ACTION,                            ///< code tree, which specifies the action to perform when an expectation's semtrex matches
    RESPOND,                           ///< an instruction to respond to the initiating signal with the what ever the sub-tree reduced to
    INTERPOLATE_FROM_MATCH,            ///< an instruction which reduces to replaced values from the match
    INTERPOLATE_SYMBOL,                ///< a place holder to indicate which symbol to insert into this part of the three
    RUN_TREE,
    PARAMS,

    //-----  Symbols for the virtual machine host
    RECEPTOR_PACKAGE,
    _LAST_SYS_SYMBOL
};
static char *G_sys_symbol_names[] = {
    "NULL_SYMBOL",
    "TREE_PATH",
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
    "RECEPTOR",
    "FLUX",
    "STRUCTURES",
    "STRUCTURE_DEF",
    "STRUCTURE_PART",
    "SYMBOLS",
    "SYMBOL_DEF",
    "SYMBOL_STRUCTURE",
    "SYMBOL_LABEL",
    "INSTANCE",
    "ASPECT",
    "SIGNALS",
    "SIGNAL",
    "LISTENERS",
    "LISTENER",
    "EXPECTATION",
    "ACTION",
    "RESPOND",
    "INTERPOLATE_FROM_MATCH",
    "INTERPOLATE_SYMBOL",
    "RUN_TREE",
    "PARAMS",
    "RECEPTOR_PACKAGE",
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


/** @}*/
