#include "../../src/ceptr.h"

int proc_int_packet_print(Receptor *r, void *this) {
    printf("destination: %d, noun: %d, payload: %d", *(int *) this, *(((int *) this) + 1), *(((int *) this) + 2));
    return 0;
}

Xaddr initIntPacket(Receptor *r) {
    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
    stack_push(r, CSTRING_NOUN, &"DESTINATION");
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr dest_xaddr;
    stack_pop(r, XADDR_NOUN, &dest_xaddr);

    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
    stack_push(r, CSTRING_NOUN, &"PAYLOAD");
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr payload_xaddr;
    stack_pop(r, XADDR_NOUN, &payload_xaddr);

    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
    stack_push(r, CSTRING_NOUN, &"PACKET_NOUN");
    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
    Xaddr packet_noun_xaddr;
    stack_pop(r, XADDR_NOUN, &packet_noun_xaddr);


    //    Symbol Y = preop_new_noun(r, r->intPatternSpecXaddr, "Y");

    Process processes[] = {
        {PRINT, (processFn) proc_int_packet_print}
    };
    Xaddr children[3] = {dest_xaddr, packet_noun_xaddr, payload_xaddr};
    return preop_new_pattern(r, "INT_PACKET", 3, children, 1, processes);
}


void testVmHost(){
    VMReceptor vmHostReceptor, *vm = &vmHostReceptor;
    vm_host_init(vm);



    Receptor echoReceptor, *echo_r = &echoReceptor;

    init(echo_r);
    echo_r->parent = &vm->base;

    Xaddr expected_xaddr;

    Xaddr intPacketSpecXaddr = initIntPacket(echo_r);

    Symbol echo_int = preop_new_noun(echo_r, intPacketSpecXaddr, "ECHO INT");
    int value[3] = {STDOUT, echo_int, 3};

    data_set(echo_r, echo_r->membraneXaddr, &value,
        size_of_named_surface(echo_r, echo_int, &value));

    // next step:  echo should write to membraneXaddr to trigger this->
//    send_message(vm, STDOUT, &valueSent, 4);























    // NEXT STEP!  when we write to stdout log, have data or vm or something wake up
    // stdout so it can actually print a char unix stdout.

    spec_is_equal( *(int *)vm->stdout.data.lastLogEntry.content, value[2]);

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
