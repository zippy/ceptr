/**
 * @file http_example.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_HTTP_EXAMPLE_H
#define _CEPTR_TEST_HTTP_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"
#include "../src/semtrex.h"

Symbol OCTET_STREAM;

Structure HTTP_REQUEST_V09;

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

T *test_HTTP_symbols,*test_HTTP_structures;
Defs test_HTTP_defs;

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

Structure HTML_ELEMENT;
Symbol HTML_ATTRIBUTES;
Symbol HTML_ATTRIBUTE;
Symbol HTML_CONTENT;
Symbol HTML_TEXT;
Symbol HTML_HTML;
Symbol HTML_HEAD;
Symbol HTML_TITLE;
Symbol HTML_BODY;
Symbol HTML_DIV;
Symbol HTML_P;
Symbol HTML_IMG;
Symbol HTML_A;
Symbol HTML_UL;
Symbol HTML_OL;
Symbol HTML_LI;
Symbol HTML_SPAN;
Symbol HTML_H1;
Symbol HTML_H2;
Symbol HTML_H3;
Symbol HTML_H4;
Symbol HTML_FORM;
Symbol HTML_INPUT;
Symbol HTML_BUTTON;

//semtrex html symbols

Symbol HTML_TOKENS;
Symbol HTML_TOK_TAG_OPEN;
Symbol HTML_TOK_TAG_CLOSE;
Symbol HTML_TOK_TAG_SELFCLOSE;
Symbol HTML_TAG;

void _setup_html_defs(Defs d) {
    SY(d,HTML_TOKENS,LIST);
    SY(d,HTML_TOK_TAG_OPEN,CSTRING);
    SY(d,HTML_TOK_TAG_CLOSE,CSTRING);
    SY(d,HTML_TOK_TAG_SELFCLOSE,CSTRING);
    SY(d,HTML_TAG,CSTRING);


    SY(d,HTML_ATTRIBUTES,LIST);
    SY(d,HTML_ATTRIBUTE,KEY_VALUE_PARAM);
    SY(d,HTML_CONTENT,LIST);  // really should be semtrex: /(HTML_ELEMENT|HTML_TEXT)+
    SY(d,HTML_TEXT,CSTRING);
    ST(d,HTML_ELEMENT,2,
       HTML_ATTRIBUTES,
       HTML_CONTENT
       );
    SY(d,HTML_HTML,HTML_ELEMENT);
    SY(d,HTML_HEAD,HTML_ELEMENT);
    SY(d,HTML_TITLE,HTML_ELEMENT);
    SY(d,HTML_BODY,HTML_ELEMENT);
    SY(d,HTML_DIV,HTML_ELEMENT);
    SY(d,HTML_P,HTML_ELEMENT);
    SY(d,HTML_IMG,HTML_ELEMENT);
    SY(d,HTML_A,HTML_ELEMENT);
    SY(d,HTML_UL,HTML_ELEMENT);
    SY(d,HTML_OL,HTML_ELEMENT);
    SY(d,HTML_LI,HTML_ELEMENT);
    SY(d,HTML_SPAN,HTML_ELEMENT);
    SY(d,HTML_H1,HTML_ELEMENT);
    SY(d,HTML_H2,HTML_ELEMENT);
    SY(d,HTML_H3,HTML_ELEMENT);
    SY(d,HTML_H4,HTML_ELEMENT);
    SY(d,HTML_FORM,HTML_ELEMENT);
    SY(d,HTML_INPUT,HTML_ELEMENT);
    SY(d,HTML_BUTTON,HTML_ELEMENT);
}

void _setup_HTTPDefs() {
    test_HTTP_defs.symbols = test_HTTP_symbols = _t_new_root(SYMBOLS);
    test_HTTP_defs.structures = test_HTTP_structures = _t_new_root(STRUCTURES);
    Defs d = test_HTTP_defs;

    SY(d,OCTET_STREAM,CSTRING);

    SY(d,HTTP_REQUEST_METHOD,CSTRING);

    _setup_uri_defs(d);
    SY(d,HTTP_REQUEST_PATH,URI);

    _setup_version_defs(d);
    SY(d,HTTP_REQUEST_VERSION,VERSION);

    ST(d,HTTP_REQUEST_V09,3,HTTP_REQUEST_VERSION,HTTP_REQUEST_METHOD,HTTP_REQUEST_PATH);
    SY(d,HTTP_REQUEST,HTTP_REQUEST_V09);

    SY(d,HTTP_REQUEST_HOST,CSTRING);

    SY(d,HTTP_RESPONSE,TREE);
    SY(d,HTTP_RESPONSE_CONTENT_TYPE,CSTRING);
    SY(d,HTTP_RESPONSE_BODY,CSTRING);

    _setup_html_defs(d);
}

void _cleanup_HTTPDefs() {
    _t_free(test_HTTP_symbols);
    _t_free(test_HTTP_structures);
}


//! [makeTestHTTPRequestTree]
/**
 * generate a test semantic tree that represents an HTTP request
 *
 * @snippet spec/http_example.h makeTestHTTPRequestTree
 */
