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
#include "ceptr_types.h"

#define is_sys_symbol(s) (s.context == SYS_CONTEXT)
#define is_sys_test_symbol(s) (s.context == TEST_CONTEXT)

Symbol TEST_INT_SYMBOL;
Symbol TEST_INT_SYMBOL2;
Symbol TEST_STR_SYMBOL;
Symbol TEST_FLOAT_SYMBOL;
Symbol TEST_TREE_SYMBOL;
Symbol TEST_TREE_SYMBOL2;
Symbol TEST_NAME_SYMBOL;
Symbol TEST_FIRST_NAME_SYMBOL;
Symbol TEST_RECEPTOR_SYMBOL;
Symbol TEST_ALPHABETIZE_SCAPE_SYMBOL;

enum SystemSymbolIDs
{
    //-----  Basic symbols for underlying data types
    NULL_SYMBOL_ID = 0,
    DEFINITIONS_ID,
    STRUCTURES_ID,
    STRUCTURE_DEFINITION_ID,
    STRUCTURE_LABEL_ID,
    STRUCTURE_PARTS_ID,
    STRUCTURE_PART_ID,
    SYMBOLS_ID,
    SYMBOL_DECLARATION_ID,
    SYMBOL_STRUCTURE_ID,
    SYMBOL_LABEL_ID,

    TRUE_FALSE_ID,

    //-----  Symbols for the different semantic parts of semtrexes
    SEMTREX_MATCHED_PATH_ID,              ///< path to symbol matched by semtrex
    SEMTREX_SYMBOL_LITERAL_ID,            ///< This system symbol matches on the semantic type.         Ex: /TEST_SYMBOL
    SEMTREX_SYMBOL_LITERAL_NOT_ID,                ///< Match any symbol or value of the node except.           Ex: .
    SEMTREX_SYMBOL_SET_ID,
    SEMTREX_SYMBOL_ID,                    ///< symbols in the literal set
    SEMTREX_SEQUENCE_ID,                  ///< Match on a sequence of child nodes which are any valid semtrex's.  Ex: comma separated nodes
    SEMTREX_OR_ID,                        ///< Logical OR between two Semtrex expressions.      Ex: |
    SEMTREX_NOT_ID,                       ///< Logical ~ between of a Semtrex expressions.      Ex: ~
    SEMTREX_SYMBOL_ANY_ID,                ///< Match any symbol or value of the node.           Ex: .
    SEMTREX_ZERO_OR_MORE_ID,              ///< Requires one child Semtrex and matches on zero or more of that Semtrex.  Ex: /0/TestSemtrex*
    SEMTREX_ONE_OR_MORE_ID,               ///< Requires one child Semtrex and matches on one or more of that Semtrex.   Ex: /0/TestSemtrex+
    SEMTREX_ZERO_OR_ONE_ID,               ///< Requires one child Semtrex and matches on zero or one of that Semtrex.   Ex: /0/TestSemtrex?
    SEMTREX_VALUE_LITERAL_ID,	          ///< Matches on the semantic type and the data values.
    SEMTREX_VALUE_LITERAL_NOT_ID,	  ///< Matches on logical not of semantic type and data values
    SEMTREX_VALUE_SET_ID,                 ///< list containing the values to match on
    SEMTREX_GROUP_ID,                     ///< Grouping                                 Ex: (...)operator
    SEMTREX_DESCEND_ID,
    SEMTREX_WALK_ID,
    SEMTREX_MATCH_ID,                     ///< Returns result and sibling count.        Ex: {name:expr} (verify this is what it's supposed to do)
    SEMTREX_MATCH_CURSOR_ID,              ///< stores c pointer to tree node at start of match
    SEMTREX_MATCH_RESULTS_ID,             ///< In the FSA_ID, keeps track of which part matches so it can be referenced
    SEMTREX_MATCH_SYMBOL_ID,
    SEMTREX_MATCH_SIBLINGS_COUNT_ID,      ///< In the FSA_ID, it's the length of the match

    //----- Symbols for parsing  matching string representation of semtrex
    ASCII_CHAR_ID,
    ASCII_CHARS_ID,

