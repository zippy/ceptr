/**
 * @file stream_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/stream.h"

void testStream() {
    Stream *s = _st_new_unix_stream(stdin);
    spec_is_equal(s->type,UnixStream);
    spec_is_equal(s->flags,StreamHasData+StreamCloseOnFree);
    spec_is_ptr_equal(s->data.unix_stream,stdin);
    s->flags &= ~StreamCloseOnFree; // don't close the stdin on free...

    _st_free(s);
}
