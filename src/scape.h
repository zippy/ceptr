/**
 * @ingroup receptor
 *
 * @{
 * @file scape.h
 * @brief scape header files
 *
 * scapes organized and relate receptor contents
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_SCAPE_H
#define _CEPTR_SCAPE_H

#include "tree.h"

Scape *_s_new(Symbol key_source,Symbol data_source);
void _s_free(Scape *s);
void _s_add(Scape *s,TreeHash h,Xaddr x);
Xaddr _s_get(Scape *s,TreeHash h);

#endif
/** @}*/
