/**
 * @ingroup stream
 *
 * @{
 *
 * @file stream.c
 * @brief stream abstraction implementation
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "stream.h"
#include <stdlib.h>
#include <errno.h>
#include "process.h"
#include "debug.h"
#include "util.h"

// stream reading routine
void *__st_unix_stream_read(void *arg) {
    Stream *st = (Stream *) arg;
    do {
        debug(D_STREAM,"wating for read.\n");
        pthread_mutex_lock(&st->mutex);
        st->flags |= StreamAlive; // don't change the state until the mutex is locked
        st->flags |= StreamWaiting;
        pthread_cond_wait(&st->cv, &st->mutex);
        st->flags &= ~StreamWaiting;

        if (!(st->flags & StreamHasData) && (st->flags & StreamAlive)) {
            debug(D_STREAM,"starting read.\n");
            FILE *stream = st->data.unix_stream;
            int ch;
            char *buf = st->buf;
            int i = 0;
            st->err = 0;
            //@todo integrity checks?
            while ((ch = fgetc (stream)) != EOF && ch != '\n') {
                buf[i++] = ch;
                if (i == st->buf_size) {
                    st->buf_size *= 2;
                    debug(D_STREAM,"realloc read-buffer to %d\n",st->buf_size);
                    st->buf = realloc(st->buf,st->buf_size);
                }
            }
            debug(D_STREAM,"eof state after read: %d\n",feof(stream));
            if (ch == EOF) {
                st->flags &= ~StreamAlive;
                if (errno) {
                    st->err = unixErrnoReductionErr;
                    debug(D_STREAM,"Got unix error %d during READ\n",errno);
                }

                if (i) st->flags |= StreamHasData;

                debug(D_STREAM,"Got EOF during READ\n");
            }
            else {
                st->flags |= StreamHasData;
            }
            if (!st->err) {
                buf[i++]=0;
                st->data_size = i;
                debug(D_STREAM,"just read: %s\n",buf);
            }
        }
        else {
            if (st->flags & StreamHasData) debug(D_STREAM,"hmmm, stream already has data on read wakeup!\n");
            if (!(st->flags & StreamAlive)) debug(D_STREAM,"hmmm, stream dead on read wakeup!\n");
        }
        pthread_mutex_unlock(&st->mutex);
        if (st->callback) {
            (st->callback)(st);
        }
    } while (st->flags & StreamAlive);
    pthread_exit(NULL);
}


/**
 * create a new stream object of the unix flavor
 */
Stream *_st_new_unix_stream(FILE *stream,int reader) {
    Stream *s = malloc(sizeof(Stream));
    s->type = UnixStream;
    s->flags = StreamCloseOnFree;
    s->data.unix_stream = stream;
    s->err = 0;
    s->callback = NULL;
    if (reader) {
        s->flags |= StreamReader;

        s->buf = malloc(1000);
        s->buf_size = 1000;
        pthread_mutex_init(&(s->mutex), NULL);
        pthread_cond_init(&(s->cv), NULL);

        int rc = pthread_create(&s->pthread,0,__st_unix_stream_read,s);
        if (rc){
            raise_error("Error starting reader thread; return code from pthread_create() is %d\n", rc);
        }

        // wait for the the reader thread to block on wait
        while(!(s->flags & StreamAlive)) {
            sleepns(1);
        };
    }
    return s;
}

/**
 * wake the stream reader thread
*/
void _st_start_read(Stream *st) {
    if (st->flags & StreamHasData) {raise_error("stream data hasn't been consumed!");}
    debug(D_STREAM,"waking stream reader\n");
    pthread_mutex_lock(&st->mutex);
    pthread_cond_signal(&st->cv);
    pthread_mutex_unlock(&st->mutex);
}

/**
 * mark that data in a stream has been consumed
 */
void _st_data_read(Stream *st) {
    debug(D_STREAM,"marking data as read\n");
    st->flags &= ~StreamHasData;
}

/**
 * kill a stream
 *
 * if a reader stream is blocked and waiting, calls  _st_start_read so that
 * the reader thread can complete.
 */
void _st_kill(Stream *st) {
    st->flags &= ~StreamAlive;
    if (st->flags & StreamWaiting) {
        _st_start_read(st);
        while(st->flags & StreamWaiting) {sleepms(1);};
    }
}

/**
 * release the resources of a stream object
 *
 */
void _st_free(Stream *stream) {

    if (stream->flags & StreamCloseOnFree) {
        fclose(stream->data.unix_stream);
    }
    _st_kill(stream);
    //@todo who should clean up the mutexes??
    if (stream->flags & StreamReader) {
        debug(D_STREAM,"cleaning up reader\n");
        free(stream->buf);
        pthread_mutex_destroy(&stream->mutex);
        pthread_cond_destroy(&stream->cv);
    }
    free(stream);
}
