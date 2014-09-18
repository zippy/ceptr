/**
 * @file ceptr_specs.c
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 * @brief implements a main() entry point for running all the tests
 */

#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "tree_spec.h"
#include "def_spec.h"
#include "label_spec.h"
#include "semtrex_spec.h"
#include "receptor_spec.h"
#include "process_spec.h"
#include "scape_spec.h"
#include "vmhost_spec.h"

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    //**** core tests
    testTree();
    testDef();
    testLabel();
    testSemtrex();
    testReceptor();
    testProcess();
    testScape();
    testVMHost();

    report_tests();
    return 0;
}
