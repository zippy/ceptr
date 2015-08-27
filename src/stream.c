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

/**
 * create a new stream object of the unix flavor
 */
Stream *_st_new_unix_stream(FILE *stream) {
    Stream *s = malloc(sizeof(Stream));
    s->type = UnixStream;
    s->flags = StreamHasData + StreamCloseOnFree;
    s->data.unix_stream = stream;
    return s;
}

/**
 * release the memory of a stream object
 *
 */
void _st_free(Stream *stream) {
    if (stream->flags & StreamCloseOnFree) {
        fclose(stream->data.unix_stream);
    }
    free(stream);
}
