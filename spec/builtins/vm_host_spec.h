#include "../../src/ceptr.h"

void echo_log_proc(Receptor *r) {
    raise_error0("nuh uh\n");
}

void testSendMessageFromEchoToStdoutLog() {
    HostReceptor vmHostReceptor;
    HostReceptor *vm = &vmHostReceptor;
    vm_host_init(vm);

    Receptor *echo_r = vmh_receptor_new(vm, echo_log_proc);

    Packet p;
    p.destination = STDOUT;
    p.value = echo_r->intPatternSpecXaddr;

    send_message(echo_r, &p);

    spec_is_equal( vm->receptors[STDOUT].data.lastLogEntry.noun, echo_r->intPatternSpecXaddr.noun);
    printf("\nshould have printed 'Int Spec'\n");
}

void testGetLogProcFromStdout() {
    HostReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);
    LogProc lp = getLogProc(&vm->receptors[STDOUT]);
    spec_is_long_equal((long)lp, (long)stdout_log_proc);
}

void testVmHost(){
    testSendMessageFromEchoToStdoutLog();
    testGetLogProcFromStdout();
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