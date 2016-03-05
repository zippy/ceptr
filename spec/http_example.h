/**
 * @file http_example.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_HTTP_EXAMPLE_H
#define _CEPTR_TEST_HTTP_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/protocol.h"
#include "../src/def.h"
#include "../src/semtrex.h"
#include "spec_utils.h"
#include "../src/vmhost.h"
#include "../src/accumulator.h"
#include <unistd.h>

//! [makeTestHTTPRequestTree]
/**
 * generate a test semantic tree that represents an HTTP request
 *
 * @snippet spec/http_example.h makeTestHTTPRequestTree
 */
T *_makeTestHTTPRequestTree() {
    T *t = _t_clone(G_http_req_example);

    // confirm that we built the request right!
     T *stx = _d_build_def_semtrex(G_sem,HTTP_REQUEST,0);

     if (!_t_match(stx,t)) {raise_error("BAD HTTP_REQUEST semtrex");}
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
void _makeTestHTTPResponseProcess(Receptor *r,T **paramsP,Process *pP) {
    // the action simply responds back with the method that was originally sent
    // this test should be made more real... but for now it responds back with a ping
    // like message that contains what the first path segment was

    T *resp = _t_new_root(RESPOND);
    _t_news(resp,CARRIER,HTTP_RESPONSE);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(resp,PARAM_REF,pt1,sizeof(int)*3);

    Process p = _r_define_process(r,resp,"code path ping","respond with the value of the first parameter",NULL,NULL);
    *pP = p;

    T *params = _t_build(G_sem,0,
                         PARAMS,
                         HTTP_RESPONSE,
                         HTTP_RESPONSE_STATUS,STATUS_VALUE,200,STATUS_TEXT,"OK",NULL_SYMBOL,
                         HTTP_HEADERS,
                         CONTENT_TYPE,MEDIA_TYPE_IDENT,TEXT_MEDIA_TYPE,MEDIA_SUBTYPE_IDENT,CEPTR_TEXT_MEDIA_SUBTYPE,NULL_SYMBOL,NULL_SYMBOL,
                         HTTP_RESPONSE_BODY,
                         SLOT,USAGE,HTTP_REQUEST_PATH_SEGMENT,NULL_SYMBOL,
                         NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL
                         );


    /*     _t_new_root(PARAMS); */
    /* T *http_resp = _t_newr(params,HTTP_RESPONSE); */
    /* _t_new(http_resp,HTTP_RESPONSE_CONTENT_TYPE,"CeptrSymbol/HTTP_REQUEST_PATH_SEGMENT",38); */
    /* T *s = _t_newr(http_resp,SLOT); */
    /* _t_news(s,USAGE,HTTP_REQUEST_PATH_SEGMENT); */
    *paramsP = params;
}
//! [makeTestHTTPResponseProcess]

char *G_stxs = "/ASCII_CHARS/<HTTP_REQUEST:<HTTP_REQUEST_METHOD:ASCII_CHAR!=' '+>,ASCII_CHAR=' ',<HTTP_REQUEST_PATH:<HTTP_REQUEST_PATH_SEGMENTS:(ASCII_CHAR='/',<HTTP_REQUEST_PATH_SEGMENT:ASCII_CHAR!={'/','?',' '}*>)+>>,(ASCII_CHAR='?',<HTTP_REQUEST_PATH_QUERY:<HTTP_REQUEST_PATH_QUERY_PARAMS:(<HTTP_REQUEST_PATH_QUERY_PARAM:<PARAM_KEY:ASCII_CHAR!={'&',' ','='}+>,ASCII_CHAR='=',<PARAM_VALUE:ASCII_CHAR!={'&',' '}*>>,ASCII_CHAR='&'?)+>+>)?,ASCII_CHAR=' ',ASCII_CHAR='H',ASCII_CHAR='T',ASCII_CHAR='T',ASCII_CHAR='P',ASCII_CHAR='/',<HTTP_REQUEST_VERSION:<VERSION_MAJOR:ASCII_CHAR='0'>,ASCII_CHAR='.',<VERSION_MINOR:ASCII_CHAR='9'>>>";

/**
 * generate a semtrex that will match a valid HTTP request in an ASCII_CHARS tree
 *
 */
T *_makeHTTPRequestSemtrex() {
    T *t = parseSemtrex(G_sem,G_stxs);
    return t;

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

T *makeDeltaAdd(T *src,T *t) {
    int *path = _t_get_path(src);
    T *d = makeDelta(TREE_DELTA_ADD,path,t,0);
    free(path);
    return d;
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

    Symbol G_tag_sym[] = {HTML_HTML,HTML_HEAD,HTML_TITLE,HTML_BODY,HTML_DIV,HTML_P,HTML_A,HTML_B,HTML_UL,HTML_OL,HTML_LI,HTML_SPAN,HTML_H1,HTML_H2,HTML_H3,HTML_H4,HTML_FORM};
    char *G_tag_str[] ={"html","head","title","body","div","p","a","b","ul","ol","li","span","h1","h2","h3","h4","form"};

    Symbol G_stag_sym[] = {HTML_IMG,HTML_INPUT,HTML_BUTTON};
    char *G_stag_str[] ={"img","input","button"};

    T *t,*h = makeASCIITree(html);

    /////////////////////////////////////////////////////
    // build the token stream out of an ascii stream
    // PATTERN
    T *s;
    char *stx = "/ASCII_CHARS/<HTML_TOKENS:(ASCII_CHAR='<',<HTML_TOK_TAG_SELFCLOSE:ASCII_CHAR!={'>',' '}+>,<HTML_ATTRIBUTES:(ASCII_CHAR=' ',<HTML_ATTRIBUTE:<PARAM_KEY:ASCII_CHAR!={'>',' ','='}+>,ASCII_CHAR='=',ASCII_CHAR='\"',<PARAM_VALUE:ASCII_CHAR!='\"'+>,ASCII_CHAR='\"'>)*>,ASCII_CHAR='/',ASCII_CHAR='>'|ASCII_CHAR='<',ASCII_CHAR='/',<HTML_TOK_TAG_CLOSE:ASCII_CHAR!='>'+>,ASCII_CHAR='>'|ASCII_CHAR='<',<HTML_TOK_TAG_OPEN:ASCII_CHAR!={'>',' '}+>,<HTML_ATTRIBUTES:(ASCII_CHAR=' ',<HTML_ATTRIBUTE:<PARAM_KEY:ASCII_CHAR!={'>',' ','='}+>,ASCII_CHAR='=',ASCII_CHAR='\"',<PARAM_VALUE:ASCII_CHAR!='\"'+>,ASCII_CHAR='\"'>)*>,ASCII_CHAR='>'|<HTML_TEXT:ASCII_CHAR!='<'+>)+>";

    s = parseSemtrex(G_sem,stx);

    int fnc = 0;
    wjson(G_sem,s,"htmlparse",-1);
    wjson(G_sem,h,"htmlascii",-1);

    T *results,*tokens;
    if (_t_matchr(s,h,&results)) {
    tokens = _t_new_root(HTML_TOKENS);
    int i,m = _t_children(results);
    wjson(G_sem,tokens,"html",fnc++);
    T *delta,*src;
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
        src = _t_child(tokens,_t_children(tokens));
        delta = a;
        _t_add(src,a);
        }
        else {
        src = tokens;
        delta = asciiT_tos(h,c,tokens,ts);
        }
        delta = makeDeltaAdd(src,delta);
        wjson(G_sem,delta,"html",fnc++);
        _t_free(delta);
    }
    _t_free(results);
    _t_free(s);

    s = _t_new_root(SEMTREX_WALK);
    //  T *st = _t_newr(sq,SEMTREX_ZERO_OR_MORE);
    //  _t_newr(st,SEMTREX_SYMBOL_ANY);
    T *g =  _t_news(s,SEMTREX_GROUP,HTML_TAG);
    T *sq = _t_newr(g,SEMTREX_SEQUENCE);
    _sl(sq,HTML_TOK_TAG_OPEN);
    g = _t_news(sq,SEMTREX_GROUP,HTML_CONTENT);
    T* st = _t_newr(g,SEMTREX_ZERO_OR_MORE);
    __sl(st,1,2,HTML_TOK_TAG_OPEN,HTML_TOK_TAG_CLOSE);
    _sl(sq,HTML_TOK_TAG_CLOSE);

    //  stx = "%<HTML_TAG:HTML_TOK_TAG_OPEN,!{HTML_TOK_TAG_OPEN,HTML_TOK_TAG_CLOSE},HTML_TOK_TAG_CLOSE>";
    //s = parseSemtrex(d,stx);
    //  return tokens;
    while (_t_matchr(s,tokens,&results)) {
        T *m = _t_get_match(results,HTML_TAG);
        int *path = _t_surface(_t_child(m,2));
        int count = *(int *)_t_surface(_t_child(results,3));
        T *ot = _t_get(tokens,path);
        path[_t_path_depth(path)-1] += count-1;
        T *ct = _t_get(tokens,path);
        char *otag = _t_surface(ot);
        char *ctag = _t_surface(ct);
        if (strcmp(otag,ctag)) {raise_error("Mismatched tags %s,%s",otag,ctag)};

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
        delta = makeDelta(TREE_DELTA_REPLACE,path,tag,count);
        wjson(G_sem,delta,"html",fnc++);
        _t_free(delta);
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
        delta = makeDelta(TREE_DELTA_REPLACE,path,t,1);
        wjson(G_sem,delta,"html",fnc++);
        _t_free(delta);
        _t_free(results);

    }
    _t_free(s);
    results = _t_detach_by_idx(tokens,1);
    _t_free(tokens);
    _t_free(h);
    return results;
    }
    raise_error("HTML doesn't match");
}

