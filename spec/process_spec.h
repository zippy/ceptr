/**
 * @file process_spec.h
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#include "../src/ceptr.h"
#include "../src/process.h"

void testRunTree() {
    T *code;

    // a process that would look something like this in lisp:
    // (defun my_if (true_branch false_branch condition) (if (condition) (true_branch) (false_branch)))
    code = _t_parse(G_sem,0,"(IF (PARAM_REF:/2/3) (PARAM_REF:/2/1) (PARAM_REF:/2/2))");

    T *signature = __p_make_signature("result",SIGNATURE_PASSTHRU,NULL_STRUCTURE,
                                   "condition",SIGNATURE_PROCESS,BOOLEAN,
                                   "true_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                   "false_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                   NULL);

    // test that __p_make_signature does what we think it should
    spec_is_str_equal(t2s(signature),"(PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:result)) (SIGNATURE_PASSTHRU)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:condition)) (SIGNATURE_PROCESS:BOOLEAN)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:true_branch)) (SIGNATURE_ANY)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:false_branch)) (SIGNATURE_ANY)))");

    Process p = _d_define_process(G_sem,code,"myif","a duplicate of the sys if process with params in different order",signature,NULL,TEST_CONTEXT);

    T *params = _t_parse(G_sem,0,"(PARAMS (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:321) (BOOLEAN:1))");

    T *r = _p_make_run_tree(G_sem,p,params,NULL);
    _t_free(params);

    spec_is_str_equal(t2s(r),"(RUN_TREE (process:IF (PARAM_REF:/2/3) (PARAM_REF:/2/1) (PARAM_REF:/2/2)) (PARAMS (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:321) (BOOLEAN:1)))");

    spec_is_equal(_p_reduce(G_sem,r),noReductionErr);

    spec_is_str_equal(t2s(_t_child(r,1)),"(TEST_INT_SYMBOL:123)");

    _t_free(r);

    params = _t_parse(G_sem,0,"(PARAMS (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:321))");
    // you can also create a run tree with a system process

    r = _p_make_run_tree(G_sem,ADD_INT,params,NULL);
    _t_free(params);
    spec_is_str_equal(t2s(r),"(RUN_TREE (process:ADD_INT (TEST_INT_SYMBOL:123) (TEST_INT_SYMBOL:321)) (PARAMS))");

    _t_free(r);
}

//-----------------------------------------------------------------------------------------
// tests of system processes

void testProcessGet() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    T *t = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr x = _r_new_instance(r,t);

    T *n = _t_newr(0,GET);
    _t_new(n,WHICH_XADDR,&x,sizeof(Xaddr));
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_INT_SYMBOL:314) (PARAMS))");
    t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&r->instances,TEST_INT_SYMBOL,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA (TEST_INT_SYMBOL:314))");
    _t_free(t);

    _r_free(r);
}

void testProcessDel() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    T *t = _t_newi(0,TEST_INT_SYMBOL,314);
    Xaddr x = _r_new_instance(r,t);

    T *n = _t_newr(0,DEL);
    _t_new(n,WHICH_XADDR,&x,sizeof(Xaddr));
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_INT_SYMBOL:314) (PARAMS))");
    t = _t_new_root(ITERATION_DATA);
    _a_get_instances(&r->instances,TEST_INT_SYMBOL,t);
    spec_is_str_equal(t2s(t),"(ITERATION_DATA)");
    _t_free(t);

    _r_free(r);
}

void testProcessNew() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    T *n = _t_newr(0,NEW);
    _t_news(n,NEW_TYPE,TEST_INT_SYMBOL);
    _t_new_str(n,TEST_STR_SYMBOL,"fish");
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    _p_reduceq(q);

    spec_is_equal(r->q->completed->context->err,structureMismatchReductionErr);

    n = _t_newr(0,NEW);
    _t_news(n,NEW_TYPE,TEST_INT_SYMBOL);
    _t_newi(n,TEST_INT_SYMBOL2,314);
    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (WHICH_XADDR:TEST_INT_SYMBOL.1) (PARAMS))");
    T *i = _r_get_instance(r,*(Xaddr *)_t_surface(_t_child(run_tree,1)));
    spec_is_str_equal(t2s(i),"(TEST_INT_SYMBOL:314)");
    //    _t_free(i);
    _r_free(r);
}

void testProcessDefine() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *p = _t_parse(r->sem,0,"(DEF_SYMBOL (SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:\"age\")) (SYMBOL_STRUCTURE:INTEGER)))");
    spec_is_equal(__p_reduce_sys_proc(0,DEF_SYMBOL,p,r->q),noReductionErr);
    spec_is_str_equal(t2s(p),"(RESULT_SYMBOL:age)");
    T *d = _sem_get_def(r->sem,*(Symbol *)_t_surface(p));
    spec_is_str_equal(t2s(d),"(SYMBOL_DEFINITION (SYMBOL_LABEL (ENGLISH_LABEL:age)) (SYMBOL_STRUCTURE:INTEGER))");
    _t_free(p);

    p = _t_parse(r->sem,0,"(DEF_STRUCTURE (STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:\"age_pair\")) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:age) (STRUCTURE_SYMBOL:age)))");
    spec_is_equal(__p_reduce_sys_proc(0,DEF_STRUCTURE,p,r->q),noReductionErr);
    spec_is_str_equal(t2s(p),"(RESULT_STRUCTURE:age_pair)");
    d = _sem_get_def(r->sem,*(Symbol *)_t_surface(p));
    spec_is_str_equal(t2s(d),"(STRUCTURE_DEFINITION (STRUCTURE_LABEL (ENGLISH_LABEL:age_pair)) (STRUCTURE_SEQUENCE (STRUCTURE_SYMBOL:age) (STRUCTURE_SYMBOL:age)))");
    _t_free(p);

    p = _t_parse(r->sem,0,"(DEF_PROCESS (PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:\"power\")) (PROCESS_INTENTION:\"takes the mathematical power of the two params\") (NOOP) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:\"result\")) (SIGNATURE_SYMBOL:NULL_SYMBOL)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:\"val\")) (SIGNATURE_STRUCTURE:INTEGER)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:\"exponent\")) (SIGNATURE_STRUCTURE:INTEGER))) (PROCESS_LINK (PROCESS_OF_STRUCTURE:INTEGER) (PROCESS_TYPE (OPERATOR))))");
    spec_is_equal(__p_reduce_sys_proc(0,DEF_PROCESS,p,r->q),noReductionErr);
    spec_is_str_equal(t2s(p),"(RESULT_PROCESS:power)");
    d = _sem_get_def(r->sem,*(Symbol *)_t_surface(p));
    spec_is_str_equal(t2s(d),"(PROCESS_DEFINITION (PROCESS_NAME (ENGLISH_LABEL:power)) (PROCESS_INTENTION:takes the mathematical power of the two params) (process:NOOP) (PROCESS_SIGNATURE (OUTPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:result)) (SIGNATURE_SYMBOL:NULL_SYMBOL)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:val)) (SIGNATURE_STRUCTURE:INTEGER)) (INPUT_SIGNATURE (SIGNATURE_LABEL (ENGLISH_LABEL:exponent)) (SIGNATURE_STRUCTURE:INTEGER))) (PROCESS_LINK (PROCESS_OF_STRUCTURE:INTEGER) (PROCESS_TYPE (OPERATOR))))");
    _t_free(p);

    p = _t_parse(r->sem,0,"(DEF_RECEPTOR (RECEPTOR_DEFINITION (RECEPTOR_LABEL (ENGLISH_LABEL:\"streamscapes\")) (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (RECEPTORS) (PROTOCOLS) (SCAPES)))");
    spec_is_equal(__p_reduce_sys_proc(0,DEF_RECEPTOR,p,r->q),noReductionErr);
    spec_is_str_equal(t2s(p),"(RESULT_RECEPTOR:streamscapes)");
    d = _sem_get_def(r->sem,*(Symbol *)_t_surface(p));
    spec_is_str_equal(t2s(d),"(RECEPTOR_DEFINITION (RECEPTOR_LABEL (ENGLISH_LABEL:streamscapes)) (DEFINITIONS (STRUCTURES) (SYMBOLS) (PROCESSES) (RECEPTORS) (PROTOCOLS) (SCAPES)))");
    _t_free(p);

    _d_define_symbol(G_sem,RECEPTOR_ADDRESS,"agent",r->context);
    _d_define_symbol(G_sem,PROCESS,"process",r->context);
    _d_define_symbol(G_sem,SYMBOL,"data",r->context);
    _d_define_symbol(G_sem,INTERACTION,"act",r->context);

    p = _t_parse(r->sem,0,"(DEF_PROTOCOL (PROTOCOL_DEFINITION (PROTOCOL_LABEL (ENGLISH_LABEL:\"do\")) (PROTOCOL_SEMANTICS (ROLE:agent) (GOAL:process) (USAGE:data)) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:data) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (SLOT (GOAL:process) (SLOT_IS_VALUE_OF:ACTION))))))");
    spec_is_equal(__p_reduce_sys_proc(0,DEF_PROTOCOL,p,r->q),noReductionErr);
    spec_is_str_equal(t2s(p),"(RESULT_PROTOCOL:do)");
    d = _sem_get_def(r->sem,*(Symbol *)_t_surface(p));
    spec_is_str_equal(t2s(d),"(PROTOCOL_DEFINITION (PROTOCOL_LABEL (ENGLISH_LABEL:do)) (PROTOCOL_SEMANTICS (ROLE:agent) (GOAL:process) (USAGE:data)) (act (EXPECT (ROLE:agent) (SOURCE (ROLE:agent)) (PATTERN (SEMTREX_SYMBOL_LITERAL (SLOT (USAGE:data) (SLOT_IS_VALUE_OF:SEMTREX_SYMBOL)))) (SLOT (GOAL:process) (SLOT_IS_VALUE_OF:ACTION)))))");
    _t_free(p);

    _r_free(r);
}

void testProcessDo() {
    T *code = _t_parse(G_sem,0,"(DO (SCOPE (TEST_INT_SYMBOL:1) (TEST_INT_SYMBOL:2)))");
    spec_is_equal(__p_reduce_sys_proc(0,DO,code,0),noReductionErr);
    spec_is_str_equal(t2s(code),"(TEST_INT_SYMBOL:2)");
    _t_free(code);
}

void testProcessTranscode() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // transcode of same structure should just change the symbol type
    T *n;
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:TEST_INT_SYMBOL)) (TRANSCODE_ITEMS (TEST_INT_SYMBOL2:314)))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:314)");
    _t_free(n);

    // transcode of INTEGER to CSTRING
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:TEST_STR_SYMBOL)) (TRANSCODE_ITEMS (TEST_INT_SYMBOL2:314)))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_STR_SYMBOL:314)");
    _t_free(n);

    // transcode of FLOAT to CSTRING
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:TEST_STR_SYMBOL)) (TRANSCODE_ITEMS (TEST_FLOAT_SYMBOL:3.14159)))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_STR_SYMBOL:3.141590)");
    _t_free(n);

    // transcode of CHAR to CSTRING
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:TEST_STR_SYMBOL)) (TRANSCODE_ITEMS (ASCII_CHAR:'x')))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_STR_SYMBOL:x)");
    _t_free(n);

    // transcode of CSTRING to INTEGER
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:TEST_INT_SYMBOL)) (TRANSCODE_ITEMS (TEST_STR_SYMBOL:\"314\")))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:314)");
    _t_free(n);

    //debug_enable(D_TRANSCODE);
    // transcode of matching optionality structures
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:LINES)) (TRANSCODE_ITEMS (TEST_INTEGERS (TEST_INT_SYMBOL:1) (TEST_INT_SYMBOL:2) (TEST_INT_SYMBOL:314))))");
    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),redoReduction);
    spec_is_str_equal(t2s(n),"(LINES (LINE:1) (LINE:2) (LINE:314))");
    _t_free(n);
    debug_disable(D_TRANSCODE);

    //transcode of constructed symbol to CSTRING
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:LINE)) (TRANSCODE_ITEMS (CONTENT_TYPE (MEDIA_TYPE_IDENT:TEXT_MEDIA_TYPE) (MEDIA_SUBTYPE_IDENT:CEPTR_TEXT_MEDIA_SUBTYPE))))");
    Q *q = r->q;
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(_t_child(run_tree,1)),"(LINE:Content-Type: text/ceptr)");

    //        debug_enable(D_REDUCE+D_REDUCEV);
    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:LINE)) (TRANSCODE_ITEMS (TODAY (YEAR:2015) (MONTH:1) (DAY:30))))");

    //    spec_is_equal(__p_reduce_sys_proc(0,TRANSCODE,n,r->q),noReductionErr);
    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    debug_disable(D_REDUCE+D_REDUCEV);
    spec_is_str_equal(t2s(_t_child(run_tree,1)),"(US_SHORT_DATE:1/30/2015)");

    n = _t_parse(G_sem,0,"(TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:LINES)) (TRANSCODE_ITEMS (HTTP_HEADERS (CONTENT_TYPE (MEDIA_TYPE_IDENT:TEXT_MEDIA_TYPE) (MEDIA_SUBTYPE_IDENT:CEPTR_TEXT_MEDIA_SUBTYPE)) (CONTENT_TYPE (MEDIA_TYPE_IDENT:TEXT_MEDIA_TYPE) (MEDIA_SUBTYPE_IDENT:HTML_TEXT_MEDIA_SUBTYPE)))))");
    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (LINES (LINE:Content-Type: text/ceptr) (LINE:Content-Type: text/html)) (PARAMS))");

    //debug_enable(D_TRANSCODE+D_STEP);
    //    debug_enable(D_REDUCE+D_REDUCEV);
    n = _t_parse(G_sem,0,"(ascii_chars_2_http_req (TRANSCODE (TRANSCODE_PARAMS (TRANSCODE_TO:ASCII_CHARS)) (TRANSCODE_ITEMS (TEST_STR_SYMBOL:\"GET /path/to/file.ext?name=joe&age=30 HTTP/0.9\n\"))))");

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (HTTP_REQUEST (HTTP_REQUEST_METHOD:GET) (HTTP_REQUEST_PATH (HTTP_REQUEST_PATH_SEGMENTS (HTTP_REQUEST_PATH_SEGMENT:path) (HTTP_REQUEST_PATH_SEGMENT:to) (HTTP_REQUEST_PATH_SEGMENT:file.ext))) (HTTP_REQUEST_PATH_QUERY (HTTP_REQUEST_PATH_QUERY_PARAMS (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:name) (PARAM_VALUE:joe)) (HTTP_REQUEST_PATH_QUERY_PARAM (PARAM_KEY:age) (PARAM_VALUE:30)))) (HTTP_REQUEST_VERSION (VERSION_MAJOR:0) (VERSION_MINOR:9))) (PARAMS))");

    debug_disable(D_TRANSCODE);
    debug_disable(D_REDUCE+D_REDUCEV);
    debug_disable(D_STEP);

    _r_free(r);
}

void testProcessDissolve() {
    T *n = _t_new_root(DISSOLVE);
    spec_is_equal(__p_reduce_sys_proc(0,DISSOLVE,n,0),structureMismatchReductionErr);
    _t_free(n);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    n = _t_parse(G_sem,0,"(LINES (LINE:\"fish\") (DISSOLVE (LINES (LINE:\"cat\") (LINE:\"dog\"))) (CONCAT_STR (RESULT_SYMBOL:LINE) (TEST_STR_SYMBOL:\"sh\") (TEST_STR_SYMBOL:\"oe\")))");

    Q *q = r->q;
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(_t_child(run_tree,1)),"(LINES (LINE:fish) (LINE:cat) (LINE:dog) (LINE:shoe))");

    // @todo when we have var arg signatures then this spec should be made to work
    /* n = _t_build(G_sem,0,LINES,LINE,"fish",DISSOLVE,LINE,"cat",LINE,"dog",NULL_SYMBOL,NULL_SYMBOL); */
    /* run_tree = __p_build_run_tree(n,0); */
    /* _t_free(n); */
    /* _p_addrt2q(q,run_tree); */
    /* spec_is_equal(_p_reduceq(q),noReductionErr); */
    /* spec_is_str_equal(t2s(_t_child(run_tree,1)),"(LINES (LINE:fish) (LINE:cat) (LINE:dog))"); */

    _r_free(r);
}

