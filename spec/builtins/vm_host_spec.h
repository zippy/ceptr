#include "../../src/ceptr.h"


void testSendMessageFromEchoToStdoutLog() {
    VMReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);

    Receptor echoReceptor, *echo_r = &echoReceptor;

    init(echo_r);
    echo_r->parent = &vm->base;

    Xaddr expected_xaddr;

//    Xaddr packetSpecXaddr = initPacket(echo_r);

    Packet p;
    p.destination = STDOUT;
    p.value = echo_r->intPatternSpecXaddr;

    send_message(echo_r, &p);

//    data_set(echo_r, echo_r->membraneXaddr, &p, 0);

    spec_is_equal( vm->stdout.data.lastLogEntry.noun, echo_r->intPatternSpecXaddr.noun);
    printf("\nshould have printed 'Int Spec'\n");
}

void testGetLogProcFromStdout() {
    VMReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);
    LogProc lp = getLogProc(&vm->stdout);
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