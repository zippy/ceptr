#include "../../src/ceptr.h"



void testVmHost(){
    VMReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);



    Receptor echoReceptor, *echo_r = &echoReceptor;

    init(echo_r);


    Xaddr expected_xaddr;
    int valueSent = 3;


    // next step:  echo should write to membraneXaddr to trigger this->
    send_message(vm, STDOUT, &valueSent, 4);

























    spec_is_equal( *(int *)vm->stdout.data.lastLogEntry.content, valueSent);

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
}
