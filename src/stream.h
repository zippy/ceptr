/**
 * @ingroup stream
 *
 * @{
 * @file stream.h
 * @brief streams abstraction header file
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

#ifndef _CEPTR_STREAM_H
#define _CEPTR_STREAM_H

#include <stdio.h>
#include <pthread.h>

enum StreamTypes {UnixStream=123};
enum {StreamHasData=0x0001,StreamCloseOnFree=0x0002,StreamReader=0x0004,StreamWaiting=0x0008,StreamAlive=0x8000};

typedef struct Stream Stream;

typedef void (*hasDataCallbackFn)(Stream *);

/**
 * abstraction for a stream data type
 */
struct Stream {
    int type;
    int flags;
    int err;
    pthread_t pthread;
    pthread_mutex_t mutex;
    pthread_cond_t cv;
    char *buf;
    int buf_size;
    int data_size;
    hasDataCallbackFn callback;
    void *callback_arg1;
    int callback_arg2;
    // hold data for the different types of streams that are implemented
    // for now just unix_streams
    union {
        FILE *unix_stream;
    } data;
};

Stream *_st_new_unix_stream(FILE *stream,int reader);

void _st_start_read(Stream *st);
void _st_data_read(Stream *st);
void _st_kill(Stream *st);
void _st_free(Stream *);
#define _st_data(st) (st)->buf
#define _st_data_size(st) (st)->data_size

#endif
/** @}*/
