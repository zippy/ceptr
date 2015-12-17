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
#include "spec_utils.h"

Receptor *test_profile_receptor;

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
    Receptor *r = test_profile_receptor = _r_new(G_sem,TEST_RECEPTOR);

    SY(r,FIRST_NAME,CSTRING);
    SY(r,LAST_NAME,CSTRING);
    ST(r,NAME,2,
       FIRST_NAME,
       LAST_NAME
       );
    SY(r,PROFILE_NAME,NAME);

    SY(r,STREET_ADDRESS,CSTRING);
    SY(r,CITY,CSTRING);
    SY(r,STATE,CSTRING);
    SY(r,ZIP,CSTRING);
    SY(r,COUNTRY,CSTRING);

    ST(r,ADDRESS,5,
       STREET_ADDRESS,
       CITY,
       STATE,
       ZIP,
       COUNTRY
       );
    SY(r,PROFILE_ADDRESS,ADDRESS);

    SY(r,PROFILE_EMAIL,CSTRING);
    ST(r,PROFILE,3,
       PROFILE_NAME,
       PROFILE_ADDRESS,
       PROFILE_EMAIL);
    SY(r,USER_PROFILE,PROFILE);

    SY(r,MAILING_LABEL,CSTRING);
}

void _cleanupProfileDefs() {
    _r_free(test_profile_receptor);
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

    SemTable *sem = test_profile_receptor->sem;
    wjson(sem,t,"user_profile",-1);

    spec_is_str_equal(t2s(t),"(USER_PROFILE (PROFILE_NAME (FIRST_NAME:Jane) (LAST_NAME:Smith)) (PROFILE_ADDRESS (STREET_ADDRESS:126 Main Street) (CITY:Smallville) (STATE:CA) (ZIP:12345) (COUNTRY:USA)) (PROFILE_EMAIL:test@example.com))");

    T *signature = __p_make_signature("result",SIGNATURE_SYMBOL,NULL_SYMBOL,
                                      "mailing_profile",SIGNATURE_STRUCTURE,CSTRING, // should actually be MAILING_LABEL symbol or something like that
                                      "the_int?",SIGNATURE_STRUCTURE,INTEGER,
                                      "the_profile",SIGNATURE_STRUCTURE,PROFILE,
                                      NULL);

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

    Process p = _d_define_process(sem,code,"profileToMailingLabel","given a profile produce a mailing label",signature,test_profile_receptor->context);

    T *params = _t_new_root(PARAMS);
    _t_add(params,t);

    T *r = _p_make_run_tree(G_sem,p,params,NULL);
    wjson(sem,r,"profile",0);

    //    spec_is_str_equal(_t2s(&test_profile_defs,r),"");

    startVisdump("profile");
    spec_is_equal(_p_reduce(sem,r),noReductionErr);
    int pt[] = {TREE_PATH_TERMINATOR};

    _visdump(sem,_t_child(r,1),pt);
    endVisdump();

    spec_is_str_equal(t2s(_t_child(r,1)),"(MAILING_LABEL:Jane Smith\\n126 Main Street\\nSmallville, CA 12345)");

    _t_free(r);
    _cleanupProfileDefs();
}

#endif