T *_makeTestHTTPRequestTree() {
    // manually build up a tree for the HTTP request:
    //     GET /groups/5/users.json?sort_by=last_name?page=2 HTTP/1.0
    // Note that we put the version at the beginning of our tree just because in ceptr we
    // do that as a best practice so that semtrex expectation matching can efficiently
    // switch to different processing based on version numbers.
    T *t = _t_new_root(HTTP_REQUEST);
    T *t_version = _t_newr(t,HTTP_REQUEST_VERSION);
    _t_newi(t_version,VERSION_MAJOR,1);
    _t_newi(t_version,VERSION_MINOR,0);
    T *t_method = _t_new(t,HTTP_REQUEST_METHOD,"GET",4);
    T *t_path = _t_newr(t,HTTP_REQUEST_PATH);
    T *t_segments = _t_newr(t_path,HTTP_REQUEST_PATH_SEGMENTS);
    _t_new(t_segments,HTTP_REQUEST_PATH_SEGMENT,"groups",7);
    _t_new(t_segments,HTTP_REQUEST_PATH_SEGMENT,"5",2);
    T *t_file = _t_newr(t_path,HTTP_REQUEST_PATH_FILE);
    _t_new(t_file,FILE_NAME,"users",6);
    _t_new(t_file,FILE_EXTENSION,"json",5);
    T *t_query = _t_newr(t_path,HTTP_REQUEST_PATH_QUERY);
    T *t_params = _t_newr(t_query,HTTP_REQUEST_PATH_QUERY_PARAMS);
    T *t_param1 = _t_newr(t_params,HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param1,PARAM_KEY,"sort_by",8);
    _t_new(t_param1,PARAM_VALUE,"last_name",10);
    T *t_param2 = _t_newr(t_params,HTTP_REQUEST_PATH_QUERY_PARAM);
    _t_new(t_param2,PARAM_KEY,"page",5);
    _t_new(t_param2,PARAM_VALUE,"2",2);

    // confirm that we built the request right!
     T *stx = _d_build_def_semtrex(test_HTTP_defs,HTTP_REQUEST,0);

     if (!_t_match(stx,t)) {raise_error0("BAD HTTP_REQUEST semtrex");}
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

    T *resp = _t_new_root(RESPOND);
    T *n = _t_newr(resp,INTERPOLATE_FROM_MATCH);
    T *http_resp = _t_newr(n,HTTP_RESPONSE);
    _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT",38);
    _t_news(http_resp,INTERPOLATE_SYMBOL,HTTP_REQUEST_PATH_SEGMENT);

    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,2,TREE_PATH_TERMINATOR};

    _t_new(n,PARAM_REF,pt1,sizeof(int)*3);
    _t_new(n,PARAM_REF,pt2,sizeof(int)*3);

    T *input = _t_new_root(INPUT);
    T *output = _t_new_root(OUTPUT_SIGNATURE);
    Process p = _r_code_process(r,resp,"code path ping","respond with the first segment of the code path",input,output);
    return p;
}
//! [makeTestHTTPResponseProcess]

/**
 * generate a semtrex that will match a valid HTTP request in an ASCII_CHARS tree
 *
 */
