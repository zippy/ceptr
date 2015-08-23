/**
 * @file debug.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */


#ifndef CEPTR_DEBUG_H
#define CEPTR_DEBUG_H

#define CEPTR_DEBUG
#ifdef CEPTR_DEBUG

#define D_ALL 0xFFFF
#define D_SIGNALS 0x0001

#define DEBUG_MASK D_SIGNALS

#ifndef DEBUG_MASK
#define DEBUG_MASK 0
#endif

#include <stdio.h>
#define _debug(...) do {fprintf(stderr,"DEBUG: ");fprintf(stderr,__VA_ARGS__);}while(0);
#define debug(type,...) do {if (type&DEBUG_MASK) _debug(__VA_ARGS__);} while(0);

#else

#define debug(type,...)
#endif

#endif
