#include "../../src/ceptr.h"

void testSendMessageFromEchoToStdoutLog() {
    HostReceptor vmHostReceptor;
    HostReceptor *vm = &vmHostReceptor;
    vm_host_init(vm);

    Receptor *echo_r = vmh_receptor_new(vm, null_proc);

    Packet p;
    p.destination = STDOUT;
    p.value = echo_r->intPatternSpecXaddr;

    send_message(echo_r, &p);

    LogEntry *le = &vm->receptors[STDOUT].data.log[vm->receptors[STDOUT].data.log_tail];

    spec_is_equal( le->noun, echo_r->intPatternSpecXaddr.noun);
    printf("\nshould have printed 'Int Spec'\n");
}

void testPlantListenerOnStdinSendsMessagestoEcho() {
    HostReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);

    Receptor *echo_r = vmh_receptor_new(vm, null_proc);
    listen(echo_r, STDIN);
    int val = 33;
    write_out(vm, &vm->receptors[STDIN], echo_r->charIntNoun, &val, 4);

    LogEntry *le = &echo_r->data.log[echo_r->data.log_tail];
    spec_is_equal( le->noun, echo_r->charIntNoun);
    spec_is_equal( *(int *)le->content, val);
}


void testGetLogProcFromStdout() {
    HostReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);
    LogProc lp = getLogProc(&vm->receptors[STDOUT]);
    spec_is_long_equal((long)lp, (long)stdout_log_proc);
}

void echo_log_proc(Receptor *r, LogEntry *le) {
    size_t size = size_of_named_surface(r, le->noun, le->content);
    _send_message((HostReceptor *)r->parent, STDOUT, le->noun, le->content, size);
}

void testEcho(){
    HostReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);
    Receptor *echo_r = vmh_receptor_new(vm, echo_log_proc);
    listen(echo_r, STDIN);
    vm_host_run(vm);
    // spec:  should do the echo loop
}

void testVmHost(){
//    testSendMessageFromEchoToStdoutLog();
//    testGetLogProcFromStdout();
//    testPlantListenerOnStdinSendsMessagestoEcho();
    testEcho();
}




//    data_set


//    invoke on external address
//    0. child invokes WRITE on external addr
//    1. parent receptor takes element from stack & writes to xaddr for log of receiving receptor
//    2. data engine pokes receiving receptor to notify it of new log entry
//



// spec_is_equal( data_read_from_log( receptor ), nullXaddr )
// data_write_to_log( receptor, valueXaddr )
// spec_is_equal( data_read_from_log( receptor ), valueXaddr )



// result = query scape for list of receptor xaddrs in VmHost
// assert_equal  result,  ["STDIN", "STDOUT", "ECHO"]

// run.  talk to stdin/out
// stop on 'q'