void testProcessSemtrex() {

    // test string concatenation
    T *n = _t_new_root(MATCH);
    T *pattern = _sl(n,TICK);
    T *t = __r_make_tick();
    _t_add(n,t);
    __p_reduce_sys_proc(0,MATCH,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    n = _t_new_root(MATCH);
    pattern = _sl(n,TICK);
    _t_newi(n,TEST_INT_SYMBOL,1);
    __p_reduce_sys_proc(0,MATCH,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(MATCH);
    pattern = _t_newr(n,SEMTREX_WALK);
    T *g = _t_news(pattern,SEMTREX_GROUP,TEST_CHAR_SYMBOL);
    __stxcv(g,'s');
    t = makeASCIITree("fish");
    _t_add(n,t);
    _t_newi(n,BOOLEAN,1);
    __p_reduce_sys_proc(0,MATCH,n,0);
    spec_is_str_equal(t2s(n),"(SEMTREX_MATCH:1 (SEMTREX_MATCH_SYMBOL:TEST_CHAR_SYMBOL) (SEMTREX_MATCH_PATH:/3) (SEMTREX_MATCH_SIBLINGS_COUNT:1))");

    _t_free(n);
}

void testProcessFill() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    T *n = _t_parse(G_sem,0,"(FILL (SLOT (GOAL:REQUEST_HANDLER) (SLOT_CHILDREN (TEST_INT_SYMBOL:1) (SLOT (USAGE:REQUEST_DATA)))) (SEMANTIC_MAP (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (TEST_INT_SYMBOL:32))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:ADD_INT)))))");
    spec_is_equal(__p_reduce_sys_proc(0,FILL,n,r->q),noReductionErr);

    //@todo, should this have been reduced too?  Or should there be an explicit eval kind of thing?
    spec_is_str_equal(t2s(n),"(process:ADD_INT (TEST_INT_SYMBOL:1) (TEST_INT_SYMBOL:32))");
    _t_free(n);
    _r_free(r);
}

void testProcessFillMatch() {
    T *t = _t_new_root(RUN_TREE);
    // test FILL_FROM_MATCH which takes three params, the template tree, the stx-match and the tree it matched on
    T *n = _t_new_root(FILL_FROM_MATCH);
    T *p1 = _t_newr(n,TEST_ANYTHING_SYMBOL);
    T *s = _t_newr(p1,SLOT);
    _t_news(s,USAGE,TEST_INT_SYMBOL2);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,TEST_INT_SYMBOL2);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCH_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL2,314);

    // @todo N.B. SEMTREX_MATCH is not used correctly, the structure is interpreted two different
    // ways, as both and INT and SYMBOL, so we can't use parse because it will fail on the incorrect type
    //n = _t_parse(G_sem,0,"(FILL_FROM_MATCH (TEST_ANYTHING_SYMBOL (SLOT (USAGE:TEST_INT_SYMBOL2))) (SEMTREX_MATCH:1 (SEMTREX_MATCH:TEST_INT_SYMBOL2) (SEMTREX_MATCH_PATH:/) (SEMTREX_MATCH_SIBLINGS_COUNT:1)) (TEST_INT_SYMBOL2:314))");

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL2:314))");
    _t_free(t);
    _t_free(n);
}

