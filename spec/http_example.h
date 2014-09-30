/**
 * @file http_example.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_HTTP_EXAMPLE_H
#define _CEPTR_TEST_HTTP_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"

Structure HTTP_REQUEST_V11;

Structure URI;
Symbol HTTP_REQUEST_PATH_SEGMENTS;
Symbol HTTP_REQUEST_PATH_SEGMENT;
Symbol HTTP_REQUEST_PATH_FILE;
Symbol FILE_NAME;
Symbol FILE_EXTENSION;
Structure FILE_HANDLE;

Symbol HTTP_REQUEST_PATH_QUERY;
Symbol HTTP_REQUEST_PATH_QUERY_PARAMS;
Symbol HTTP_REQUEST_PATH_QUERY_PARAM;
Structure KEY_VALUE_PARAM;
Symbol PARAM_KEY;
Symbol PARAM_VALUE;

Symbol HTTP_REQUEST;
Symbol HTTP_REQUEST_METHOD;
Symbol HTTP_REQUEST_PATH;
Symbol HTTP_REQUEST_HOST;

Structure VERSION;
Symbol HTTP_REQUEST_VERSION;
Symbol VERSION_MAJOR;
Symbol VERSION_MINOR;

Symbol HTTP_RESPONSE;
Symbol HTTP_RESPONSE_CONTENT_TYPE;
Symbol HTTP_RESPONSE_BODY;

Tnode *test_HTTP_symbols,*test_HTTP_structures;
Defs test_HTTP_defs;

//SY(HTTP_REQUEST,HTTP_REQUEST_V11)

void _setup_version_defs(Defs d) {
    SY(d,VERSION_MAJOR,INTEGER);
    SY(d,VERSION_MINOR,INTEGER);
    ST(d,VERSION,2,VERSION_MAJOR,VERSION_MINOR);
}

//@todo make this actually match request-URI rather than just the path
void _setup_uri_defs(Defs d) {
    SY(d,HTTP_REQUEST_PATH_SEGMENTS,LIST);
    SY(d,HTTP_REQUEST_PATH_SEGMENT,CSTRING);
    SY(d,FILE_NAME,CSTRING);
    SY(d,FILE_EXTENSION,CSTRING);
    ST(d,FILE_HANDLE,2,FILE_NAME,FILE_EXTENSION);
    SY(d,HTTP_REQUEST_PATH_FILE,FILE_HANDLE);
    SY(d,HTTP_REQUEST_PATH_QUERY,LIST);
    SY(d,HTTP_REQUEST_PATH_QUERY_PARAMS,LIST);
    SY(d,PARAM_KEY,CSTRING);
    SY(d,PARAM_VALUE,CSTRING);
    ST(d,KEY_VALUE_PARAM,2,PARAM_KEY,PARAM_VALUE);
    SY(d,HTTP_REQUEST_PATH_QUERY_PARAM,KEY_VALUE_PARAM);
    ST(d,URI,3,
       HTTP_REQUEST_PATH_SEGMENTS,
       HTTP_REQUEST_PATH_FILE,
       HTTP_REQUEST_PATH_QUERY
       );
}

void _setup_HTTPDefs() {
    test_HTTP_defs.symbols = test_HTTP_symbols = _t_new_root(SYMBOLS);
    test_HTTP_defs.structures = test_HTTP_structures = _t_new_root(STRUCTURES);
    Defs d = test_HTTP_defs;

    SY(d,HTTP_REQUEST_METHOD,INTEGER);

    _setup_uri_defs(d);
    SY(d,HTTP_REQUEST_PATH,URI);

    _setup_version_defs(d);
    SY(d,HTTP_REQUEST_VERSION,VERSION);

    ST(d,HTTP_REQUEST_V11,3,HTTP_REQUEST_VERSION,HTTP_REQUEST_METHOD,HTTP_REQUEST_PATH);
    SY(d,HTTP_REQUEST,HTTP_REQUEST_V11);

    SY(d,HTTP_REQUEST_HOST,CSTRING);

    SY(d,HTTP_RESPONSE,TREE);
    SY(d,HTTP_RESPONSE_CONTENT_TYPE,CSTRING);
    SY(d,HTTP_RESPONSE_BODY,CSTRING);

}

void _cleanup_HTTPDefs() {
    _t_free(test_HTTP_symbols);
    _t_free(test_HTTP_structures);
}

enum {TEST_HTTP_METHOD_GET_VALUE,TEST_HTTP_METHOD_PUT_VALUE,TEST_HTTP_METHOD_POST_VALUE};

//! [makeTestHTTPRequestTree]
/**
 * generate a test semantic tree that represents an HTTP request
 *
 * @snippet spec/http_example.h makeTestHTTPRequestTree
 */
