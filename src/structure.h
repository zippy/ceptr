/**
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 * @file structure.h
 * @brief Defines hard-coded system defined structures.
 *
 * Structures are definitons of how data is organized
 *
 * This file defines the hard-coded (negative value) system defined structures
 */

#ifndef _CEPTR_STRUCTURE_H
#define _CEPTR_STRUCTURE_H

/// System defined structures
enum SystemStructure
    {
	NULL_STRUCTURE = -999,
	SEMTREX,
	INTEGER,
	FLOAT,
	CSTRING,
	SURFACE,
	_LAST_SYS_STRUCTURE

    };
typedef int Structure;
static char *G_sys_structure_names[] = {
    "NULL_STRUCTURE",
    "SEMTREX",
    "INTEGER",
    "FLOAT",
    "CSTRING",
    "SURFACE"
};
#endif