void testProcessFillMatchFull() {
    T *t = _t_new_root(RUN_TREE);
    // test FILL_FROM_MATCH which takes three params, the template tree, the stx-match and the tree it matched on
    T *n = _t_new_root(FILL_FROM_MATCH);
    T *p1 = _t_newr(n,TEST_ANYTHING_SYMBOL);
    T *s = _t_newr(p1,SLOT);
    _t_news(s,USAGE,NULL_SYMBOL);
    T *p2 = _t_newi(n,SEMTREX_MATCH,1);
    _t_news(p2,SEMTREX_MATCH,NULL_SYMBOL);
    int path[] = {TREE_PATH_TERMINATOR};
    _t_new(p2,SEMTREX_MATCH_PATH,path,2*sizeof(int));
    _t_newi(p2,SEMTREX_MATCH_SIBLINGS_COUNT,1);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,314);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL:314))");
    _t_free(t);
    _t_free(n);
}

/// @todo when interpolating from a match, how do we handle non-leaf interpollations, i.e. where do you hook children onto?

void testProcessIf() {
    // test IF which takes three parameters, the condition, the true code tree and the false code tree
    T *n = _t_new_root(IF);
    T *p1 = _t_newi(n,BOOLEAN,1);
    T *p2 = _t_newi(n,TEST_INT_SYMBOL,99);
    T *p3 = _t_newi(n,TEST_INT_SYMBOL,100);

    __p_reduce_sys_proc(0,IF,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
}

void testProcessCond(){
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_parse(G_sem,0,"(COND (CONDITIONS (COND_PAIR (BOOLEAN:0) (ASCII_CHAR:'x')) (COND_ELSE (ASCII_CHAR:'y'))))");
    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(ASCII_CHAR:'y')");
    _t_free(t);
    _t_free(n);

    t = _t_new_root(RUN_TREE);
    n = _t_parse(G_sem,0,"(COND (CONDITIONS (COND_PAIR (BOOLEAN:0) (ASCII_CHAR:'x')) (COND_PAIR (BOOLEAN:1) (ASCII_CHAR:'y')) (COND_ELSE (ASCII_CHAR:'z'))))");
    c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(ASCII_CHAR:'y')");
    _t_free(t);
    _t_free(n);
}

void testProcessSym() {
    T *n = _t_new_root(EQ_SYM);
    _t_news(n,EQUALITY_TEST_SYMBOL,TEST_INT_SYMBOL);
    _t_news(n,TEST_SYMBOL_SYMBOL,TEST_INT_SYMBOL);
    __p_reduce_sys_proc(0,EQ_SYM,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);
    n = _t_new_root(EQ_SYM);
    _t_news(n,EQUALITY_TEST_SYMBOL,TEST_INT_SYMBOL);
    _t_news(n,TEST_SYMBOL_SYMBOL,TEST_STR_SYMBOL);
    __p_reduce_sys_proc(0,EQ_SYM,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);
}

void testProcessIntMath() {
    T *t;

    // test addition
    T *n = _t_new_root(ADD_INT);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,ADD_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)");
    _t_free(n);

    /// @todo structure type checking for integer math?  Too expensive?
    // where do we get the defs.  Right now they are referred to in the process q
    // which the contexts don't have direct access to.
    /* n = _t_new_root(ADD_INT); */
    /* spec_is_sem_equal(_sem_get_symbol_structure(G_sem,s),INTEGER); */
    /* _t_newi(n,TEST_INT_SYMBOL,99); */
    /* T *s = _t_newr(n,SLOT); */
    /* _t_news(s,USAGE,TEST_INT_SYMBOL); */
    /* spec_is_equal(__p_reduce_sys_proc(0,ADD_INT,n,0),incompatibleTypeReductionErr); */
    /* spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:199)"); */
    /* _t_free(n); */

    // test subtraction
    n = _t_new_root(SUB_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,SUB_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test multiplication
    n = _t_new_root(MULT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,98);
    __p_reduce_sys_proc(0,MULT_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:9800)");
    _t_free(n);

    // test division
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,48);
    __p_reduce_sys_proc(0,DIV_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:2)");
    _t_free(n);

    // test division with divide by zero
    n = _t_new_root(DIV_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,DIV_INT,n,0),divideByZeroReductionErr);
    _t_free(n);

    // test modulo
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,MOD_INT,n,0);
    spec_is_str_equal(t2s(n),"(TEST_INT_SYMBOL:0)");
    _t_free(n);

    // test modulo with divide by zero
    n = _t_new_root(MOD_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,0);
    spec_is_equal(__p_reduce_sys_proc(0,MOD_INT,n,0),divideByZeroReductionErr);
    _t_free(n);

    // test equals
    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,2);
    __p_reduce_sys_proc(0,EQ_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(EQ_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,EQ_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test <
    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    n = _t_new_root(LT_INT);
    _t_newi(n,TEST_INT_SYMBOL,100);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,LT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
   _t_free(n);

    // test >
    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,2);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    n = _t_new_root(GT_INT);
    _t_newi(n,TEST_INT_SYMBOL,101);
    _t_newi(n,TEST_INT_SYMBOL,100);
    __p_reduce_sys_proc(0,GT_INT,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

}

void testProcessPath() {
    T *p = _t_parse(G_sem,0,"(POP_PATH (RECEPTOR_PATH:/4/1/1) (RESULT_SYMBOL:CONTINUE_LOCATION))");
    __p_reduce_sys_proc(0,POP_PATH,p,0);
    spec_is_str_equal(t2s(p),"(CONTINUE_LOCATION:/4/1)");
    _t_free(p);
    p = _t_parse(G_sem,0,"(POP_PATH (RECEPTOR_PATH:/4/1/1) (RESULT_SYMBOL:CONTINUE_LOCATION) (POP_COUNT:2))");
    __p_reduce_sys_proc(0,POP_PATH,p,0);
    spec_is_str_equal(t2s(p),"(CONTINUE_LOCATION:/4)");
    _t_free(p);
    p = _t_parse(G_sem,0,"(POP_PATH (RECEPTOR_PATH:/4/1/1) (RESULT_SYMBOL:CONTINUE_LOCATION) (POP_COUNT:3))");
    __p_reduce_sys_proc(0,POP_PATH,p,0);
    spec_is_str_equal(t2s(p),"(CONTINUE_LOCATION:/)");
    _t_free(p);
    p = _t_parse(G_sem,0,"(POP_PATH (RECEPTOR_PATH:/4/1/1) (RESULT_SYMBOL:CONTINUE_LOCATION) (POP_COUNT:10))");
    __p_reduce_sys_proc(0,POP_PATH,p,0);
    spec_is_str_equal(t2s(p),"(CONTINUE_LOCATION:/)");
    _t_free(p);}

void testProcessString() {

    T *run_tree = _t_new_root(RUN_TREE);
    R *c = __p_make_context(run_tree,0,0,NULL);
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // test string concatenation
    T *n = _t_new_root(CONCAT_STR);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_new_str(n,TEST_STR_SYMBOL,"Fred");
    _t_new_str(n,TEST_STR_SYMBOL," ");
    _t_new_str(n,TEST_STR_SYMBOL,"Smith");
    spec_is_equal(__p_reduce_sys_proc(c,CONCAT_STR,n,r->q),noReductionErr);

    spec_is_str_equal(t2s(n),"(TEST_NAME_SYMBOL:Fred Smith)");
    _t_free(n);

    // test string to char tree conversion
    n = _t_new_root(EXPAND_STR);
    _t_new_str(n,TEST_STR_SYMBOL,"fish");
    spec_is_equal(__p_reduce_sys_proc(0,EXPAND_STR,n,0),noReductionErr);
    spec_is_str_equal(t2s(n),"(ASCII_CHARS (ASCII_CHAR:'f') (ASCII_CHAR:'i') (ASCII_CHAR:'s') (ASCII_CHAR:'h'))");
    _t_free(n);

    // test string contraction
    n = _t_new_root(CONTRACT_STR);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    char xx = 'F';
    _t_new(n,ASCII_CHAR,&xx,sizeof(char));
    _t_new_str(n,TEST_STR_SYMBOL,"red");
    xx = ' ';
    _t_new(n,ASCII_CHAR,&xx,sizeof(char));
    _t_new_str(n,TEST_STR_SYMBOL,"Smith");
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TEST_NAME_SYMBOL:Fred Smith)");
    _t_free(n);

    n = _t_new_root(CONTRACT_STR);
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),tooFewParamsReductionErr);
    _t_newi(n,TEST_INT_SYMBOL,314);
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),signatureMismatchReductionErr);
    _t_news(n,RESULT_SYMBOL,TEST_INT_SYMBOL);
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),signatureMismatchReductionErr);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),tooFewParamsReductionErr);
    _t_news(n,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_newi(n,TEST_INT_SYMBOL,314);
    spec_is_equal(__p_reduce_sys_proc(c,CONTRACT_STR,n,r->q),incompatibleTypeReductionErr);
    _t_free(n);

    _p_free_context(c);
    _r_free(r);

}

