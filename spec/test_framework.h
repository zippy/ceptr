#include "../src/ceptr.h"

#define MAX_FAILURES 1000
int spec_failures = 0;
int spec_total = 0;
char failures[MAX_FAILURES][255];

#define spec_is_true(x) spec_total++;if (x){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be true",__FUNCTION__,__LINE__,#x);}
#define spec_is_equal(got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d but was %d",__FUNCTION__,__LINE__,#got,expected,got);}

typedef size_t (*intSizeFunction)(Receptor *, Symbol, void *);

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
