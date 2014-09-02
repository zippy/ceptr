/**
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @file label.h
 * @brief Label tables map human readable text strings to symbols and structures.
 *
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