void testProcessRespond() {
    // testing responding to a signal requires setting up a sending signal context

    T *signal_contents = _t_newi(0,TEST_INT_SYMBOL,314);
    ReceptorAddress f = {3}; // DUMMY ADDR
    ReceptorAddress t = {4}; // DUMMY ADDR

    T *s = __r_make_signal(f,t,DEFAULT_ASPECT,TESTING,signal_contents,0,defaultRequestUntil(),0);

    T *run_tree = _t_new_root(RUN_TREE);
    T *n = _t_newr(run_tree,RESPOND);
    _t_news(n,CARRIER,TESTING);

    T *response_contents = _t_newi(n,TEST_INT_SYMBOL,271);

    R *c = __p_make_context(run_tree,0,0,NULL);

    // if this is a run-tree that's not a child of a signal we can't respond!
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n,0),notInSignalContextReductionError);

    // now add it to the signal and try again
    _t_add(s,run_tree);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // it should create a response signal with the source UUID as the responding to UUID
    spec_is_equal(__p_reduce_sys_proc(c,RESPOND,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(SIGNAL_UUID)");
    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:4)) (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (IN_RESPONSE_TO_UUID)) (BODY:{(TEST_INT_SYMBOL:271)}))))");
    T *u1 = _t_child(_t_child(s,SignalEnvelopeIdx),EnvelopeSignalUUIDIdx);
    int p[] = {1,SignalMessageIdx,MessageHeadIdx,HeadOptionalsIdx,TREE_PATH_TERMINATOR};
    T *u2 = _t_get(r->pending_signals,p);
    spec_is_true(__uuid_equal(_t_surface(u1),_t_surface(u2)));

    _p_free_context(c);
    _t_free(s);

    _r_free(r);
}
extern int G_next_process_id;

void testProcessSay() {
    T *p = _t_newr(0,SAY);
    ReceptorAddress to = {99}; // DUMMY ADDR

    __r_make_addr(p,TO_ADDRESS,to);
    _t_news(p,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(p,CARRIER,TESTING);
    _t_newi(p,TEST_INT_SYMBOL,314);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *run_tree = __p_build_run_tree(p,0);
    _t_free(p);

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state
    // and the signal should be on the pending signals list
    // the UUID should be in pending responses list
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);

    // say reduces to the UUID generated for the sent signal
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING)) (BODY:{(TEST_INT_SYMBOL:314)}))))");
    _r_free(r);
}

void testProcessRequest() {
    T *p = _t_newr(0,REQUEST);
    ReceptorAddress to = {99}; // DUMMY ADDR

    __r_make_addr(p,TO_ADDRESS,to);
    _t_news(p,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(p,CARRIER,TESTING);
    _t_newi(p,TEST_INT_SYMBOL,314);
    _t_news(p,RESPONSE_CARRIER,TESTING);
    T *ec = _t_newr(p,END_CONDITIONS);
    _t_newi(ec,COUNT,1);

    T *code =_t_rclone(p);
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *run_tree = __p_build_run_tree(code,0);

    spec_is_str_equal(_td(r,run_tree),"(RUN_TREE (process:REQUEST (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (TEST_INT_SYMBOL:314) (RESPONSE_CARRIER:TESTING) (END_CONDITIONS (COUNT:1))) (PARAMS))");

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state
    // and the signal should be on the pending signals list
    // the UUID should be in pending responses list
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_equal(c->state,Block);
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES (PENDING_RESPONSE (SIGNAL_UUID) (CARRIER:TESTING) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1)) (END_CONDITIONS (COUNT:1))))");

    // request reduces to the UUID generated for the sent signal
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))");
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (END_CONDITIONS (COUNT:1))) (BODY:{(TEST_INT_SYMBOL:314)}))))");

    // debug_enable(D_SIGNALS);
    // generate a response signal

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"one fish"),_t_surface(_t_child(run_tree,1)),0,0);
    _r_deliver(r,s);
    spec_is_str_equal(_td(r,r->pending_responses),"(PENDING_RESPONSES)");

    //   _p_freeq(q);
    // clear off the signal in the list
    _t_free(_t_detach_by_idx(r->pending_signals,1));

    /* // now test the callback request case */
    /* _t_newi(p,BOOLEAN,1); */

    /* run_tree = __p_build_run_tree(p,0); */

    /* r->q = q = _p_newq(r); */
    /* ps = r->pending_signals; */
    /* e = _p_addrt2q(q,run_tree); */
    /* c = e->context; */

    /* // after reduction the context should have been moved to the completed list */
    /* // and the signal should be on the pending signals list */
    /* spec_is_equal(_p_reduceq(q),noReductionErr); */

    /* spec_is_equal(q->contexts_count,0); */
    /* spec_is_ptr_equal(q->completed,e); */

    /* spec_is_str_equal(t2s(run_tree),"(RUN_TREE (SIGNAL_UUID) (PARAMS))"); */
    /* spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (FROM_ADDRESS (CONTEXT_NUM:0)) (TO_ADDRESS (CONTEXT_NUM:3)) (DEFAULT_ASPECT) (CARRIER:TEST_INT_SYMBOL) (SIGNAL_UUID)) (BODY:{(TEST_INT_SYMBOL:314)})))"); */

    debug_disable(D_SIGNALS);

    _r_free(r);
    _t_free(p);
    _t_free(code);
}


T *_testProcessAddSay(T *parent,int id,T *message) {
    T *say =  _t_newr(parent,SAY);
    ReceptorAddress to = {id}; // DUMMY ADDR
    __r_make_addr(say,TO_ADDRESS,to);
    _t_news(say,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(say,CARRIER,TESTING);
    _t_add(say,message);
    return say;
}

/*
  (DO
     (SCOPE
       (SAY <to> <aspect> <carrier> <message> )
       (CONVERSE
          (SCOPE
            (LISTEN <aspect> <carrier> <action>)
            (SAY <to> <aspect> <carrier> <message> )
          )
        (END_CONDITIONS (UNLIMITED))
      )
    )
  )
*/

void testProcessConverse() {
    T *p = _t_newr(0,CONVERSE);
    T *scope = _t_newr(p,SCOPE);
    _t_newi(p,BOOLEAN,1);
    _testProcessAddSay(scope,100,_t_newi(0,TEST_INT_SYMBOL,31415));
    T *code = _testProcessAddSay(0,99,p);

    T *run_tree = __p_build_run_tree(code,0);
    _t_free(code);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:SAY (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (process:CONVERSE (SCOPE (process:SAY (TO_ADDRESS (RECEPTOR_ADDR:100)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (TEST_INT_SYMBOL:31415))) (BOOLEAN:1))) (PARAMS))");

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *cons = r->conversations;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context be paused because we set wait to true
    // and there should also be a conversation recorded
    //debug_enable(D_STEP);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    debug_disable(D_STEP);
    spec_is_str_equal(t2s(cons),"(CONVERSATIONS (CONVERSATION (CONVERSATION_UUID) (END_CONDITIONS (UNLIMITED)) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1/4))))");

    //CONVERSE should be reduced to the signal UUID from the containing scope
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:SAY (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (SIGNAL_UUID)) (PARAMS))");
    spec_is_ptr_equal(q->blocked,e);

    // The signal to 100 should have the conversation id in it
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:100)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (CONVERSATION_UUID)) (BODY:{(TEST_INT_SYMBOL:31415)}))))");

    // now use the COMPLETE instruction to clean-up
    code = _t_newr(0,COMPLETE);
    _t_newi(code,TEST_INT_SYMBOL,123);
    _t_add(code,_t_clone(_t_getv(cons,1,ConversationIdentIdx,TREE_PATH_TERMINATOR)));

    T *run_tree2 = __p_build_run_tree(code,0);
    _t_free(code);
    Qe *e2 =_p_addrt2q(q,run_tree2);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_ptr_equal(q->blocked,NULL);
    spec_is_ptr_equal(q->active,NULL);
    spec_is_ptr_equal(q->completed,e);
    spec_is_ptr_equal(q->completed->next,e2);

    // and the conversation should be cleaned up and the CONVERSE run-tree should have reduced
    spec_is_str_equal(t2s(cons),"(CONVERSATIONS)");
    // and the run tree should be completed with the value from the COMPLETE instruction
    spec_is_str_equal(t2s(run_tree2),"(RUN_TREE (CONVERSATION_UUID) (PARAMS))");

    // and the second signal shouldn't have the conversation id in it and it's body should be the
    // 'with' value from the COMPLETE
    spec_is_str_equal(t2s(ps),"(PENDING_SIGNALS (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:100)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (CONVERSATION_UUID)) (BODY:{(TEST_INT_SYMBOL:31415)}))) (SIGNAL (ENVELOPE (SIGNAL_UUID)) (MESSAGE (HEAD (FROM_ADDRESS (RECEPTOR_ADDR:3)) (TO_ADDRESS (RECEPTOR_ADDR:99)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING)) (BODY:{(TEST_INT_SYMBOL:123)}))))");


    // setup a case for testing the COMPLETE instruction within the CONVERSE SCOPE
    p = _t_newr(0,CONVERSE);
    scope = _t_newr(p,SCOPE);
    T *complete = _t_newr(scope,COMPLETE);
    _t_newi(complete,TEST_INT_SYMBOL,321);
    _t_newi(scope,TEST_INT_SYMBOL,123);

    run_tree = __p_build_run_tree(p,0);
    _t_free(p);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:CONVERSE (SCOPE (process:COMPLETE (TEST_INT_SYMBOL:321)) (TEST_INT_SYMBOL:123))) (PARAMS))");
    e =_p_addrt2q(q,run_tree);
    //debug_enable(D_REDUCE+D_REDUCEV);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    debug_disable(D_STEP);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_INT_SYMBOL:321) (PARAMS))");

    _r_free(r);
}

