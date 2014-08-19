#include "../src/label.h"
#include "../src/ceptr.h"


void testLabel() {
    int p[] = {1,3,4,TREE_PATH_TERMINATOR};
    int *pp;
    LabelTable table = NULL;

    spec_is_equal(str2label("fish"),1529677330);
    labelSet(&table,"fish",p);
    pp = labelGet(&table,"fish");
    spec_is_path_equal(p,pp);

    lableTableFree(&table);
}
