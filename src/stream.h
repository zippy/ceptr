/**
 * @ingroup stream
 *
 * @{
 * @file stream.h
 * @brief streams abstraction header file
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

#ifndef _CEPTR_STREAM_H
#define _CEPTR_STREAM_H

#include <stdio.h>

enum StreamTypes {UnixStream=123};
enum {StreamHasData=0x0001,StreamCloseOnFree=0x0002};

/**
 * abstraction for a stream data type
 */
typedef struct Stream {
    int type;
    int flags;

    // hold data for the different types of streams that are implemented
    // for now just unix_streams
    union {
        FILE *unix_stream;
    } data;
} Stream;

Stream *_st_new_unix_stream(FILE *stream);
void _st_free(Stream *);

#endif
/** @}*/