void testProcessConverseListen() {

    T *code = _t_parse(G_sem,0,"(NEW (NEW_TYPE:TEST_INT_SYMBOL) (PARAM_REF:/2/1))");

    T *signature = __p_make_signature("result",SIGNATURE_PASSTHRU,NULL_STRUCTURE,
                                      "int",SIGNATURE_SYMBOL,TEST_INT_SYMBOL,
                                      NULL);
    Process p = _d_define_process(G_sem,code,"new_int","",signature,NULL,TEST_CONTEXT);

    code = _t_parse(G_sem,0,"(CONVERSE (SCOPE (LISTEN (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TEST_INT_SYMBOL) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:new_int (TEST_INT_SYMBOL:314)))) (BOOLEAN:1))");

    T *run_tree = __p_build_run_tree(code,0);
    _t_free(code);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:CONVERSE (SCOPE (process:LISTEN (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TEST_INT_SYMBOL) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:new_int (TEST_INT_SYMBOL:314)))) (BOOLEAN:1)) (PARAMS))");

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *cons = r->conversations;
    T *ps = r->pending_signals;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_ptr_equal(q->blocked,e);

    T *ex = __r_get_expectations(r,DEFAULT_ASPECT);
    spec_is_str_equal(t2s(ex),"(EXPECTATIONS (EXPECTATION (CARRIER:TEST_INT_SYMBOL) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_INT_SYMBOL))) (ACTION:new_int (TEST_INT_SYMBOL:314)) (PARAMS (SLOT (USAGE:NULL_SYMBOL))) (END_CONDITIONS (UNLIMITED)) (CONVERSATION_UUID)))");

    T *t = _t_newi(0,TEST_INT_SYMBOL,314);
    //    debug_enable(D_LISTEN+D_SIGNALS);
    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TEST_INT_SYMBOL,t,0,0,0);
    _r_deliver(r,s);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    // we should have no instances in the instance store, because the signal wasn't part of the conversation
    spec_is_str_equal(t2s(r->instances),"");

    T *cid = _t_clone(_t_getv(cons,1,ConversationIdentIdx,TREE_PATH_TERMINATOR));
    s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TEST_INT_SYMBOL,_t_clone(t),0,0,cid);
    _r_deliver(r,s);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    debug_disable(D_LISTEN+D_SIGNALS);
    spec_is_str_equal(t2s(r->instances),"(INSTANCE_STORE (INSTANCES (SYMBOL_INSTANCES:TEST_INT_SYMBOL (TEST_INT_SYMBOL:314))))");

    _r_free(r);
}

void testProcessThisScope() {
    T *code = _t_new_root(CONVERSE);
    T *scope = _t_newr(code,SCOPE);
    _t_newr(scope,THIS_SCOPE);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *run_tree = __p_build_run_tree(code,0);
    _t_free(code);

    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:CONVERSE (SCOPE (process:THIS_SCOPE))) (PARAMS))");

    // add the run tree into a queue and run it
    G_next_process_id = 0; // reset the process ids so the test will always work
    Q *q = r->q;
    T *cons = r->conversations;
    Qe *e =_p_addrt2q(q,run_tree);
    R *c = e->context;

    //debug_enable(D_STEP);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    debug_disable(D_STEP);
    spec_is_str_equal(t2s(cons),"(CONVERSATIONS (CONVERSATION (CONVERSATION_UUID) (END_CONDITIONS (UNLIMITED)) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1))))");

    // should reduce to the conversations ID because of the THIS_SCOPE instruction
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (CONVERSATION_UUID) (PARAMS))");
    _r_free(r);

}

void testProcessQuote() {
    T *t = _t_new_root(RUN_TREE);

    // test process quoting
    T *n = _t_new_root(QUOTE);

    T *a = _t_newr(n,ADD_INT);
    _t_newi(a,TEST_INT_SYMBOL,99);
    _t_newi(a,TEST_INT_SYMBOL,100);

    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(t),"(RUN_TREE (process:ADD_INT (TEST_INT_SYMBOL:99) (TEST_INT_SYMBOL:100)))");

    _t_free(n);
    _t_free(t);
}

void testProcessStream() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    FILE *stream;
    char buffer[500] = "line1\nabc\n";
    stream = fmemopen(buffer, 500, "r+");

    // test the basic case of the STREAM_ALIVE process which returns
    // a boolean if the stream is readable or not.
    T *n = _t_new_root(STREAM_ALIVE);
    Stream *st = _st_new_unix_stream(stream,1);
    _t_new_cptr(n,EDGE_STREAM,st);
    __p_reduce_sys_proc(0,STREAM_ALIVE,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);

    // test reading a stream
    n = _t_new_root(STREAM_READ);
    _t_new_cptr(n,EDGE_STREAM,st);
    _t_news(n,RESULT_SYMBOL,TEST_STR_SYMBOL);

    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);
    R *c = e->context;

    // after reduction the context should be in the blocked state

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_equal(c->state,Block);

    // wait for read to complete, after which it should have also unblocked the
    // context which should thus be ready for reduction again.
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(q->contexts_count,1);

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (TEST_STR_SYMBOL:line1) (PARAMS))");

    // now test reading again but this time into an ASCII_CHARS tree
    n = _t_new_root(STREAM_READ);
    _t_new_cptr(n,EDGE_STREAM,st);
    _t_news(n,RESULT_SYMBOL,ASCII_CHARS);

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    while(!(st->flags&StreamHasData) && st->flags&StreamAlive ) {sleepms(1);};
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (ASCII_CHARS (ASCII_CHAR:'a') (ASCII_CHAR:'b') (ASCII_CHAR:'c')) (PARAMS))");

    // test writing to the stream
    fseek(stream,strlen(buffer),SEEK_SET);
    n = _t_new_root(STREAM_WRITE);
    _t_new_cptr(n,EDGE_STREAM,st);
    _t_new_str(n,TEST_STR_SYMBOL,"fish\n");
    _t_new_str(n,LINE,"cow");
    T *lns = _t_newr(n,LINES);
    _t_new_str(lns,LINE,"thing1");
    _t_new_str(lns,LINE,"thing2");
    _t_newi(n,TEST_INT_SYMBOL,314);

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);

    spec_is_equal(_p_reduceq(q),noReductionErr);

    char *expected_result = "line1\nabc\nfish\ncow\nthing1\nthing2\n(TEST_INT_SYMBOL:314)";

    spec_is_str_equal(buffer,expected_result);

    _st_free(st);


    // test writing to a readonly stream
    stream = fmemopen(buffer, strlen (buffer), "r");
    st = _st_new_unix_stream(stream,0);
    n = _t_new_root(STREAM_WRITE);

    _t_new_cptr(n,EDGE_STREAM,st);
    _t_new_str(n,TEST_STR_SYMBOL,"fish\n");

    run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    // context should have recorded the err
    spec_is_equal(e->context->err,unixErrnoReductionErr);

    // buffer should remain unchanged
    spec_is_str_equal(buffer,expected_result);

    //    debug_enable(D_STREAM+D_SOCKET+D_SIGNALS);

    _st_kill(st);
    n = _t_new_root(STREAM_ALIVE);
    _t_new_cptr(n,EDGE_STREAM,st);
    __p_reduce_sys_proc(0,STREAM_ALIVE,n,0);
    spec_is_str_equal(t2s(n),"(BOOLEAN:0)");
    _t_free(n);

    _st_free(st);
    _r_free(r);
    debug_disable(D_STREAM+D_SOCKET+D_SIGNALS);

}

void testProcessStreamClose() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    FILE *stream;
    char buffer[500] = "line1\nabc\n";
    stream = fmemopen(buffer, 500, "r+");

    T *n = _t_new_root(STREAM_CLOSE);
    Stream *st = _st_new_unix_stream(stream,true);
    _t_new_cptr(n,EDGE_STREAM,st);
    //debug_enable(D_STREAM+D_SOCKET+D_SIGNALS);
    __p_reduce_sys_proc(0,STREAM_CLOSE,n,0);
    //@todo figure out what STREAM_CLOSE should really return
    spec_is_str_equal(t2s(n),"(BOOLEAN:1)");
    _t_free(n);
    debug_disable(D_STREAM+D_SOCKET+D_SIGNALS);

    _r_free(r);
}


void testProcessInitiate(){
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;
    Protocol time;
    __sem_get_by_label(G_sem,"time",&time,CLOCK_CONTEXT);

    T *n = _t_new_root(INITIATE_PROTOCOL);
    T *p = _t_news(n,PNAME,time);
    T *i = _t_news(n,WHICH_INTERACTION,tell_time);
    T *bindings = _t_newr(n,PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_TELLER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_HEARER);
    ReceptorAddress clock_addr = {3}; // @todo bogus!!! fix getting clock address somehow
    __r_make_addr(w,ACTUAL_RECEPTOR,clock_addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,REQUEST_HANDLER);
    T *noop = _t_new_root(NOOP);
    Process proc = _r_define_process(r,noop,"do nothing","long desc...",NULL,NULL);
    _t_news(w,ACTUAL_PROCESS,proc);

    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Qe *e = _p_addrt2q(q,run_tree);

    spec_is_str_equal(_td(r,r->pending_signals),"(PENDING_SIGNALS)");
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(_t_children(r->pending_signals),1);

    _r_free(r);
}

