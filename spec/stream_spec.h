/**
 * @file stream_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/stream.h"
#include <unistd.h>

void testCallback(Stream *st) {
    spec_is_str_equal(_st_data(st),"line2");
    spec_is_str_equal((char *)st->callback_arg1,"arg val");
    st->callback = 0;
}

void testStream() {

    // test basic stream creation
    Stream *s = _st_new_unix_stream(stdout,0);
    spec_is_equal(s->type,UnixStream);
    spec_is_equal(s->flags,StreamCloseOnFree);
    spec_is_ptr_equal(s->data.unix_stream,stdout);
    s->flags &= ~StreamCloseOnFree; // don't close the stdin on free...
    _st_free(s);

    //    debug_enable(D_STREAM);
    // test reading from a stream
    FILE *input;

    char data[] = "line1\nline2\n";
    input = fmemopen(data, strlen(data), "r");
    s = _st_new_unix_stream(input,1);
    spec_is_true(s->flags&StreamReader);
    spec_is_true(s->flags&StreamWaiting);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && s->flags&StreamAlive ) {sleepms(1);};
    spec_is_str_equal(_st_data(s),"line1");
    spec_is_equal(_st_data_size(s),6);
    _st_data_read(s);
    spec_is_false(s->flags&StreamHasData);

    s->callback = testCallback;
    s->callback_arg1 = "arg val";
    _st_start_read(s);
    while(!(s->flags&StreamHasData) && s->flags&StreamAlive ) {sleepms(1);};
    spec_is_true(s->flags&StreamHasData);
    _st_data_read(s);
    spec_is_false(s->flags&StreamHasData);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && s->flags&StreamAlive ) {sleepms(1);};
    spec_is_false(s->flags&StreamHasData);
    spec_is_false(s->flags&StreamAlive);

    debug_disable(D_STREAM);

    void *status;
    int rc;

    rc = pthread_join(s->pthread, &status);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }
    _st_free(s);
}
