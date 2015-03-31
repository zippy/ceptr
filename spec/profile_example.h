/**
 * @file profile_example.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

#ifndef _CEPTR_TEST_PROFILE_EXAMPLE_H
#define _CEPTR_TEST_PROFILE_EXAMPLE_H

#include "../src/ceptr.h"
#include "../src/receptor.h"
#include "../src/def.h"
#include "../src/semtrex.h"


T *test_profile_symbols,*test_profile_structures;
Defs test_profile_defs;
Symbol FIRST_NAME;
Symbol LAST_NAME;
Structure NAME;
Symbol PROFILE_NAME;

Symbol STREET_ADDRESS;
Symbol CITY;
Symbol STATE;
Symbol ZIP;
Symbol COUNTRY;
Structure ADDRESS;
Symbol PROFILE_ADDRESS;

Symbol PROFILE_EMAIL;
Structure PROFILE;
Symbol USER_PROFILE;

Symbol MAILING_LABEL;

void _setupProfileDefs() {
    test_profile_defs.symbols = test_profile_symbols = _t_new_root(SYMBOLS);
    test_profile_defs.structures = test_profile_structures = _t_new_root(STRUCTURES);
    test_profile_defs.processes = _t_new_root(PROCESSES);
    Defs d = test_profile_defs;

    SY(d,FIRST_NAME,CSTRING);
    SY(d,LAST_NAME,CSTRING);
    ST(d,NAME,2,
       FIRST_NAME,
       LAST_NAME
       );
    SY(d,PROFILE_NAME,NAME);

    SY(d,STREET_ADDRESS,CSTRING);
    SY(d,CITY,CSTRING);
    SY(d,STATE,CSTRING);
    SY(d,ZIP,CSTRING);
    SY(d,COUNTRY,CSTRING);

    ST(d,ADDRESS,5,
       STREET_ADDRESS,
       CITY,
       STATE,
       ZIP,
       COUNTRY
       );
    SY(d,PROFILE_ADDRESS,ADDRESS);

    SY(d,PROFILE_EMAIL,CSTRING);
    ST(d,PROFILE,3,
       PROFILE_NAME,
       PROFILE_ADDRESS,
       PROFILE_EMAIL);
    SY(d,USER_PROFILE,PROFILE);

    SY(d,MAILING_LABEL,CSTRING);
}

void _cleanupProfileDefs() {
    _t_free(test_profile_symbols);
    _t_free(test_profile_structures);
}

void testProfileExample() {
    _setupProfileDefs();
    T *t = _t_new_root(USER_PROFILE);
    T *n = _t_newr(t,PROFILE_NAME);
    T *a = _t_newr(t,PROFILE_ADDRESS);
    T *e = _t_new_str(t,PROFILE_EMAIL,"test@example.com");
    _t_new_str(n,FIRST_NAME,"Jane");
    _t_new_str(n,LAST_NAME,"Smith");
    _t_new_str(a,STREET_ADDRESS,"126 Main Street");
    _t_new_str(a,CITY,"Smallville");
    _t_new_str(a,STATE,"CA");
    _t_new_str(a,ZIP,"12345");
    _t_new_str(a,COUNTRY,"USA");

    spec_is_str_equal(_t2s(&test_profile_defs,t),"(USER_PROFILE (PROFILE_NAME (FIRST_NAME:Jane) (LAST_NAME:Smith)) (PROFILE_ADDRESS (STREET_ADDRESS:126 Main Street) (CITY:Smallville) (STATE:CA) (ZIP:12345) (COUNTRY:USA)) (PROFILE_EMAIL:test@example.com))");

    T *input = _t_new_root(INPUT);
    T *i = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i,SIGNATURE_STRUCTURE,INTEGER);
    _t_new_str(i,INPUT_LABEL,"the_profile");
    i = _t_newr(input,INPUT_SIGNATURE);
    _t_news(i,SIGNATURE_STRUCTURE,PROFILE);

    T* output = _t_new_root(OUTPUT_SIGNATURE);
    i = _t_newr(output,OUTPUT_SIGNATURE);
    _t_news(i,SIGNATURE_STRUCTURE,CSTRING);  // should actually be MAILING_LABEL symbol or something like that

    T *processes = test_profile_defs.processes;

    T *code = _t_new_root(CONCAT_STR);

    int pt1[] = {2,1,1,1,TREE_PATH_TERMINATOR};
    int pt2[] = {2,1,1,2,TREE_PATH_TERMINATOR};
    int pt3[] = {2,1,2,1,TREE_PATH_TERMINATOR};
    int pt4[] = {2,1,2,2,TREE_PATH_TERMINATOR};
    int pt5[] = {2,1,2,3,TREE_PATH_TERMINATOR};
    int pt6[] = {2,1,2,4,TREE_PATH_TERMINATOR};

    _t_news(code,RESULT_SYMBOL,MAILING_LABEL);
    T *c = _t_newr(code,CONCAT_STR);
    //    _t_news(c,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_new(c,PARAM_REF,pt1,sizeof(int)*5);
    _t_new_str(c,TEST_STR_SYMBOL," ");
    _t_new(c,PARAM_REF,pt2,sizeof(int)*5);
    _t_new_str(code,TEST_STR_SYMBOL,"\\n");
    _t_new(code,PARAM_REF,pt3,sizeof(int)*5);
    _t_new_str(code,TEST_STR_SYMBOL,"\\n");
    c = _t_newr(code,CONCAT_STR);
    //    _t_news(c,RESULT_SYMBOL,TEST_NAME_SYMBOL);
    _t_new(c,PARAM_REF,pt4,sizeof(int)*5);
    _t_new_str(c,TEST_STR_SYMBOL,", ");
    _t_new(c,PARAM_REF,pt5,sizeof(int)*5);
    _t_new_str(c,TEST_STR_SYMBOL," ");
    _t_new(c,PARAM_REF,pt6,sizeof(int)*5);

    //    _t_new(code,PARAM_REF,pt2,sizeof(int)*4);

    Process p = _d_code_process(processes,code,"profileToMailingLabel","given a profile produce a mailing label",input,output,RECEPTOR_CONTEXT);

    T *act = _t_newp(0,ACTION,p);

    T *r = _p_make_run_tree(processes,act,1,t);
    wjson(&test_profile_defs,r,"profile",0);

    //    spec_is_str_equal(_t2s(&test_profile_defs,r),"");

    G_reduce_fn = "profile";
    G_reduce_count = 1;
    spec_is_equal(_p_reduce(test_profile_defs,r),noReductionErr);
    G_reduce_count = 0;

    spec_is_str_equal(_t2s(&test_profile_defs,_t_child(r,1)),"(MAILING_LABEL:Jane Smith\\n126 Main Street\\nSmallville, CA 12345)");

    _t_free(r);
    _t_free(t);
    _t_free(processes);
    _cleanupProfileDefs();
}

#endif
