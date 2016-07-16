/**
 * @file ceptr_specs.c
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 * @ingroup tests
 * @brief implements a main() entry point for running all the tests
 */

#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "semtable_spec.h"
#include "def_spec.h"
#include "tree_spec.h"
#include "mtree_spec.h"
#include "stream_spec.h"
#include "label_spec.h"
#include "semtrex_spec.h"
#include "receptor_spec.h"
#include "process_spec.h"
#include "scape_spec.h"
#include "vmhost_spec.h"
#include "accumulator_spec.h"

#include "profile_example.h"
#include "group_spec.h"
#include "protocol_spec.h"
#include "http_example.h"

#include <setjmp.h>

Symbol A,B,C,D,E,F,Root;

jmp_buf G_err;

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    int err;
    if ((err = setjmp(G_err))) {
        printf("ERROR: %d\n",err);
    }
    else {
    G_sem = def_sys();
    //debug_enable(D_BOOT);
    load_contexts(G_sem);
    //debug_disable(D_BOOT);

    // define some generic symbols for doing specs
    sYt(A,NULL_STRUCTURE);
    sYt(B,NULL_STRUCTURE);
    sYt(C,NULL_STRUCTURE);
    sYt(D,NULL_STRUCTURE);
    sYt(E,NULL_STRUCTURE);
    sYt(F,NULL_STRUCTURE);
    sYt(Root,NULL_STRUCTURE);

    //**** core tests
    testSemTable();
    testDef();
    testTree();
    testMTree();
    testStream();
    testLabel();
    testSemtrex();
    testProcess();
    testReceptor();
    testScape();
    testVMHost();
    testAccumulator();
    testProtocol();

    /* /\**** receptor tests *\/ */
    /* /\* /\\* testGroup(); *\\/ *\/ */

    testHTTP();

    /****** examples */
    testProfileExample();

    sys_free(G_sem);
    report_tests();
    }
    pthread_exit(NULL);
    //    return 0;
}