//-----------------------------------------------------------------------------------------
// tests of process execution (reduction)

void testProcessReduce() {
    T *t = _t_new_root(RUN_TREE);

    T *n = _t_new_root(IF); // multi-level IF to test descending a number of levels
    _t_newi(n,BOOLEAN,1);
    T *n1 = _t_newr(n,IF);
    T *n2 = _t_newr(n1,IF);
    _t_newi(n2,BOOLEAN,0);
    _t_newi(n2,BOOLEAN,1);
    _t_newi(n2,BOOLEAN,0);
    _t_newi(n1,TEST_INT_SYMBOL,98);
    _t_newi(n1,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,100);

    T *c = _t_rclone(n);
    _t_add(t,c);

    R *context = __p_make_context(t,0,99,NULL);

    spec_is_equal(context->id,99);

    // build a fake Receptor and Q on the stack so _p_step will work
    Receptor r;
    Q q;
    r.root = NULL;
    r.q = &q;
    q.r = &r;

    spec_is_equal(rt_cur_child(c),0);
    // first step is Eval and next step is Descend
    spec_is_equal(_p_step(&q,&context),Descend);
    spec_is_equal(rt_cur_child(c),0);

    // next step after Descend changes node pointer and moved to Eval
    spec_is_equal(_p_step(&q,&context),Eval);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,1));
    spec_is_equal(rt_cur_child(c),1);

    // after Eval next step will be Ascend
    spec_is_equal(_p_step(&q,&context),Ascend);

    // step ascends back to top if and marks boolean complete
    spec_is_equal(_p_step(&q,&context),Eval);

    spec_is_equal(rt_cur_child(_t_child(c,1)),RUN_TREE_EVALUATED);
    spec_is_ptr_equal(context->node_pointer,c);

    spec_is_equal(_p_step(&q,&context),Descend);

    // third step goes into the second level if
    spec_is_equal(_p_step(&q,&context),Eval);
    spec_is_equal(rt_cur_child(c),2);
    spec_is_ptr_equal(context->node_pointer,_t_child(c,2));

    free(context);
    // not specing out all the steps because there are soooo many...

    // just re-running them all for final result
    _t_free(_t_detach_by_idx(t,1));
    c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(c),"(TEST_INT_SYMBOL:99)");

    _t_free(n);
    _t_free(t);
}

void testProcessParameter() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *n = _t_parse(G_sem,0,"(TEST_ANYTHING_SYMBOL (DISSOLVE (PARAMETER (PARAMETER_REFERENCE (PARAM_PATH:/2/1)) (PARAMETER_RESULT (RESULT_VALUE)))) (PARAMETER (PARAMETER_REFERENCE (PARAM_PATH:/2/2)) (PARAMETER_RESULT (RESULT_SYMBOL:TEST_SYMBOL_SYMBOL))) (PARAMETER (PARAMETER_REFERENCE (PARAM_PATH:/2/1)) (PARAMETER_RESULT (RESULT_LABEL:ENGLISH_LABEL))))");

    T *p1 = _t_newi(0,TEST_INT_SYMBOL,314);
    T *p2 = _t_new_str(0,TEST_STR_SYMBOL,"314");
    T *run_tree = __p_build_run_tree(n,2,p1,p2);
    _t_free(p1);
    _t_free(p2);
    _t_free(n);
    Q *q = r->q;
    Qe *e = _p_addrt2q(q,run_tree);

    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_str_equal(t2s(run_tree), "(RUN_TREE (TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL:314) (TEST_SYMBOL_SYMBOL:TEST_STR_SYMBOL) (ENGLISH_LABEL:TEST_INT_SYMBOL)) (PARAMS (TEST_INT_SYMBOL:314) (TEST_STR_SYMBOL:314)))");

    _r_free(r);
}

void testProcessRefs() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    int pt2[] = {SignalMessageIdx,MessageBodyIdx,0,TREE_PATH_TERMINATOR};

    T *n = _t_newr(0,NOOP);
    T *t = _t_newr(n,TEST_ANYTHING_SYMBOL);
    _t_new(t,PARAM_REF,pt1,sizeof(int)*4);
    _t_new(t,SIGNAL_REF,pt2,sizeof(int)*4);
    t = _t_newi(0,TEST_INT_SYMBOL,314);  // a param to the run tree
    T *run_tree = __p_build_run_tree(n,1,t);
    _t_free(n);
    _t_free(t);
    ReceptorAddress fm = {3}; // DUMMY ADDR
    ReceptorAddress to = {4}; // DUMMY ADDR
    T *signal = __r_make_signal(fm,to,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"foo"),0,0,0);

    // simulate that this run-tree is on the flux.
    _t_add(signal,run_tree);

    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    spec_is_str_equal(t2s(run_tree), "(RUN_TREE (TEST_ANYTHING_SYMBOL (TEST_INT_SYMBOL:314) (TEST_STR_SYMBOL:foo)) (PARAMS (TEST_INT_SYMBOL:314)))");

    // now remove the run-tree from the signal or it won't get cleanup up by the free.
    _t_detach_by_ptr(signal,run_tree);
    _t_free(signal);

    _r_free(r);
}

Process G_ifeven;
/**
 * helper to generate an example process definition that acts as an if for even numbers
 *
 * @snippet spec/process_spec.h defIfEven
 */
//! [defIfEven]
void _defIfEven() {
    T *code = _t_parse(G_sem,0,"(IF (EQ_INT (MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");

    T *signature = __p_make_signature("result",SIGNATURE_PASSTHRU,NULL_STRUCTURE,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      "true_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                      "false_branch",SIGNATURE_ANY,NULL_STRUCTURE,
                                      NULL);

    G_ifeven =  _d_define_process(G_sem,code,"if even","return 2nd child if even, third if not",signature,NULL,TEST_CONTEXT);
}
//! [defIfEven]

/**
 * helper to generate an example process definition that creates a divide by zero error
 *
 * @snippet spec/process_spec.h defDivZero
 */
//! [defDivZero]
Process _defDivZero() {
    T *code;

    /* a process that would look something like this in lisp:
       (defun div_zero (val) (/ val 0))
    */
    code = _t_new_root(DIV_INT);                       // IF is a system process
    _t_newi(code,TEST_INT_SYMBOL,2);
    _t_newi(code,TEST_INT_SYMBOL,0);

    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "val",SIGNATURE_STRUCTURE,INTEGER,
                                      NULL);

    return _d_define_process(G_sem,code,"divByZero","create a divide by zero error",signature,NULL,TEST_CONTEXT);
}
//! [defDivZero]

void testProcessReduceDefinedProcess() {
    //! [testProcessReduceDefinedProcess]

    Process if_even = G_ifeven;
    T *processes = __sem_get_defs(G_sem,SEM_TYPE_PROCESS,TEST_CONTEXT);

    // check that it dumps nicely, including showing the param_refs as paths
    int p[] = {if_even.id,ProcessDefCodeIdx,TREE_PATH_TERMINATOR};

    spec_is_str_equal(t2s(_t_get(processes,p)),"(process:IF (process:EQ_INT (process:MOD_INT (PARAM_REF:/2/1) (TEST_INT_SYMBOL:2)) (TEST_INT_SYMBOL:0)) (PARAM_REF:/2/2) (PARAM_REF:/2/3))");


    // create a run tree right in the state to "call"this function
    T *n = _t_new_root(PARAMS);
    _t_newi(n,TEST_INT_SYMBOL,99);
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);
    T *t = _p_make_run_tree(G_sem,if_even,n,NULL);

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(G_sem,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_INT_SYMBOL:124)");

    _t_free(t);_t_free(n);
    //! [testProcessReduceDefinedProcess]
}

