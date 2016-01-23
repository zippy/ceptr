/**
 * @file test_framework.h
 * @brief implements the testing harness
 *
 * Our testing harness consists of macros each of which test, for a given data-type, whether
 * an expectation matches a result.
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).

 * @ingroup tests
 */

#include "../src/ceptr.h"

#define MAX_FAILURES 1000
int spec_failures = 0;
int spec_total = 0;
char failures[MAX_FAILURES][5000];
char buf1[2000];
char buf2[2000];

#define spec_is_true(x) spec_total++;if (x){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be true",__FUNCTION__,__LINE__,#x);}
#define spec_is_false(x) spec_total++;if (!(x)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be false",__FUNCTION__,__LINE__,#x);}
#define spec_is_equal(got, expected) spec_total++; {int __got=got;int __ex=expected;if (__ex==__got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d but was %d",__FUNCTION__,__LINE__,#got,__ex,__got);}}
#define spec_is_str_equal(got, expected) spec_total++; {char *__got=got;char *__ex=expected; if (strcmp(__got,__ex)==0){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be:\n \"%s\"\nbut_was:\n \"%s\"",__FUNCTION__,__LINE__,#got,__ex,__got);}}
#define spec_is_long_equal(got, expected) spec_total++; {long __got=got;long __ex=(long)expected;if (__ex==__got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %ld but was %ld",__FUNCTION__,__LINE__,#got,__ex,__got);}}
#define spec_is_ptr_equal(got, expected) spec_total++; {void *__got=got;void *__ex=expected;if (__ex==__got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %p but was %p",__FUNCTION__,__LINE__,#got,__ex,__got);}}
#define spec_is_path_equal(got, expected) spec_total++; {int *__got=got;int *__ex=expected; if (__got && _t_path_equal(__got,__ex)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_t_sprint_path(__ex,buf1),__got ? _t_sprint_path(__got,buf2) : "nil");}}
#define spec_is_float_equal(got, expected) spec_total++; {float=__got;float __ex=expected; if (__ex==__got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %f but was %f",__FUNCTION__,__LINE__,#got,__ex,__got);}}
#define spec_is_buffer_equal(got, expected, length) spec_total++; if (memcmp(got,expected,length)==0){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %ld bytes from %s to match %s by they didn't",__FUNCTION__,__LINE__,(long)length,#got,#expected);}

void report_tests() {
    int i;
    if (spec_failures > 0) {
        printf("\n%d out of %d specs failed:\n", spec_failures,spec_total);
        for (i = 0; i < spec_failures; i++) {
            printf("%s\n", failures[i]);
        }
    }
    else {
        printf("\nAll %d specs pass\n", spec_total);
    }
}
