/**
 * @file tree_http_example.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_HTTP_EXAMPLE_H
#define _CEPTR_TEST_HTTP_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"

enum HTTPRequestTestSymbols {
    TSYM_HTTP_REQUEST,
    TSYM_HTTP_REQUEST_METHOD,
    TSYM_HTTP_REQUEST_PATH,
    TSYM_HTTP_REQUEST_PATH_SEGMENTS,
    TSYM_HTTP_REQUEST_PATH_SEGMENT,
    TSYM_HTTP_REQUEST_PATH_FILE,
    TSYM_HTTP_REQUEST_PATH_FILE_NAME,
    TSYM_HTTP_REQUEST_PATH_FILE_EXTENSION,
    TSYM_HTTP_REQUEST_PATH_QUERY,
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAMS,
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM,
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_KEY,
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_VALUE,
    TSYM_HTTP_REQUEST_VERSION,
    TSYM_HTTP_REQUEST_VERSION_MAJOR,
    TSYM_HTTP_REQUEST_VERSION_MINOR,
};
enum {TEST_HTTP_METHOD_GET_VALUE,TEST_HTTP_METHOD_PUT_VALUE,TEST_HTTP_METHOD_POST_VALUE};

/**
 * generate a test semantic tree that represents an HTTP request
 *
 * @snippet spec/tree_spec.h makeTestHTTPRequestTree
 */
//! [makeTestHTTPRequestTree]
Tnode *_makeTestHTTPRequestTree() {
    // manually build up a tree for the HTTP request:
    //     GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    // Note that we put the version at the beginning of our tree just because in ceptr we
    // do that as a best practice so that semtrex expectation matching can efficiently
    // switch to different processing based on version numbers.
    Tnode *t = _t_new_root(TSYM_HTTP_REQUEST);
    Tnode *t_version = _t_newr(t,TSYM_HTTP_REQUEST_VERSION);
    _t_newi(t_version,TSYM_HTTP_REQUEST_VERSION_MAJOR,1);
    _t_newi(t_version,TSYM_HTTP_REQUEST_VERSION_MINOR,0);
    Tnode *t_method = _t_newi(t,TSYM_HTTP_REQUEST_METHOD,TEST_HTTP_METHOD_GET_VALUE);
    Tnode *t_path = _t_newr(t,TSYM_HTTP_REQUEST_PATH);
    Tnode *t_segments = _t_newr(t_path,TSYM_HTTP_REQUEST_PATH_SEGMENTS);
    _t_new(t_segments,TSYM_HTTP_REQUEST_PATH_SEGMENT,"groups",7);
    _t_new(t_segments,TSYM_HTTP_REQUEST_PATH_SEGMENT,"5",2);
    Tnode *t_file = _t_newr(t_path,TSYM_HTTP_REQUEST_PATH_FILE);
    _t_new(t_file,TSYM_HTTP_REQUEST_PATH_FILE_NAME,"users",6);
    _t_new(t_file,TSYM_HTTP_REQUEST_PATH_FILE_EXTENSION,"json",5);
    Tnode *t_query = _t_newr(t_path,TSYM_HTTP_REQUEST_PATH_QUERY);
    Tnode *t_params = _t_newr(t_query,TSYM_HTTP_REQUEST_PATH_QUERY_PARAMS);
    Tnode *t_param1 = _t_newr(t_params,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param1,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_KEY,"sort_by",8);
    _t_new(t_param1,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_VALUE,"last_name",10);
    Tnode *t_param2 = _t_newr(t_params,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param2,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_KEY,"page",5);
    _t_new(t_param2,TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_VALUE,"2",2);
    return t;
}
//! [makeTestHTTPRequestTree]
#endif
