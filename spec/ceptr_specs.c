/**
 * @file ceptr_specs.c
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 * @brief implements a main() entry point for running all the tests
 */

#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "tree_spec.h"
#include "mtree_spec.h"
#include "stream_spec.h"
#include "def_spec.h"
#include "label_spec.h"
#include "semtrex_spec.h"
#include "receptor_spec.h"
#include "process_spec.h"
#include "scape_spec.h"
#include "vmhost_spec.h"
#include "accumulator_spec.h"

#include "profile_example.h"

#include <setjmp.h>

jmp_buf G_err;

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    int err;
    if ((err = setjmp(G_err))) {
    printf("ERROR: %d\n",err);
    }
    else {
    def_sys();
    //**** core tests
    testDef();
    testTree();
    testMTree();
    testStream();
    testLabel();
    testSemtrex();
    testReceptor();
    testProcess();
    testScape();
    testVMHost();
    testAccumulator();

    //***** examples
    testProfileExample();
    testHTTPExample();

    sys_free();
    report_tests();
    }
    pthread_exit(NULL);
    //    return 0;
}
