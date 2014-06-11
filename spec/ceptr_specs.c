#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "tree_spec.h"

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    //**** core tests
    testTree();

    report_tests();
    return 0;
}
