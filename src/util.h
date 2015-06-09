/**
 * @file util.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_UTIL_H
#define _CEPTR_UTIL_H

#include <ctype.h>
#include <stdio.h>
#include <time.h>

// some basic c utility functions.  Functions in this file have no dependencies on any other src files

void hexDump(char *desc, void *addr, int len);
int strcicmp(char const *a, char const *b);
void writeFile(char *fn,void *data,size_t size);
long diff_micro(struct timespec *start, struct timespec *end);

#endif
