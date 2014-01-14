#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "init_spec.h"
#include "stack_spec.h"

#include "builtins/noun_spec.h"
#include "builtins/int_spec.h"
#include "builtins/array_spec.h"

#include "examples/point.h"
#include "examples/line.h"
#include "examples/stars.h"

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    testStack();
    testGetSymbol();
    testInit();

    testInt();
    testPoint();
//    testLine();
//    testArray();

    report_tests();
    return 0;
}
