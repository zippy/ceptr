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
Symbol ZIP;
Symbol COUNTRY;
Structure ADDRESS;
Symbol PROFILE_ADDRESS;

Symbol PROFILE_EMAIL;
Structure PROFILE;
Symbol USER_PROFILE;

void _setupProfileDefs() {
    test_profile_defs.symbols = test_profile_symbols = _t_new_root(SYMBOLS);
    test_profile_defs.structures = test_profile_structures = _t_new_root(STRUCTURES);
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
    SY(d,ZIP,CSTRING);
    SY(d,COUNTRY,CSTRING);

    ST(d,ADDRESS,4,
       STREET_ADDRESS,
       CITY,
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

}

void _cleanupProfileDefs() {
    _t_free(test_profile_symbols);
    _t_free(test_profile_structures);
}

#define _t_new_str(t,s,str) _t_new(t,s,str,strlen(str)+1)

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
    _t_new_str(a,COUNTRY,"USA");
    _t_new_str(a,ZIP,"12345");

    spec_is_str_equal(_t2s(&test_profile_defs,t),"(USER_PROFILE (PROFILE_NAME (FIRST_NAME:Jane) (LAST_NAME:Smith)) (PROFILE_ADDRESS (STREET_ADDRESS:126 Main Street) (CITY:Smallville) (COUNTRY:USA) (ZIP:12345)) (PROFILE_EMAIL:test@example.com))");

    _t_free(t);
    _cleanupProfileDefs();
}

#endif
