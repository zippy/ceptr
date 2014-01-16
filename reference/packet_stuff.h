//
//int proc_int_packet_print(Receptor *r, void *this) {
//    printf("destination: %d, noun: %d, payload: %d", *(int *) this, *(((int *) this) + 1), *(((int *) this) + 2));
//    return 0;
//}
//
//Xaddr initPacket(Receptor *r) {
//    stack_push(r, XADDR_NOUN, &r->intPatternSpecXaddr);
//    stack_push(r, CSTRING_NOUN, &"DESTINATION");
//    op_invoke(r, r->nounSpecXaddr, INSTANCE_NEW);
//    Xaddr dest_xaddr;
//    stack_pop(r, XADDR_NOUN, &dest_xaddr);
//
//    Process processes[] = {
//        {PRINT, (processFn) proc_int_packet_print}
//    };
//    Xaddr children[2] = {dest_xaddr, r->xaddrXaddr};
//    return preop_new_pattern(r, "PACKET", 2, children, 1, processes);
//}
