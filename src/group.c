/**
 * @ingroup receptor
 *
 * @{
 * @file group.c
 * @brief implementation of some group communication receptors
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "group.h"

Receptor *makeGroup(VMHost *v,char *label) {
    Symbol group = _r_declare_symbol(v->r,RECEPTOR,label);
    Receptor *r = _r_new(group);
    Xaddr shellx = _v_new_receptor(v,v->r,group,r);
    return r;
}

/** @}*/
