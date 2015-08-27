/**
 * @file debug.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */


#ifndef CEPTR_DEBUG_H
#define CEPTR_DEBUG_H

#define CEPTR_DEBUG
#ifdef CEPTR_DEBUG

#define D_NONE       0x0000
#define D_ALL        0xFFFF
#define D_SPEC       0x8000
#define D_SIGNALS    0x0001
#define D_REDUCE     0x0002
#define D_REDUCEV    0x0004
#define D_STX_MATCH  0x0008
#define D_CLOCK      0x0010
#define D_STREAM     0x0020

#define DEBUG_DEFAULT_MASK D_NONE
//#define DEBUG_DEFAULT_MASK D_SIGNALS

#ifndef DEBUG_MASK
#define DEBUG_MASK 0
#endif

#include <stdio.h>

#define debugging(type) (type&G_debug_mask)

char *dtypestr(int type);
void debug_enable(int type);
void debug_disable(int type);
int G_debug_mask;


#define _debug(type,...) do {fprintf(stderr,"D%s: ",dtypestr(type));fprintf(stderr,__VA_ARGS__);}while(0);
#define debug(type,...) do {if debugging(type) _debug(type,__VA_ARGS__);} while(0);

#else

#define debug(type,...)
#endif

#endif
