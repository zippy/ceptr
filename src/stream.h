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
#include <stdbool.h>

enum StreamTypes {UnixStream,SocketStream};
enum {StreamHasData=0x0001,StreamCloseOnFree=0x0002,StreamReader=0x0004,StreamWaiting=0x0008,StreamAlive=0x8000};

typedef struct Stream Stream;

typedef void (*hasDataCallbackFn)(Stream *);

enum ScanStates {StreamScanInitial,StreamScanSuccess,StreamScanPartial,StreamScanComplete};
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
    size_t buf_size;
    size_t bytes_used;
    int scan_state;
    size_t unit_start;
    size_t unit_size;
    size_t partial;
    hasDataCallbackFn callback;
    void *callback_arg1;
    int callback_arg2;
    // hold data for the different types of streams that are implemented
    // for now just unix_streams
    union {
        FILE *unix_stream;
        int socket_stream;
    } data;
};

typedef struct SocketListener SocketListener;
typedef void (*lisenterConnectionCallbackFn)(Stream *);

struct SocketListener {
    int port;
    int sockfd;
    pthread_t pthread;
    lisenterConnectionCallbackFn callback;
};
#define DEFAULT_READER_BUFFER_SIZE 1000
#define _st_new_unix_stream(s,r) __st_new_unix_stream(s,r?DEFAULT_READER_BUFFER_SIZE:0)
Stream *__st_new_unix_stream(FILE *stream,size_t reader_buffer_size);
Stream *__st_alloc_stream();
#define __st_init_scan(s) s->scan_state = StreamScanInitial
#define __st_buf_full(s) (s->bytes_used == s->buf_size)

void __st_scan(Stream *st);

SocketListener *_st_new_socket_listener(int port,lisenterConnectionCallbackFn fn);
void _st_close_listener(SocketListener *l);

void _st_start_read(Stream *st);
void _st_data_consumed(Stream *st);
#define _st_is_alive(st) ((st->flags & StreamAlive) || (st->scan_state != StreamScanComplete))
void _st_kill(Stream *st);
void _st_free(Stream *);
#define _st_data(st) (&(st)->buf[st->unit_start])
#define _st_data_size(st) (st)->unit_size

int _st_write(Stream *stream,char *buf,size_t len);

#endif
/** @}*/
