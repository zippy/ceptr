#include "../src/ceptr.h"

#define MAX_FAILURES 1000
int spec_failures = 0;
int spec_total = 0;
char failures[MAX_FAILURES][512];
char buf1[255];
char buf2[255];

#define spec_is_true(x) spec_total++;if (x){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be true",__FUNCTION__,__LINE__,#x);}
#define spec_is_false(x) spec_total++;if (!(x)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be false",__FUNCTION__,__LINE__,#x);}
#define spec_is_equal(got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d but was %d",__FUNCTION__,__LINE__,#got,expected,got);}
#define spec_is_str_equal(got, expected) spec_total++; if (strcmp(got,expected)==0){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be \"%s\" but was \"%s\"",__FUNCTION__,__LINE__,#got,expected,got);}
#define spec_is_long_equal(got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %ld but was %ld",__FUNCTION__,__LINE__,#got,expected,got);}
#define spec_is_ptr_equal(got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %p but was %p",__FUNCTION__,__LINE__,#got,expected,got);}
#define spec_is_path_equal(got, expected) spec_total++; if (_t_path_equal(got,expected)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_t_sprint_path(expected,buf1),_t_sprint_path(got,buf2));}
#define spec_is_float_equal(got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %f but was %f",__FUNCTION__,__LINE__,#got,expected,got);}

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
