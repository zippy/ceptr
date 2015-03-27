/**
 * @ingroup tree
 * @file label.h
 * @brief Label tables map human readable text strings to parts of semantic trees.
 *
 * Labeling is used to priovide human readable references for symbols, structures,
 * processes (including locals/parameter references in process code) in receptor definitions,
 * but is a general process for providing semantic references to parts of semantic trees
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_LABEL_H
#define _CEPTR_LABEL_H
#include "ceptr_types.h"

Label str2label(char *);
void labelSet(LabelTable *table, char *,int *path);
void _labelSet(LabelTable *table, Label l,int *path);
int *labelGet(LabelTable *table, char * l);
int *_labelGet(LabelTable *table, Label l);

void lableTableFree(LabelTable *table);

#endif
