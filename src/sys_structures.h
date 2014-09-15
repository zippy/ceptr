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

#define is_sys_structure(s) (s >= NULL_STRUCTURE && s < _LAST_SYS_STRUCTURE)

/// System defined structures
enum SystemStructure
    {
	NULL_STRUCTURE = 0x7fff0000,
	BOOLEAN,
	INTEGER,
	FLOAT,
	CSTRING,
	SYMBOL,
	TREE_PATH,
	XADDR,
	LIST,
	SURFACE,
	TREE,
	RECEPTOR,
	PROCESS,
	STRUCTURE,
	_LAST_SYS_STRUCTURE

    };

static char *G_sys_structure_names[] = {
    "NULL_STRUCTURE",
    "BOOLEAN",
    "INTEGER",
    "FLOAT",
    "CSTRING",
    "SYMBOL",
    "TREE_PATH",
    "XADDR",
    "LIST",
    "SURFACE",
    "TREE",
    "RECEPTOR",
    "PROCESS",
    "STRUCTURE"
};
#endif
