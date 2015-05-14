/**
 * @file label_spec.h
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 */

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
