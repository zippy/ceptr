/**
 * @file http_example.h
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_HTTP_EXAMPLE_H
#define _CEPTR_TEST_HTTP_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"

Structure TSTR_HTTP_REQUEST;
Structure TSTR_HTTP_REQUEST_VERSION;
Structure TSTR_HTTP_REQUEST_PATH;

Symbol TSYM_HTTP_REQUEST;
Symbol TSYM_HTTP_REQUEST_METHOD;
Symbol TSYM_HTTP_REQUEST_PATH;
Symbol TSYM_HTTP_REQUEST_PATH_SEGMENTS;
Symbol TSYM_HTTP_REQUEST_PATH_SEGMENT;
Symbol TSYM_HTTP_REQUEST_PATH_FILE;
Symbol TSYM_HTTP_REQUEST_PATH_FILE_NAME;
Symbol TSYM_HTTP_REQUEST_PATH_FILE_EXTENSION;
Symbol TSYM_HTTP_REQUEST_PATH_QUERY;
Symbol TSYM_HTTP_REQUEST_PATH_QUERY_PARAMS;
Symbol TSYM_HTTP_REQUEST_PATH_QUERY_PARAM;
Symbol TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_KEY;
Symbol TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_VALUE;
Symbol TSYM_HTTP_REQUEST_VERSION;
Symbol TSYM_HTTP_REQUEST_VERSION_MAJOR;
Symbol TSYM_HTTP_REQUEST_VERSION_MINOR;

Symbol TSYM_HTTP_RESPONSE;
Symbol TSYM_HTTP_RESPONSE_CONTENT_TYPE;
Symbol TSYM_HTTP_RESPONSE_BODY;

Tnode *test_HTTP_symbols,*test_HTTP_structures;
Defs test_HTTP_defs;

void _setup_HTTPDefs() {
    test_HTTP_defs.symbols = test_HTTP_symbols = _t_new_root(SYMBOLS);
    test_HTTP_defs.structures = test_HTTP_structures = _t_new_root(STRUCTURES);

    TSYM_HTTP_REQUEST_METHOD = _d_declare_symbol(test_HTTP_symbols,INTEGER,"HTTP_REQUEST_METHOD");

    TSYM_HTTP_REQUEST_PATH_SEGMENTS = _d_declare_symbol(test_HTTP_symbols,LIST,"HTTP_REQUEST_PATH_SEGMENTS");
    TSYM_HTTP_REQUEST_PATH_SEGMENT = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_REQUEST_PATH_SEGMENT");
    TSYM_HTTP_REQUEST_PATH_FILE = _d_declare_symbol(test_HTTP_symbols,LIST,"HTTP_REQUEST_PATH_FILE");
    TSYM_HTTP_REQUEST_PATH_FILE_NAME = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_REQUEST_PATH_FILE_NAME");
    TSYM_HTTP_REQUEST_PATH_FILE_EXTENSION = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_REQUEST_PATH_FILE_EXTENSION");
    TSYM_HTTP_REQUEST_PATH_QUERY = _d_declare_symbol(test_HTTP_symbols,LIST,"HTTP_REQUEST_PATH_QUERY");
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAMS = _d_declare_symbol(test_HTTP_symbols,LIST,"HTTP_REQUEST_PATH_QUERY_PARAMS");
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM = _d_declare_symbol(test_HTTP_symbols,TREE,"HTTP_REQUEST_PATH_QUERY_PARAM");
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_KEY = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_REQUEST_PATH_QUERY_PARAM_KEY");
    TSYM_HTTP_REQUEST_PATH_QUERY_PARAM_VALUE = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_REQUEST_PATH_QUERY_PARAM_VALUE");

    TSTR_HTTP_REQUEST_PATH =  _d_define_structure(test_HTTP_structures,"HTTP_REQUEST_PATH_STRUCTURE",
						   TSYM_HTTP_REQUEST_PATH_SEGMENTS,
						   TSYM_HTTP_REQUEST_PATH_FILE,
						   TSYM_HTTP_REQUEST_PATH_QUERY
						   );

    TSYM_HTTP_REQUEST_PATH = _d_declare_symbol(test_HTTP_symbols,TSTR_HTTP_REQUEST_PATH,"HTTP_REQUEST_PATH");

    TSYM_HTTP_REQUEST_VERSION_MAJOR = _d_declare_symbol(test_HTTP_symbols,INTEGER,"HTTP_REQUEST_VERSION_MAJOR");
    TSYM_HTTP_REQUEST_VERSION_MINOR = _d_declare_symbol(test_HTTP_symbols,INTEGER,"HTTP_REQUEST_VERSION_MINOR");
    TSTR_HTTP_REQUEST_VERSION = _d_define_structure(test_HTTP_structures,"HTTP_REQUEST_VERSION_STRUCTURE",2,
					    TSYM_HTTP_REQUEST_VERSION_MAJOR,TSYM_HTTP_REQUEST_VERSION_MINOR);

    TSYM_HTTP_REQUEST_VERSION = _d_declare_symbol(test_HTTP_symbols,TSTR_HTTP_REQUEST_VERSION,"HTTP_REQUEST_VERSION");

    TSYM_HTTP_RESPONSE = _d_declare_symbol(test_HTTP_symbols,TREE,"HTTP_RESPONSE");
    TSYM_HTTP_RESPONSE = _d_declare_symbol(test_HTTP_symbols,TREE,"HTTP_RESPONSE");
    TSYM_HTTP_RESPONSE_CONTENT_TYPE = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_RESPONSE_CONTENT_TYPE");
    TSYM_HTTP_RESPONSE_BODY = _d_declare_symbol(test_HTTP_symbols,CSTRING,"HTTP_RESPONSE_BODY");

    TSTR_HTTP_REQUEST = _d_define_structure(test_HTTP_structures,"HTTP_REQUEST_STRUCTURE",3,
					    TSYM_HTTP_REQUEST_VERSION,
					    TSYM_HTTP_REQUEST_METHOD,
					    TSYM_HTTP_REQUEST_PATH
					    );
    TSYM_HTTP_REQUEST = _d_declare_symbol(test_HTTP_symbols,TREE,"HTTP_REQUEST");


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
    Tnode *http_resp = _t_newr(n,TSYM_HTTP_RESPONSE);
    _t_new(http_resp,TSYM_HTTP_RESPONSE_CONTENT_TYPE,"CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT",38);
    _t_newi(http_resp,INTERPOLATE_SYMBOL,TSYM_HTTP_REQUEST_PATH_SEGMENT);

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
