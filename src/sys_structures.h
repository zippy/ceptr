/**
 * @file sys_structures.h
 * @brief Defines hard-coded system defined structures.
 *
 * Structures are definitons of how data is organized
 *
 * This file defines the hard-coded (negative value) system defined structures
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup def
 */

#ifndef _CEPTR_SYS_STRUCTURES_H
#define _CEPTR_SYS_STRUCTURES_H

#include "ceptr_types.h"

#define is_sys_structure(s) (is_structure(s) && (s.context == SYS_CONTEXT))

/// System defined structures
enum SystemStructureID
    {
	NULL_STRUCTURE_ID,
	BOOLEAN_ID,
	INTEGER_ID,
	FLOAT_ID,
	CSTRING_ID,
	SYMBOL_ID,
	ENUM_ID,
	TREE_PATH_ID,
	XADDR_ID,
	LIST_ID,
	SURFACE_ID,
	TREE_ID,
	RECEPTOR_ID,
	PROCESS_ID,
	STRUCTURE_ID,
	SCAPE_ID,
    };

static char *G_sys_structure_names[] = {
    "NULL_STRUCTURE",
    "BOOLEAN",
    "INTEGER",
    "FLOAT",
    "CSTRING",
    "SYMBOL",
    "ENUM",
    "TREE_PATH",
    "XADDR",
    "LIST",
    "SURFACE",
    "TREE",
    "RECEPTOR",
    "PROCESS",
    "STRUCTURE",
    "SCAPE"
};

Structure NULL_STRUCTURE;
Structure BOOLEAN;
Structure INTEGER;
Structure FLOAT;
Structure CSTRING;
Structure SYMBOL;
Structure ENUM;
Structure TREE_PATH;
Structure XADDR;
Structure LIST;
Structure SURFACE;
Structure TREE;
Structure RECEPTOR;
Structure PROCESS;
Structure STRUCTURE;
Structure SCAPE;
#endif