T *_makeHTTPRequestSemtrex() {
    T *t;

    T *stx= _sl(0,ASCII_CHARS);
    t = _t_news(stx,SEMTREX_GROUP,HTTP_REQUEST);
    T *sq = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_METHOD);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    __stxcvm(t,1,1,' ');  //!= ' '
    __stxcv(sq,' ');

    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_PATH);
    T *sqq = _t_newr(t,SEMTREX_SEQUENCE);

    t = _t_news(sqq,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENTS);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(t,'/');
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_SEGMENT);
    t = _t_newr(t,SEMTREX_ZERO_OR_MORE);
    __stxcvm(t,1,3,'/','?',' ');

    /* t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_PATH_FILE); */
    /* T *f = _t_newr(t,SEMTREX_SEQUENCE); */
    /* t = _t_news(f,SEMTREX_GROUP,FILE_NAME); */
    /* t = _t_newr(t,SEMTREX_ONE_OR_MORE); */
    /* __stxcvm(t,1,2,'?',' '); */

    /* t = _t_newr(f,SEMTREX_ZERO_OR_ONE); */
    /* t = _t_newr(t,SEMTREX_SEQUENCE); */
    /* __stxcv(t,'.'); */
    /* t = _t_news(t,SEMTREX_GROUP,FILE_EXTENSION); */
    /* t = _t_newr(t,SEMTREX_ONE_OR_MORE); */
    /* __stxcvm(t,1,2,'?',' '); */

    t = _t_newr(sq,SEMTREX_ZERO_OR_ONE);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    __stxcv(t,'?');

    T *f;
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_QUERY);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    t = _t_news(t,SEMTREX_GROUP,HTTP_REQUEST_PATH_QUERY_PARAMS);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    T *qps = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(qps,SEMTREX_GROUP,HTTP_REQUEST_PATH_QUERY_PARAM);
    f = _t_newr(t,SEMTREX_SEQUENCE);
    t = _t_news(f,SEMTREX_GROUP,PARAM_KEY);
    t = _t_newr(t,SEMTREX_ONE_OR_MORE);
    __stxcvm(t,1,3,'&',' ','=');
    __stxcv(f,'=');
    t = _t_news(f,SEMTREX_GROUP,PARAM_VALUE);
    t = _t_newr(t,SEMTREX_ZERO_OR_MORE);
    __stxcvm(t,1,2,'&',' ');

    t = _t_newr(qps,SEMTREX_ZERO_OR_ONE);
    __stxcv(t,'&');

    __stxcv(sq,' ');

    __stxcv(sq,'H');
    __stxcv(sq,'T');
    __stxcv(sq,'T');
    __stxcv(sq,'P');
    __stxcv(sq,'/');
    t = _t_news(sq,SEMTREX_GROUP,HTTP_REQUEST_VERSION);
    t = _t_newr(t,SEMTREX_SEQUENCE);
    f = _t_news(t,SEMTREX_GROUP,VERSION_MAJOR);
    __stxcv(f,'0');
    __stxcv(t,'.');
    f = _t_news(t,SEMTREX_GROUP,VERSION_MINOR);
    __stxcv(f,'9');
    return stx;
}

Symbol getTag(char *otag,Symbol tag_sym[],char *tag_str[]) {
    Symbol ts = NULL_SYMBOL;
    int i;
    for(i=0;i<15;i++) {
	if (!strcicmp(otag,tag_str[i])) {ts = tag_sym[i];break;}
    }
    if (semeq(ts,NULL_SYMBOL)) {raise_error("invalid tag: %s",otag);}
    return ts;
}