    //-----  Symbols for receptors
    RECEPTOR_XADDR_ID,                    ///< An Xaddr that points to a receptor
    FLUX_ID,                              ///< tree to hold all incoming and in process signals on the various aspects
    SCAPE_SPEC_ID,
    ASPECTS_ID,
    ASPECT_DEF_ID,
    ASPECT_TYPE_ID,
    CARRIER_ID,
    ASPECT_ID,
    SIGNALS_ID,                           ///< list of signals on an aspect in the flux
    SIGNAL_ID,                            ///< a signal on the flux.  It's first child is the contents of the signal
    ENVELOPE_ID,
    BODY_ID,
    LISTENERS_ID,                         ///< list of carrier/expectation/action tress that "listen" to changes on the flux
    LISTENER_ID,                          ///< surface of the listener is the carrier symbol, and it has two children_ID, expectation semtrex and action code tree
    EXPECTATION_ID,                       ///< expectation is a semtrex (thus has one child which is the first part of the semtrex)
    ACTION_ID,                            ///< code tree_ID, which specifies the action to perform when an expectation's semtrex matches
    INTERPOLATE_SYMBOL_ID,                ///< a place holder to indicate which symbol to insert into this part of the three
    PROCESSES_ID,
    PROCESS_CODING_ID,
    PROCESS_NAME_ID,
    PROCESS_INTENTION_ID,
    INPUT_ID,
    INPUT_SIGNATURE_ID,
    INPUT_LABEL_ID,
    SIGNATURE_STRUCTURE_ID,
    OUTPUT_SIGNATURE_ID,
    RUN_TREE_ID,                         ///< think about this as a stack frame and its code
    PARAM_REF_ID,                        ///< used in a code tree as a reference to a parameter
    PARAMS_ID,
    PROTOCOLS_ID,
    PROTOCOL_ID,
    ROLES_ID,
    ROLE_ID,
    INTERACTIONS_ID,
    INTERACTION_ID,
    STEP_ID,
    FROM_ROLE_ID,
    TO_ROLE_ID,
    RESPONSE_STEPS_ID,
    SCAPES_ID,

    //-----  Symbols for the virtual machine host
    VM_HOST_RECEPTOR_ID,
    COMPOSITORY_ID,                      ///< receptor that holds available receptor packages for installation
    MANIFEST_ID,                         ///< configuration template to be filled out for the installation of a receptor
    MANIFEST_PAIR_ID,
    MANIFEST_LABEL_ID,                   ///< a label in the manifest to identify a binding
    MANIFEST_SPEC_ID,                    ///< a symbol to specify what type of data must be provided for a given manifest label
    RECEPTOR_PACKAGE_ID,                 ///< a manifest, a symbol declaration tree, a structure definition tree_ID, and an identifier
    RECEPTOR_IDENTIFIER_ID,              ///< uuid that identifies receptors
    INSTALLED_RECEPTOR_ID,               ///< contains the installed receptor as well as state information (enabled,disabled_ID, etc..)
    ACTIVE_RECEPTORS_ID,                 ///< list of currently active INSTALLED_RECEPTORS
    PENDING_SIGNALS_ID,                  ///< list of currently active INSTALLED_RECEPTORS
    BINDINGS_ID,                         ///< specifics that match a MANIFEST and allow a receptor to be installed
    BINDING_PAIR_ID,                     ///< a pair that matches a MANIFEST_LABEL with a given binding
};