void testProcessSignatureMatching() {
    Process if_even = G_ifeven;

    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(if_even);
    T *x = _t_new(n,TEST_STR_SYMBOL,"test",5);  // this should be an INTEGER!!
    _t_newi(n,TEST_INT_SYMBOL,123);
    _t_newi(n,TEST_INT_SYMBOL,124);

    spec_is_equal(__p_check_signature(G_sem,if_even,n,NULL),signatureMismatchReductionErr);

    T *c = _t_rclone(n);
    _t_add(t,c);
    spec_is_equal(_p_reduce(G_sem,t),signatureMismatchReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // too few params
    c = _t_rclone(n);
    _t_add(t,c);
    _t_free(_t_detach_by_idx(c,1));
    spec_is_equal(_p_reduce(G_sem,t),tooFewParamsReductionErr);
    _t_free(_t_detach_by_idx(t,1));

    // add too many params
    // fix the mismatch in the first param so the test won't fail by mismatch type
    int v = 0;
    __t_morph(x,TEST_INT_SYMBOL,&v,sizeof(int),0);
    c = _t_rclone(n);
    // add in the extra param
    _t_add(t,c);
    __t_newi(c,TEST_INT_SYMBOL,124,1);
    spec_is_equal(_p_reduce(G_sem,t),tooManyParamsReductionErr);

    _t_free(t);
    _t_free(n);

    n = _t_new_root(send_request);
    // test missing semantic map
    spec_is_equal(__p_check_signature(G_sem,send_request,n,NULL),missingSemanticMapReductionErr);
    T *sm = _t_new_root(SEMANTIC_MAP);
    // test map without matching expected slots
    spec_is_equal(__p_check_signature(G_sem,send_request,n,sm),mismatchSemanticMapReductionErr);
    _t_free(sm);
    //(TEMPLATE_SIGNATURE (EXPECTED_SLOT (ROLE:RESPONDER)) (EXPECTED_SLOT (USAGE:REQUEST_DATA)) (EXPECTED_SLOT (USAGE:RESPONSE_DATA)) (EXPECTED_SLOT (GOAL:REQUEST_HANDLER)))
    sm = _t_build(G_sem,0,
                  SEMANTIC_MAP,
                  SEMANTIC_LINK,
                  USAGE,REQUEST_DATA,
                  REPLACEMENT_VALUE,ACTUAL_SYMBOL,PING,NULL_SYMBOL,NULL_SYMBOL,
                  SEMANTIC_LINK,
                  USAGE,RESPONSE_DATA,
                  REPLACEMENT_VALUE,ACTUAL_SYMBOL,PING,NULL_SYMBOL,NULL_SYMBOL,
                  SEMANTIC_LINK,
                  GOAL,RESPONSE_HANDLER,
                  REPLACEMENT_VALUE,ACTUAL_PROCESS,NOOP,NULL_SYMBOL,NULL_SYMBOL,
                  SEMANTIC_LINK,
                  GOAL,REQUEST_HANDLER,
                  REPLACEMENT_VALUE,ACTUAL_PROCESS,NOOP,NULL_SYMBOL,
                  NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL
             );
    spec_is_str_equal(t2s(sm),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))))");

    spec_is_equal(__p_check_signature(G_sem,send_request,n,sm),mismatchSemanticMapReductionErr);

    _t_build(G_sem,sm,
             SEMANTIC_LINK,
             ROLE,RECOGNIZER,
             REPLACEMENT_VALUE,ACTUAL_RECEPTOR,FROM_ADDRESS,RECEPTOR_ADDR,3,NULL_SYMBOL,NULL_SYMBOL);

    spec_is_str_equal(t2s(sm),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))) (SEMANTIC_LINK (ROLE:RECOGNIZER) (REPLACEMENT_VALUE (ACTUAL_RECEPTOR (FROM_ADDRESS (RECEPTOR_ADDR:3))))))");

    spec_is_equal(__p_check_signature(G_sem,send_request,n,sm),mismatchSemanticMapReductionErr);

    _t_free(_t_detach_by_idx(sm,_t_children(sm)));

    _t_build(G_sem,sm,
             SEMANTIC_LINK,
             ROLE,RESPONDER,
             REPLACEMENT_VALUE,ACTUAL_RECEPTOR,FROM_ADDRESS,RECEPTOR_ADDR,3,NULL_SYMBOL,NULL_SYMBOL);

    _t_build(G_sem,sm,
             SEMANTIC_LINK,
             USAGE,CHANNEL,
             REPLACEMENT_VALUE,ASPECT_IDENT,DEFAULT_ASPECT,NULL_SYMBOL,NULL_SYMBOL,NULL_SYMBOL);

    spec_is_str_equal(t2s(sm),"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (ACTUAL_SYMBOL:PING))) (SEMANTIC_LINK (GOAL:RESPONSE_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (ACTUAL_PROCESS:NOOP))) (SEMANTIC_LINK (ROLE:RESPONDER) (REPLACEMENT_VALUE (ACTUAL_RECEPTOR (FROM_ADDRESS (RECEPTOR_ADDR:3))))) (SEMANTIC_LINK (USAGE:CHANNEL) (REPLACEMENT_VALUE (ASPECT_IDENT:DEFAULT_ASPECT))))");

    spec_is_equal(__p_check_signature(G_sem,send_request,n,sm),noReductionErr);

    _t_free(n);
    _t_free(sm);

    n = _t_new_root(fill_i_am);

    spec_is_equal(__p_check_signature(G_sem,fill_i_am,n,NULL),noReductionErr);
    _t_free(n);

}

void testProcessError() {
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_parse(G_sem,0,"(NOOP (DIV_INT (TEST_INT_SYMBOL:100) (TEST_INT_SYMBOL:0)))");
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the reduction error
    int pt[] = {RunTreeErrorParamsIdx,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,true);

    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessRaise() {
    T *n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,ZERO_DIVIDE_ERR);

    spec_is_equal(__p_reduce_sys_proc(0,RAISE,n,0),raiseReductionErr);
    _t_free(n);

    T *t = _t_new_root(RUN_TREE);
    n = _t_new_root(RAISE);
    _t_news(n,REDUCTION_ERROR_SYMBOL,NOT_A_PROCESS_ERR); // pick a random error to raise
    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the reduction error
    int pt[] = {RunTreeErrorParamsIdx,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,true);

    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(NOT_A_PROCESS_ERR (ERROR_LOCATION:/1))");
    _t_free(n);
    _t_free(t);
}

void testProcessIterate() {
    FILE *output;

    char *output_data = NULL;
    size_t size;
    output = open_memstream(&output_data,&size);

    // an iterate process that writes to a stream and 3 times
    Stream *st = _st_new_unix_stream(output,0);
    T *code = _t_parse(G_sem,0,"(ITERATE (PARAMS) (TEST_INT_SYMBOL:3) (STREAM_WRITE % (LINE:\"testing\")))",_t_new_cptr(0,EDGE_STREAM,st));

    T *t = __p_build_run_tree(code,0);
    Error e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(output_data,"testing\ntesting\ntesting\n");
    //    spec_is_str_equal(t2s(t),"xxx"); @todo something here when we figure out return value

    _t_free(t);
    _t_free(code);

    // now test iteration with a condition instead of an INTEGER
    //  a condition that checks to see if the param is less than 3
    code = _t_parse(G_sem,0,"(ITERATE (PARAMS (TEST_INT_SYMBOL:314)) (LT_INT (PARAM_REF:/1/1/1) (TEST_INT_SYMBOL:3)) (STREAM_WRITE % (LINE:\"testing\")))",_t_new_cptr(0,EDGE_STREAM,st));

    t = __p_build_run_tree(code,0);
    e = _p_reduce(G_sem,t);
    spec_is_equal(e,noReductionErr);
    spec_is_str_equal(output_data,"testing\ntesting\ntesting\n");
    _t_free(t);

    _st_free(st);
    free(output_data);
    _t_free(code);
}

void testProcessIterateOnSymbol() {
    FILE *output;

    char *output_data = NULL;
    size_t size;
    output = open_memstream(&output_data,&size);

    // an iterate process that writes to a stream and 3 times
    Stream *st = _st_new_unix_stream(output,0);
    T *code = _t_parse(G_sem,0,"(ITERATE (PARAMS) (ITERATE_ON_SYMBOL:TEST_STR_SYMBOL) (STREAM_WRITE % (PARAM_REF:/1/1/1/1)))",_t_new_cptr(0,EDGE_STREAM,st));
    T *run_tree = __p_build_run_tree(code,0);
    _t_free(code);

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    Xaddr x;
    T *t = _t_new_str(0,TEST_STR_SYMBOL,"thing1 ");
    x = _r_new_instance(r,t);
    t = _t_new_str(0,TEST_STR_SYMBOL,"thing2 ");
    x = _r_new_instance(r,t);

    Q *q = r->q;
    Qe *e = _p_addrt2q(q,run_tree);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->completed,e);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (REDUCTION_ERROR_SYMBOL:NULL_SYMBOL) (PARAMS))");

    spec_is_str_equal(output_data,"thing1 thing2 ");

    _st_free(st);
    free(output_data);
    _r_free(r);
}

void testProcessListen() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    // test regular asynchronous listening.
    T *n = _t_parse(G_sem,0,"(LISTEN (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TICK) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK))) (ACTION:NOOP (TEST_INT_SYMBOL:314)))");
    spec_is_equal(__p_reduce_sys_proc(0,LISTEN,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(REDUCTION_ERROR_SYMBOL:NULL_SYMBOL)"); //@todo is this right??
    _t_free(n);

    T *ex = __r_get_expectations(r,DEFAULT_ASPECT);
    spec_is_str_equal(t2s(ex),"(EXPECTATIONS (EXPECTATION (CARRIER:TICK) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TICK))) (ACTION:NOOP (TEST_INT_SYMBOL:314)) (PARAMS (SLOT (USAGE:NULL_SYMBOL))) (END_CONDITIONS (UNLIMITED))))");

    _r_remove_expectation(r, _t_child(ex,1));

    // test listen that blocks
    n = _t_parse(G_sem,0,"(LISTEN (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:TESTING) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL))))");

    G_next_process_id = 0; // reset the process ids so the test will always work
    T *run_tree = __p_build_run_tree(n,0);
    _t_free(n);
    Q *q = r->q;
    Qe *e = _p_addrt2q(q,run_tree);

    //debug_enable(D_LISTEN+D_SIGNALS);
    spec_is_equal(_p_reduceq(q),noReductionErr);
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->blocked,e);
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (process:LISTEN) (PARAMS))");

    spec_is_str_equal(t2s(__r_get_expectations(r,DEFAULT_ASPECT)),"(EXPECTATIONS (EXPECTATION (CARRIER:TESTING) (PATTERN (SEMTREX_SYMBOL_LITERAL (SEMTREX_SYMBOL:TEST_STR_SYMBOL))) (WAKEUP_REFERENCE (PROCESS_IDENT:1) (CODE_PATH:/1)) (PARAMS (SLOT (USAGE:NULL_SYMBOL))) (END_CONDITIONS (COUNT:1))))");

    T *s = __r_make_signal(r->addr,r->addr,DEFAULT_ASPECT,TESTING,_t_new_str(0,TEST_STR_SYMBOL,"fishy!"),0,0,0);
    _r_deliver(r,s);
    spec_is_equal(_p_reduceq(q),noReductionErr);

    // because this listen blocked, and can thus only be woken up once, the expectation
    // had to have been removed afterwards.
    spec_is_str_equal(t2s(__r_get_expectations(r,DEFAULT_ASPECT)),"(EXPECTATIONS)");

    // @todo, ok the two params thing here is wrong, but we don't actually have
    // a use case for the blocking listen, so I don't quite know how it should work... @FIXME
    spec_is_str_equal(t2s(run_tree),"(RUN_TREE (PARAMS (TEST_STR_SYMBOL:fishy!)) (PARAMS))");
    debug_disable(D_LISTEN);
    _r_free(r);
}

