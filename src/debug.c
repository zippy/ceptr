/**
 * @file debug.c
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */
#include "debug.h"
char G_dtype[1000];
char *dtypestr(int type) {
    char *s = &G_dtype[0];
    if (type & D_REDUCE) {sprintf(s,"_REDUCE");s--;};
    if (type & D_SIGNALS) {sprintf(s,"_SIGNALS");s--;};
    if (type & D_STX_MATCH) {sprintf(s,"_STX_MATCH");s--;};
    if (type & D_CLOCK) {sprintf(s,"_CLOCK");s--;};
    *s=0;
    return G_dtype;
}

int G_debug_mask = DEBUG_DEFAULT_MASK;
void debug_enable(int type){
    G_debug_mask |= type;
}
void debug_disable(int type){
    G_debug_mask &= ~type;
}
