#include "../src/ceptr.h"
#include <stdio.h>
#include <signal.h>
#include <assert.h>

#include "test_framework.h"

#include "tree_spec.h"
#include "def_spec.h"
#include "init_spec.h"
#include "stack_spec.h"
#include "scape_spec.h"

#include "builtins/noun_spec.h"
#include "builtins/pattern_spec.h"
#include "builtins/int_spec.h"
#include "builtins/str255_spec.h"
#include "builtins/cfunc_spec.h"
#include "builtins/array_spec.h"
#include "builtins/command_spec.h"
#include "builtins/stream_spec.h"
#include "builtins/receptor_spec.h"
#include "conversation_spec.h"
#include "receptor_util_spec.h"

#include "vm_spec.h"
#include "builtins/vm_host_spec.h"

#include "examples/point.h"
#include "examples/line.h"
#include "examples/stars.h"


#define NUM_THREADS     5

void *TaskCode(void *argument)
{
    int tid;

    tid = *((int *) argument);
    printf("Hello World! It's me, thread %d!\n", tid);

    /* optionally: insert more useful stuff here */

    return NULL;
}

int testThreads(void)
{
    pthread_t threads[NUM_THREADS];
    int thread_args[NUM_THREADS];
    int rc, i;

    /* create all threads one by one */
    for (i=0; i<NUM_THREADS; ++i) {
        thread_args[i] = i;
        printf("In main: creating thread %d\n", i);
        rc = pthread_create(&threads[i], NULL, TaskCode, (void *) &thread_args[i]);
        assert(0 == rc);
    }

    /* wait for all threads to complete */
    for (i=0; i<NUM_THREADS; ++i) {
        rc = pthread_join(threads[i], NULL);
        assert(0 == rc);
    }

    return 0;
}

int main(int argc, const char **argv) {
    printf("Running all tests...\n\n");

    //**** core tests
    testTree();
    testDef();
    testStack();
    testInit();
    testCommand();
    testConversation();
    testScape();

    //    testVM();
    //    testThreads();

    //**** builtins tests
    testNoun();
    testPattern();
    //testArray();
    //    testStream();
    testInt();
    testStr255();
    testCfunc();
    testReceptorUtil();
    //    testReceptor();
    //testVmHost();

    //**** examples test
    testPoint();
    //    testLine();


    report_tests();
    return 0;
}
