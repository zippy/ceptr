#include "../src/label.h"
#include "../src/ceptr.h"


void testLabel() {
    int p[] = {1,3,4,TREE_PATH_TERMINATOR};
    int *pp;
    spec_is_equal(str2label("fish"),1529677330);
    labelSet("fish",p);
    pp = labelGet("fish");
    spec_is_path_equal(p,pp);
}