void testProcessSelfAddr() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);

    T *n = _t_new_root(SELF_ADDR);
    _t_news(n,RESULT_SYMBOL,TO_ADDRESS);
    spec_is_equal(__p_reduce_sys_proc(0,SELF_ADDR,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(TO_ADDRESS (RECEPTOR_ADDR:3))");
    _t_free(n);
    _r_free(r);
}

void testProcessGetLabel() {
    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    T *n = _t_parse(G_sem,0,"(GET_LABEL (LABEL_SYMBOL:CONTENT_TYPE) (LABEL_TYPE:HTTP_HEADER_LABEL) (RESULT_SYMBOL:LINE))");
    spec_is_equal(__p_reduce_sys_proc(0,GET_LABEL,n,r->q),noReductionErr);
    spec_is_str_equal(t2s(n),"(LINE:Content-Type)");
    _t_free(n);
    _r_free(r);
}

void testProcessErrorTrickleUp() {
    //! [testProcessErrorTrickleUp]
    Process divz = _defDivZero();  // add the if_even process to our defs

    // create a run tree right in the position to "call" this function
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_new_root(NOOP);
    T *d = _t_newr(n,divz);
    _t_newi(d,TEST_INT_SYMBOL,124);

    T *c = _t_rclone(n);
    _t_add(t,c);
    T *ps = _t_newr(t,PARAMS);

    // error routine is just a param ref to pass back the error tree
    int pt[] = {4,1,TREE_PATH_TERMINATOR};
    __t_new(t,PARAM_REF,pt,sizeof(int)*4,1);

    // confirm that it reduces correctly
    spec_is_equal(_p_reduce(G_sem,t),noReductionErr);
    spec_is_str_equal(t2s(_t_child(t,1)),"(ZERO_DIVIDE_ERR (ERROR_LOCATION:/1/1))");

    _t_free(t);_t_free(n);
    //! [testProcessErrorTrickleUp]
}

void testProcessMulti() {
    //! [testProcessMulti]

    Receptor *r = _r_new(G_sem,TEST_RECEPTOR);
    Q *q = r->q;

    Process if_even = G_ifeven;

    // create two run trees
    T *n = _t_new_root(PARAMS);
    __t_newi(n,TEST_INT_SYMBOL,99,1);
    __t_newi(n,TEST_INT_SYMBOL,123,1);
    __t_newi(n,TEST_INT_SYMBOL,124,1);
    T *t1 = _p_make_run_tree(G_sem,if_even,n,NULL);
    __t_newi(n,TEST_INT_SYMBOL,100,1);
    T *l2 = __t_new(n,if_even,0,0,1);
    __t_newi(l2,TEST_INT_SYMBOL,2,1);
    __t_newi(l2,TEST_INT_SYMBOL,123,1);
    __t_newi(l2,TEST_INT_SYMBOL,124,1);
    __t_newi(n,TEST_INT_SYMBOL,314,1);

    T *t2 = _p_make_run_tree(G_sem,if_even,n,NULL);

    // add them to a processing queue
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->active,NULL);
    _p_addrt2q(q,t1);
    spec_is_equal(q->contexts_count,1);
    spec_is_ptr_equal(q->active->context->run_tree,t1);
    spec_is_ptr_equal(q->active->prev,NULL);
    _p_addrt2q(q,t2);
    spec_is_equal(q->contexts_count,2);
    spec_is_ptr_equal(q->active->context->run_tree,t1);
    spec_is_ptr_equal(q->active->next->context->run_tree,t2);
    spec_is_ptr_equal(q->active->next->prev, q->active);

    spec_is_long_equal(q->active->accounts.elapsed_time,0);
    spec_is_long_equal(q->active->next->accounts.elapsed_time,0);

    // confirm that they both reduce correctly
    pthread_t thread;
    int rc;
    rc = pthread_create(&thread,0,_p_reduceq_thread,q);
    if (rc){
        raise_error("ERROR; return code from pthread_create() is %d\n", rc);
    }
    void *status;
    rc = pthread_join(thread, &status);
    if (rc) {
        raise_error("ERROR; return code from pthread_join() is %d\n", rc);
    }
    spec_is_long_equal((long)status,noReductionErr);

    // contexts have been moved to the completed list and now have
    // some elapsed time data associated with them.
    spec_is_equal(q->contexts_count,0);
    spec_is_ptr_equal(q->completed->context->run_tree,t2);
    uint64_t et1,et2;
    spec_is_true((et1 = q->completed->accounts.elapsed_time)>0);
    spec_is_ptr_equal(q->completed->next->context->run_tree,t1);
    spec_is_true((et2 = q->completed->next->accounts.elapsed_time)>0);

    // confirm the results of the two run trees
    spec_is_str_equal(t2s(_t_child(t1,1)),"(TEST_INT_SYMBOL:124)");
    spec_is_str_equal(t2s(_t_child(t2,1)),"(TEST_INT_SYMBOL:123)");

    _p_cleanup(q);

    spec_is_ptr_equal(q->completed,NULL);
    // confirm that after cleaning up the two processes, the accounting information
    // is updated in a receptor state structure
    char buf[200];
    sprintf(buf,"(RECEPTOR_ELAPSED_TIME:%ld)",et1+et2);
    spec_is_str_equal(t2s(_t_child(_t_child(r->root,ReceptorInstanceStateIdx),ReceptorElapsedTimeIdx)),buf);

    _t_free(n);
    _r_free(r);
    //! [testProcessMulti]
}

void testRunTreeTemplate() {

    T *params = _t_new_root(PARAMS);
    T *sm = _t_parse(G_sem,0,"(SEMANTIC_MAP (SEMANTIC_LINK (USAGE:CHANNEL) (REPLACEMENT_VALUE (ASPECT_IDENT:DEFAULT_ASPECT))) (SEMANTIC_LINK (USAGE:REQUEST_DATA) (REPLACEMENT_VALUE (PING))) (SEMANTIC_LINK (USAGE:RESPONSE_DATA) (REPLACEMENT_VALUE (PING))) (SEMANTIC_LINK (ROLE:RESPONDER) (REPLACEMENT_VALUE (TO_ADDRESS (RECEPTOR_ADDR:3)))) (SEMANTIC_LINK (GOAL:REQUEST_HANDLER) (REPLACEMENT_VALUE (NOOP))))");

    T *r = _p_make_run_tree(G_sem,send_request,params,sm);
    _t_free(params);
    spec_is_str_equal(t2s(r),"(RUN_TREE (process:NOOP (process:REQUEST (TO_ADDRESS (RECEPTOR_ADDR:3)) (ASPECT_IDENT:DEFAULT_ASPECT) (CARRIER:backnforth) (PING) (CARRIER:backnforth))) (PARAMS))");
    _t_free(r);
    _t_free(sm);
}

void testProcessContinue() {

    // CONTINUE allows you to restart reduction some other place on the tree.
    T *t = _t_new_root(RUN_TREE);
    T *n = _t_parse(G_sem,0,"(NOOP (IF (BOOLEAN:0) (CONTINUE (CONTINUE_LOCATION:/1) (CONTINUE_VALUE (TEST_STR_SYMBOL:\"fish\"))) (TEST_INT_SYMBOL:413)))");
    T *c = _t_rclone(n);
    _t_add(t,c);
    _p_reduce(G_sem,t);

    spec_is_str_equal(t2s(_t_child(t,1)),"(TEST_STR_SYMBOL:fish)");
    _t_free(t);
    _t_free(n);
}

void testProcess() {
    _defIfEven();
    testProcessParameter();
    testRunTree();
    testProcessGet();
    testProcessDel();
    testProcessNew();
    testProcessDefine();
    testProcessDo();
    testProcessTranscode();
    testProcessDissolve();
    testProcessSemtrex();
    testProcessFill();
    testProcessFillMatch();
    testProcessFillMatchFull();
    testProcessIf();
    testProcessCond();
    testProcessSym();
    testProcessIntMath();
    testProcessPath();
    testProcessString();
    testProcessRespond();
    testProcessSay();
    testProcessRequest();
    testProcessConverse();
    testProcessConverseListen();
    testProcessThisScope();
    testProcessQuote();
    testProcessStream();
    testProcessStreamClose();
    testProcessInitiate();
    testProcessReduce();
    testProcessRefs();
    testProcessReduceDefinedProcess();
    testProcessSignatureMatching();
    testProcessError();
    testProcessRaise();
    testProcessIterate();
    testProcessIterateOnSymbol();
    testProcessListen();
    testProcessSelfAddr();
    testProcessGetLabel();
    testProcessErrorTrickleUp();
    testProcessMulti();
    testRunTreeTemplate();
    testProcessContinue();
}
