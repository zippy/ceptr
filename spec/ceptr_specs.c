#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "tree_spec.h"
#include "label_spec.h"
#include "semtrex_spec.h"
// #include "receptor_spec.h"

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    //**** core tests
    testTree();
    testLabel();
    testSemtrex();
//     testReceptor();

    report_tests();
    return 0;
}
