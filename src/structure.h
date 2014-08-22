#ifndef _CEPTR_STRUCTURE_H
#define _CEPTR_STRUCTURE_H

/**
 * @file structure.h
 * @brief Defines hard-coded system defined structures.
 *
 * Structures are definitons of how data is organized
 *
 * This file defines the hard-coded (negative value) system defined structures
 */

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
