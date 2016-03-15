/**
 * @file stream_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/stream.h"
#include <unistd.h>
#include <errno.h>

void testStreamCreate() {
    // test basic stream creation
    Stream *s = _st_new_unix_stream(stdout,0);
    spec_is_equal(s->type,UnixStream);
    spec_is_equal(s->flags,StreamCloseOnFree);
    spec_is_ptr_equal(s->data.unix_stream,stdout);
    s->flags &= ~StreamCloseOnFree; // don't close the stdin on free...
    _st_free(s);
}

void testStreamAlive() {
    FILE *input;

    char data[] = "line1\nline2\n";
    input = fmemopen(data, strlen(data), "r");
    Stream *s = _st_new_unix_stream(input,1);

    spec_is_true(_st_is_alive(s));
    _st_kill(s);
    spec_is_false(_st_is_alive(s));

    void *status;
    int rc;

    rc = pthread_join(s->pthread, &status);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }
    _st_free(s);
}


void testStreamScan() {

    Stream * s = __st_alloc_stream();
    s->type = 99;
    s->buf_size = 100;

    s->buf = "line1\ncat\ndog";
    s->bytes_used = strlen(s->buf);

    __st_init_scan(s);
    spec_is_equal(s->scan_state,StreamScanInitial);

    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanSuccess);
    spec_is_equal(s->unit_start,0);
    spec_is_equal(s->unit_size,5);
    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanSuccess);
    spec_is_equal(s->unit_start,6);
    spec_is_equal(s->unit_size,3);
    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanPartial);

    // repeat calls to scan continue with Partial result
    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanPartial);


    // simulate reading in some extra bytes into the buffer (but not enough)
    s->buf = "line1\ncat\ndogg";  s->bytes_used = strlen(s->buf);

    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanPartial);

    // simulate reading in more bytes plus newline
    s->buf = "line1\ncat\ndoggy\n";  s->bytes_used = strlen(s->buf);

    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanSuccess);
    spec_is_equal(s->unit_start,10);
    spec_is_equal(s->unit_size,5);

    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanComplete);

    s->buf = "";
    s->bytes_used = strlen(s->buf);

    __st_init_scan(s);
    __st_scan(s);
    spec_is_equal(s->scan_state,StreamScanComplete);


    _st_free(s);

}

void testStreamFileLoad() {
    FILE *input;
    //debug_enable(D_STREAM);

    char data[] = "line1\nline2\n";
    input = fmemopen(data, strlen(data), "r");
    Stream *s = _st_new_unix_stream(input,false);
    // manually allocate a buffer for reading
    s->buf = malloc(100);
    s->buf_size = 99;
    memset(s->buf,0,100);

    spec_is_equal(__st_unix_stream_load(s),strlen(data));
    spec_is_str_equal(s->buf,data);
    spec_is_equal(s->bytes_used,strlen(data));
    spec_is_equal(errno,0);
    spec_is_equal(s->err,0);

    // test reading after eof
    spec_is_equal(__st_unix_stream_load(s),0);
    spec_is_equal(errno,0);
    spec_is_equal(s->err,EOF);

    free(s->buf);
    _st_free(s);

    // redo with a small buffer
    input = fmemopen(data, strlen(data), "r");
    s = _st_new_unix_stream(input,false);

    // manually allocate a buffer for reading
    s->buf = malloc(11);
    s->buf_size = 10;
    memset(s->buf,0,11);

    spec_is_equal(__st_unix_stream_load(s),10);
    spec_is_str_equal(s->buf,"line1\nline");
    spec_is_true(__st_buf_full(s));
    spec_is_equal(s->bytes_used,10);
    spec_is_equal(errno,0);
    spec_is_equal(s->err,0);

    spec_is_equal(__st_unix_stream_load(s),2);
    s->buf[s->bytes_used] = 0;
    spec_is_str_equal(s->buf,data);
    spec_is_false(__st_buf_full(s));
    spec_is_equal(s->buf_size,20);
    spec_is_equal(s->bytes_used,strlen(data));
    spec_is_equal(errno,0);
    spec_is_equal(s->err,0);

    free(s->buf);
    _st_free(s);
    debug_disable(D_STREAM);
}

void testCallback(Stream *st) {
    spec_is_buffer_equal(_st_data(st),"line2",_st_data_size(st));
    spec_is_str_equal((char *)st->callback_arg1,"arg val");
    st->callback = 0;
}

// test reading from a file stream
void testStreamRead(size_t rs) {
    //debug_enable(D_STREAM);
    FILE *input;

    char data[] = "line1\nline2\nline3";
    input = fmemopen(data, strlen(data), "r");
    Stream *s = __st_new_unix_stream(input,rs);
    spec_is_true(s->flags&StreamReader);
    spec_is_true(s->flags&StreamWaiting);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && _st_is_alive(s)) {sleepms(1);};
    spec_is_buffer_equal(_st_data(s),"line1",_st_data_size(s));
    _st_data_consumed(s);
    spec_is_false(s->flags&StreamHasData);
    spec_is_true(_st_is_alive(s));

    // test callback getting the value
    s->callback = testCallback;
    s->callback_arg1 = "arg val";
    _st_start_read(s);
    while(!(s->flags&StreamHasData) && _st_is_alive(s) ) {sleepms(1);};
    spec_is_true(s->flags&StreamHasData);
    _st_data_consumed(s);
    spec_is_false(s->flags&StreamHasData);

    // test final value with no new-line
    _st_start_read(s);
    while(!(s->flags&StreamHasData) && _st_is_alive(s) ) {sleepms(1);};
    spec_is_true(s->flags&StreamHasData);
    spec_is_buffer_equal(_st_data(s),"line3",_st_data_size(s));
    _st_data_consumed(s);
    spec_is_false(s->flags&StreamHasData);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && _st_is_alive(s) ) {sleepms(1);};
    spec_is_false(s->flags&StreamHasData);
    spec_is_false(_st_is_alive(s));

    debug_disable(D_STREAM);

    void *status;
    int rc;

    rc = pthread_join(s->pthread, &status);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }
    _st_free(s);
}

void testStreamWrite() {
    char buffer[500] = "x";
    FILE *stream;
    stream = fmemopen(buffer, 500, "r+");
    Stream *st = _st_new_unix_stream(stream,1);

    spec_is_equal(_st_write(st,"fishy",6),6);

    char *expected_result = "fishy";
    spec_is_str_equal(buffer,expected_result);

    _st_free(st);
}

void testSocketListernCallback(Stream *s,void *arg) {
    spec_is_true(s->flags&StreamReader);
    spec_is_true(s->flags&StreamWaiting);
    spec_is_equal(*(int *)arg,31415);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && s->flags&StreamAlive ) {sleepms(1);};
    spec_is_buffer_equal(_st_data(s),"testing!",_st_data_size(s));
    _st_data_consumed(s);

    _st_start_read(s);
    while(!(s->flags&StreamHasData) && s->flags&StreamAlive ) {sleepms(1);};
    spec_is_buffer_equal(_st_data(s),"fish",_st_data_size(s));
    _st_data_consumed(s);
    _st_write(s,"fishy",6);
    _st_free(s);
}

void testStreamSocket() {
    //    debug_enable(D_SOCKET+D_STREAM);

    int arg = 31415;
    SocketListener *l = _st_new_socket_listener(8888,testSocketListernCallback,&arg);
    char *result = doSys("echo 'testing!\nfish\n' | nc localhost 8888");
    spec_is_str_equal(result,"fishy");
    free(result);

    result = doSys("echo 'testing!\nfish\n' | nc localhost 8888");
    spec_is_str_equal(result,"fishy");
    free(result);

    _st_close_listener(l);

    debug_disable(D_SOCKET+D_STREAM);
}

void testStream() {
    testStreamCreate();
    testStreamAlive();
    testStreamScan();
    testStreamFileLoad();
    testStreamRead(1000);
    testStreamRead(10);
    testStreamRead(2);
    testStreamWrite();
    testStreamSocket();
}
