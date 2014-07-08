#include "../src/label.h"
#include "../src/ceptr.h"


void testLabel() {
    spec_is_equal(str2label("fish"),1529677330);
    int p[] = {1,3,4,TREE_PATH_TERMINATOR};
    labelSet("fish",p);
    int *pp = labelGet("fish");
    spec_is_path_equal(p,pp);
}