Tnode *_makeTestHTTPRequestTree() {
    // manually build up a tree for the HTTP request:
    //     GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    // Note that we put the version at the beginning of our tree just because in ceptr we
    // do that as a best practice so that semtrex expectation matching can efficiently
    // switch to different processing based on version numbers.
    Tnode *t = _t_new_root(HTTP_REQUEST);
    Tnode *t_version = _t_newr(t,HTTP_REQUEST_VERSION);
    _t_newi(t_version,VERSION_MAJOR,1);
    _t_newi(t_version,VERSION_MINOR,0);
    Tnode *t_method = _t_newi(t,HTTP_REQUEST_METHOD,TEST_HTTP_METHOD_GET_VALUE);
    Tnode *t_path = _t_newr(t,HTTP_REQUEST_PATH);
    Tnode *t_segments = _t_newr(t_path,HTTP_REQUEST_PATH_SEGMENTS);
    _t_new(t_segments,HTTP_REQUEST_PATH_SEGMENT,"groups",7);
    _t_new(t_segments,HTTP_REQUEST_PATH_SEGMENT,"5",2);
    Tnode *t_file = _t_newr(t_path,HTTP_REQUEST_PATH_FILE);
    _t_new(t_file,FILE_NAME,"users",6);
    _t_new(t_file,FILE_EXTENSION,"json",5);
    Tnode *t_query = _t_newr(t_path,HTTP_REQUEST_PATH_QUERY);
    Tnode *t_params = _t_newr(t_query,HTTP_REQUEST_PATH_QUERY_PARAMS);
    Tnode *t_param1 = _t_newr(t_params,HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param1,PARAM_KEY,"sort_by",8);
    _t_new(t_param1,PARAM_VALUE,"last_name",10);
    Tnode *t_param2 = _t_newr(t_params,HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param2,PARAM_KEY,"page",5);
    _t_new(t_param2,PARAM_VALUE,"2",2);

    // confirm that we built the request right!
     Tnode *stx = _d_build_def_semtrex(test_HTTP_defs,HTTP_REQUEST,0);
     if (!_t_match(stx,t)) {raise_error0("BAD HTTP_REQUEST");}
     _t_free(stx);

    return t;
}
//! [makeTestHTTPRequestTree]


//! [makeTestHTTPResponseProcess]
/**
 * generate a test process that would generate a response to an HTTP request
 *
 * @snippet spec/http_example.h makeTestHTTPResponseProcess
 */
Process _makeTestHTTPResponseProcess(Receptor *r) {
    // the action simply responds back with the method that was originally sent
    // this test should be made more real... but for now it responds back with a ping
    // like message that contains what the first path segment was

    Tnode *resp = _t_new_root(RESPOND);
    Tnode *n = _t_newr(resp,INTERPOLATE_FROM_MATCH);
    Tnode *http_resp = _t_newr(n,HTTP_RESPONSE);
    _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT",38);
    _t_newi(http_resp,INTERPOLATE_SYMBOL,HTTP_REQUEST_PATH_SEGMENT);

    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,2,TREE_PATH_TERMINATOR};

    _t_new(n,PARAM_REF,pt1,sizeof(int)*3);
    _t_new(n,PARAM_REF,pt2,sizeof(int)*3);

    Tnode *input = _t_new_root(INPUT);
    Tnode *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _r_code_process(r,resp,"code path ping","respond with the first segment of the code path",input,output);
    return p;
}
//! [makeTestHTTPResponseProcess]
#endif