void testHTTPparseHTML() {
    T *t = parseHTML("<html><body><div>Hello <b>world!</b></div></body></html>");

    spec_is_str_equal(t2s(t),"(HTML_HTML (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_BODY (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_DIV (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_TEXT:Hello ) (HTML_B (HTML_ATTRIBUTES) (HTML_CONTENT (HTML_TEXT:world!)))))))))");

    _t_free(t);

}

Receptor *makeHTTP(VMHost *v,Process handler) {
    SemTable *sem = v->r->sem;
    Symbol http = _d_define_receptor(sem,"http server",__r_make_definitions(),DEV_COMPOSITORY_CONTEXT);
    Receptor *r = _r_new(sem,http);
    Xaddr httpx = _v_new_receptor(v,v->r,http,r);

    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,HTTP_SERVER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,HTTP_RESPONSE_HANDLER);
    _t_news(w,ACTUAL_PROCESS,handler);

    _o_express_role(r,HTTP,HTTP_SERVER,HTTP_ASPECT,bindings);
    _t_free(bindings);
    _v_activate(v,httpx);
    return r;
}

void testHTTPprotocol() {
    T *http = _sem_get_def(G_sem,HTTP);

    // check the HTTP protocol definition
    spec_is_str_equal(t2s(http),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL (ENGLISH_LABEL:HTTP)) (PROTOCOL_SEMANTICS (GOAL:HTTP_RESPONSE_HANDLER)) (INCLUSION (PNAME:REQUESTING) (CONNECTION (WHICH_ROLE (ROLE:REQUESTER) (ROLE:HTTP_CLIENT))) (CONNECTION (WHICH_ROLE (ROLE:RESPONDER) (ROLE:HTTP_SERVER))) (RESOLUTION (WHICH_SYMBOL (USAGE:REQUEST_DATA) (ACTUAL_SYMBOL:HTTP_REQUEST))) (RESOLUTION (WHICH_SYMBOL (USAGE:RESPONSE_DATA) (ACTUAL_SYMBOL:HTTP_RESPONSE))) (RESOLUTION (WHICH_SYMBOL (USAGE:CHANNEL) (ACTUAL_SYMBOL:HTTP_ASPECT))) (RESOLUTION (WHICH_PROCESS (GOAL:RESPONSE_HANDLER) (ACTUAL_PROCESS:httpresp)))))");

    T *sem_map = _t_new_root(SEMANTIC_MAP);
    T *t = _o_unwrap(G_sem,http,sem_map);
    // and also check how it gets unwrapped because it's defined in terms of REQUESTING
    // @todo, how come HTTP_SERVER and the two handler goals aren't added to the semantics?
    spec_is_str_equal(t2s(t),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL (ENGLISH_LABEL:HTTP)) (PROTOCOL_SEMANTICS (GOAL:HTTP_RESPONSE_HANDLER) (ROLE:HTTP_CLIENT)) (backnforth (INITIATE (ROLE:HTTP_CLIENT) (DESTINATION (ROLE:HTTP_SERVER)) (ACTION:send_request)) (EXPECT (ROLE:HTTP_SERVER) (SOURCE (ROLE:HTTP_CLIENT)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:HTTP_REQUEST))) (ACTION:send_response))))");

    // the unwrapping should build up a semantic map
    spec_is_str_equal(t2s(sem_map),"(SEMANTIC_MAP (SEMANTIC_LINK (ROLE:REQUESTER) (REPLACEMENT_VALUE (ROLE:HTTP_CLIENT))) (SEMANTIC_LINK (ROLE:RESPONDER) (REPLACEMENT_VALUE (ROLE:HTTP_SERVER))) (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:HTTP_REQUEST))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:HTTP_RESPONSE))) (SEMANTIC_LINK (USAGE:CHANNEL) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:HTTP_ASPECT))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:httpresp))))");
    _t_free(sem_map);

    VMHost *v = G_vm = _v_new();
    SemTable *gsem = G_sem;
    G_sem = v->sem;

    _v_instantiate_builtins(G_vm);

    //debug_enable(D_STREAM+D_SIGNALS+D_TREE+D_PROTOCOL);
    Receptor *r = makeHTTP(v,fill_i_am);

    FILE *rs,*ws;
    char buffer[] = "GET /path/to/file.ext?name=joe&age=30 HTTP/0.9\n";

    rs = fmemopen(buffer, strlen (buffer), "r");
    Stream *reader_stream = _st_new_unix_stream(rs,1);

    char *output_data;
    size_t size;
    ws = open_memstream(&output_data,&size);
    Stream *writer_stream = _st_new_unix_stream(ws,0);

    Receptor *er = _r_makeStreamEdgeReceptor(v->sem);
    Xaddr edge = _v_new_receptor(v,v->r,STREAM_EDGE,er);
    _r_addWriter(er,TEST_STREAM_SYMBOL,writer_stream,DEFAULT_ASPECT);
    _r_addReader(er,TEST_STREAM_SYMBOL,reader_stream,er->addr,HTTP_ASPECT,parse_line,ASCII_CHARS);
    _v_activate(v,edge);

    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,HTTP_REQUEST_PARSER);
    __r_make_addr(w,ACTUAL_RECEPTOR,er->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,LINE_SENDER);
    __r_make_addr(w,ACTUAL_RECEPTOR,er->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,HTTP_SERVER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    _o_express_role(er,PARSE_HTTP_REQUEST_FROM_LINE,HTTP_REQUEST_PARSER,HTTP_ASPECT,bindings);
    _t_free(bindings);

    _v_start_vmhost(G_vm);
    sleep(1);
    debug_disable(D_STREAM+D_SIGNALS+D_TREE+D_PROTOCOL);

    spec_is_true(output_data != 0); // protect against seg-faults when nothing was written to the stream...
    if (output_data != 0) {
        spec_is_str_equal(output_data,"HTTP/1.1 200 OK\nContent-Type: text/ceptr\n\nbody\n");
    }
    __r_kill(G_vm->r);

    _v_join_thread(&G_vm->clock_thread);
    _v_join_thread(&G_vm->vm_thread);

    _st_free(reader_stream);
    _st_free(writer_stream);
    free(output_data);
    _v_free(v);
    G_vm = NULL;
    G_sem = gsem;

}

void testHTTP() {
    testHTTPparseHTML();
    testHTTPprotocol();
}

#endif
//
