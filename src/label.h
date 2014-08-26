/**
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @file label.h
 * @brief Label tables map human readable text strings to symbols and structures.
 *
 */

#ifndef _CEPTR_LABEL_H
#define _CEPTR_LABEL_H
#include <stdint.h>
#include "uthash.h"

typedef uint32_t Label;
#define MAX_PATH_DEPTH 10

/**
 * An element in the label table.
 *
 * @todo convert path to variable sized item
 */
struct table_elem {
    Label label;               ///< semantic key
    int path[MAX_PATH_DEPTH];  ///< path to the labeled item in the Receptor tree
    UT_hash_handle hh;         ///< makes this structure hashable using the uthash library
};

typedef struct table_elem table_elem;
typedef table_elem *LabelTable;

Label str2label(char *);
void labelSet(LabelTable *table, char *,int *path);
void _labelSet(LabelTable *table, Label l,int *path);
int *labelGet(LabelTable *table, char * l);
int *_labelGet(LabelTable *table, Label l);

void lableTableFree(LabelTable *table);

#endif