T *parseHTML(char *html) {
    Symbol G_tag_sym[] = {HTML_HTML,HTML_HEAD,HTML_TITLE,HTML_BODY,HTML_DIV,HTML_P,HTML_UL,HTML_OL,HTML_LI,HTML_SPAN,HTML_H1,HTML_H2,HTML_H3,HTML_H4,HTML_FORM};
    char *G_tag_str[] ={"html","head","title","body","div","p","ul","ol","li","span","h1","h2","h3","h4","form"};

    Symbol G_stag_sym[] = {HTML_IMG,HTML_INPUT,HTML_BUTTON};
    char *G_stag_str[] ={"img","input","button"};

    Defs *d = &test_HTTP_defs;
    T *t,*h = makeASCIITree(html);

    /////////////////////////////////////////////////////
    // build the token stream out of an ascii stream
    // EXPECTATION
    T *s;
    char *stx = "/ASCII_CHARS/<HTML_TOKENS:(ASCII_CHAR='<',<HTML_TOK_TAG_SELFCLOSE:ASCII_CHAR!={'>',' '}+>,<HTML_ATTRIBUTES:(ASCII_CHAR=' ',<HTML_ATTRIBUTE:<PARAM_KEY:ASCII_CHAR!={'>',' ','='}+>,ASCII_CHAR='=',ASCII_CHAR='\"',<PARAM_VALUE:ASCII_CHAR!='\"'+>,ASCII_CHAR='\"'>)*>,ASCII_CHAR='/',ASCII_CHAR='>'|ASCII_CHAR='<',ASCII_CHAR='/',<HTML_TOK_TAG_CLOSE:ASCII_CHAR!='>'+>,ASCII_CHAR='>'|ASCII_CHAR='<',<HTML_TOK_TAG_OPEN:ASCII_CHAR!={'>',' '}+>,<HTML_ATTRIBUTES:(ASCII_CHAR=' ',<HTML_ATTRIBUTE:<PARAM_KEY:ASCII_CHAR!={'>',' ','='}+>,ASCII_CHAR='=',ASCII_CHAR='\"',<PARAM_VALUE:ASCII_CHAR!='\"'+>,ASCII_CHAR='\"'>)*>,ASCII_CHAR='>'|<HTML_TEXT:ASCII_CHAR!='<'+>)+>";
    s = parseSemtrex(d,stx);
    T *results,*tokens;
    if (_t_matchr(s,h,&results)) {
	tokens = _t_new_root(HTML_TOKENS);
	int i,m = _t_children(results);
	for(i=4;i<=m;i++) {
	    T *c = _t_child(results,i);
	    T *sn = _t_child(c,1);
	    Symbol ts = *(Symbol *)_t_surface(sn);
	    if (semeq(ts,HTML_ATTRIBUTES)) {
		T *a = _t_new_root(HTML_ATTRIBUTES);
		int j,ac = _t_children(c);
		for(j=4;j<=ac;j++) {
		    T *attr = _t_newr(a,HTML_ATTRIBUTE);
		    T *at = _t_child(c,j);
		    T *m = _t_get_match(at,PARAM_KEY);
		    asciiT_tos(h,m,attr,PARAM_KEY);
		    m = _t_get_match(at,PARAM_VALUE);
		    asciiT_tos(h,m,attr,PARAM_VALUE);
		}
		// we can just add the attribute directly to the previous token which will be the open tag tokens
		_t_add(_t_child(tokens,_t_children(tokens)),a);
	    }
	    else {
		asciiT_tos(h,c,tokens,ts);
	    }
	}
	_t_free(results);
	_t_free(s);

	s = _t_new_root(SEMTREX_WALK);
	//	T *st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
	//	_t_newr(st,SEMTREX_SYMBOL_ANY);
	T *g = 	_t_news(s,SEMTREX_GROUP,HTML_TAG);
	T *sq = _t_newr(g,SEMTREX_SEQUENCE);
	_sl(sq,HTML_TOK_TAG_OPEN);
	g = _t_news(sq,SEMTREX_GROUP,HTML_CONTENT);
	T* st = _t_newr(g,SEMTREX_ZERO_OR_MORE);
	__sl(st,1,2,HTML_TOK_TAG_OPEN,HTML_TOK_TAG_CLOSE);
	_sl(sq,HTML_TOK_TAG_CLOSE);

	//	stx = "%<HTML_TAG:HTML_TOK_TAG_OPEN,!{HTML_TOK_TAG_OPEN,HTML_TOK_TAG_CLOSE},HTML_TOK_TAG_CLOSE>";
	//s = parseSemtrex(d,stx);
	//	return tokens;
	while (_t_matchr(s,tokens,&results)) {
	    T *m = _t_get_match(results,HTML_TAG);
	    int *path = _t_surface(_t_child(m,2));
	    int count = *(int *)_t_surface(_t_child(results,3));
	    T *ot = _t_get(tokens,path);
	    path[_t_path_depth(path)-1] += count-1;
	    T *ct = _t_get(tokens,path);
	    char *otag = _t_surface(ot);
	    char *ctag = _t_surface(ct);
	    if (strcmp(otag,ctag)) {raise_error2("Mismatched tags %s,%s",otag,ctag)};

	    Symbol ts = getTag(otag,G_tag_sym,G_tag_str);
	    path[_t_path_depth(path)-1] -= count-1;
	    T *content = wrap(tokens,results,HTML_CONTENT,HTML_TAG);
	    T *attributes = _t_detach_by_idx(content,1);
	    __t_morph(content,HTML_CONTENT,0,0,0);
	    T *p = _t_parent(content);
	    _t_detach_by_ptr(p,content);
	    T *tag = _t_new_root(ts);
	    _t_add(tag,attributes);
	    _t_add(tag,content);
	    _t_insert_at(tokens,path,tag);
	    _t_free(results);
	}
	_t_free(s);

	s = _t_new_root(SEMTREX_WALK);
	g = _t_news(s,SEMTREX_GROUP,HTML_TAG);
	_sl(g,HTML_TOK_TAG_SELFCLOSE);
	while (_t_matchr(s,tokens,&results)) {
	    T *m = _t_get_match(results,HTML_TAG);
	    int *path = _t_surface(_t_child(m,2));
	    T *t = _t_get(tokens,path);
	    char *otag = _t_surface(t);
	    Symbol ts = getTag(otag,G_stag_sym,G_stag_str);
	    __t_morph(t,ts,0,0,0);
	    _t_newr(t,HTML_CONTENT);
	}
	_t_free(s);
	return _t_child(tokens,1);
    }
    raise_error0("HTML doesn't match");
}

#endif
//