static Structure G_sys_symbol_structures[] = {
 /*   NULL_STRUCTURE,      //NULL_SYMBOL
    BOOLEAN,             //TRUE_FALSE
    TREE_PATH,           //SEMTREX_MATCHED_PATH
    SYMBOL,              //SEMTREX_SYMBOL_LITERAL
    LIST,                //SEMTREX_SEQUENCE
    NULL_STRUCTURE,      //SEMTREX_OR
    NULL_STRUCTURE,      //SEMTREX_NOT
    NULL_STRUCTURE,      //SEMTREX_SYMBOL_ANY
    SYMBOL,      //SEMTREX_SYMBOL_ANY_EXCEPT
    NULL_STRUCTURE,      //SEMTREX_ZERO_OR_MORE
    NULL_STRUCTURE,      //SEMTREX_ONE_OR_MORE
    NULL_STRUCTURE,      //SEMTREX_ZERO_OR_ONE
    TREE,                //SEMTREX_VALUE_LITERAL
    TREE,                //SEMTREX_VALUE_LITERAL_NOT
    VALUE_SET,           //LIST
    SYMBOL,              //SEMTREX_GROUP
    NULL_STRUCTURE,      //SEMTREX_DESCEND
    INTEGER,             //SEMTREX_MATCH
    NULL_STRUCTURE,      //SEMTREX_MATCH_RESULTS
    SYMBOL,              //SEMTREX_MATCH_SYMBOL
    INTEGER,             //SEMTREX_MATCH_SIBLINGS_COUNT
    XADDR,               //RECEPTOR_XADDR
    LIST,                //FLUX
    LIST,                //DEFINITIONS
    LIST,                //STRUCTURES
    TREE,                //STRUCTURE_DEFINITION
    CSTRING,             //STRUCTURE_LABEL
    LIST,                //STRUCTURE_PARTS
    SYMBOL,              //STRUCTURE_PART
    LIST,                //SYMBOLS
    TREE,                //SYMBOL_DECLARATION
    STRUCTURE,           //SYMBOL_STRUCTURE
    CSTRING,             //SYMBOL_LABEL
    TREE,                //SCAPE_SPEC
    LIST,                //ASPECTS
    TREE,                //ASPECT_DEF
    BOOLEAN,             //ASPECT_TYPE,
    SYMBOL,              //CARRIER
    INTEGER,             //ASPECT
    LIST,                //SIGNALS
    TREE,                //SIGNAL
    LIST,                //ENVELOPE
    TREE,                //BODY
    LIST,                //LISTENERS
    INTEGER,             //LISTENER
    NULL_STRUCTURE,      //EXPECTATION
    PROCESS,             //ACTION
    SYMBOL,              //INTERPOLATE_SYMBOL
    LIST,                //PROCESSES
    TREE,                //PROCESS_CODING
    CSTRING,             //PROCESS_NAME
    CSTRING,             //PROCESS_INTENTION
    LIST,                //INPUT
    TREE,                //INPUT_SIGNATURE
    CSTRING,             //INPUT_LABEL
    STRUCTURE,           //SIGNATURE_STRUCTURE
    NULL_STRUCTURE,      //OUTPUT_SIGNATURE
    TREE,                //RUN_TREE
    TREE_PATH,           //PARAM_REF
    LIST,                //PARAMS
    LIST,//PROTOCOLS,
    TREE,//PROTOCOL,
    LIST,//ROLES,
    CSTRING,//ROLE,
    LIST,//INTERACTIONS,
    LIST,//INTERACTION
    CSTRING,//STEP,
    ENUM,//FROM_ROLE,
    ENUM,//TO_ROLE,
    LIST,//RESPONSE_STEPS,
    LIST,                //SCAPES
    RECEPTOR,            //VM_HOST_RECEPTOR
    RECEPTOR,            //COMPOSITORY
    TREE,                //MANIFEST
    LIST,                //MANIFEST_PAIR
    CSTRING,             //MANIFEST_LABEL
    SYMBOL,              //MANIFEST_SPEC
    TREE,                //RECEPTOR_PACKAGE
    INTEGER,             //RECEPTOR_IDENTIFIER
    TREE,                //INSTALLED_RECEPTOR
    LIST,                //ACTIVE_RECEPTORS
    LIST,                //PENDING_SIGNALS
    TREE,                //BINDINGS
    LIST,                //BINDING_PAIR
 */
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

T *G_sys_root;
Defs G_sys_defs;

Symbol NULL_SYMBOL;
Symbol DEFINITIONS;
Symbol STRUCTURES;
Symbol STRUCTURE_DEFINITION;
Symbol STRUCTURE_LABEL;
Symbol STRUCTURE_PARTS;
Symbol STRUCTURE_PART;
Symbol SYMBOLS;
Symbol PROCESSES;
Symbol SYMBOL_DECLARATION;
Symbol SYMBOL_STRUCTURE;
Symbol SYMBOL_LABEL;

Symbol TRUE_FALSE;


Symbol SEMTREX_MATCHED_PATH;
Symbol SEMTREX_SYMBOL_LITERAL;
Symbol SEMTREX_SYMBOL_LITERAL_NOT;
Symbol SEMTREX_SYMBOL_SET;
Symbol SEMTREX_SYMBOL;
Symbol SEMTREX_SEQUENCE;
Symbol SEMTREX_OR;
Symbol SEMTREX_NOT;
Symbol SEMTREX_SYMBOL_ANY;
Symbol SEMTREX_ZERO_OR_MORE;
Symbol SEMTREX_ONE_OR_MORE;
Symbol SEMTREX_ZERO_OR_ONE;
Symbol SEMTREX_VALUE_LITERAL;
Symbol SEMTREX_VALUE_LITERAL_NOT;
Symbol SEMTREX_VALUE_SET;
Symbol SEMTREX_GROUP;
Symbol SEMTREX_DESCEND;
Symbol SEMTREX_WALK;
Symbol SEMTREX_MATCH;
Symbol SEMTREX_MATCH_CURSOR;
Symbol SEMTREX_MATCH_RESULTS;
Symbol SEMTREX_MATCH_SYMBOL;
Symbol SEMTREX_MATCH_SIBLINGS_COUNT;
Symbol ASCII_CHAR;
Symbol ASCII_CHARS;

Symbol RECEPTOR_XADDR;
Symbol FLUX;
Symbol SCAPE_SPEC;
Symbol ASPECTS;
Symbol ASPECT_DEF;
Symbol ASPECT_TYPE;
Symbol CARRIER;
Symbol ASPECT;
Symbol SIGNALS;
Symbol SIGNAL;
Symbol ENVELOPE;
Symbol BODY;
Symbol LISTENERS;
Symbol LISTENER;
Symbol EXPECTATION;
Symbol ACTION;
Symbol INTERPOLATE_SYMBOL;
Symbol PROCESS_CODING;
Symbol PROCESS_NAME;
Symbol PROCESS_INTENTION;
Symbol INPUT;
Symbol INPUT_SIGNATURE;
Symbol INPUT_LABEL;
Symbol SIGNATURE_STRUCTURE;
Symbol OUTPUT_SIGNATURE;
Symbol RUN_TREE;
Symbol PARAM_REF;
Symbol PARAMS;
Symbol PROTOCOLS;
Symbol PROTOCOL;
Symbol ROLES;
Symbol ROLE;
Symbol INTERACTIONS;
Symbol INTERACTION;
Symbol STEP;
Symbol FROM_ROLE;
Symbol TO_ROLE;
Symbol RESPONSE_STEPS;
Symbol SCAPES;
Symbol VM_HOST_RECEPTOR;
Symbol COMPOSITORY;
Symbol MANIFEST;
Symbol MANIFEST_PAIR;
Symbol MANIFEST_LABEL;
Symbol MANIFEST_SPEC;
Symbol RECEPTOR_PACKAGE;
Symbol RECEPTOR_IDENTIFIER;
Symbol INSTALLED_RECEPTOR;
Symbol ACTIVE_RECEPTORS;
Symbol PENDING_SIGNALS;
Symbol BINDINGS;
Symbol BINDING_PAIR;

Symbol STX_TOKENS;
Symbol STX_SL;
Symbol STX_OP;
Symbol STX_CP;
Symbol STX_SET;
Symbol STX_OS;
Symbol STX_CS;
Symbol STX_LABEL;
Symbol STX_OG;
Symbol STX_CG;
Symbol STX_EQ;
Symbol STX_NEQ;
Symbol STX_WALK;
Symbol STX_STAR;
Symbol STX_PLUS;
Symbol STX_Q;
Symbol STX_OR;
Symbol STX_COMMA;
Symbol STX_EXCEPT;
Symbol STX_NOT;
Symbol STX_VAL_C;
Symbol STX_VAL_S;
Symbol STX_VAL_I;

Symbol STX_SIBS;
Symbol STX_CHILD;
Symbol STX_POSTFIX;
