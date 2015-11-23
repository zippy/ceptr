/**
 * @ingroup receptor
 *
 * @{
 * @file protocol.h
 * @brief protocol helpers header file
 *
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_PROTOCOL_H
#define _CEPTR_PROTOCOL_H

#include "receptor.h"

T *_o_new(Receptor *r,Symbol protocol,...);

void _o_express_role(Receptor *r,Symbol protocol,Symbol role,Aspect aspect,T *action);

#endif
/** @}*/
