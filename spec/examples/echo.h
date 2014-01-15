#include "../../src/ceptr.h"


/*
Code init,log;
void push_instruction(Code *c, OpCode op, InstructionParams params);

push_instruction(&init,LISTEN,build_params(external_xaddr(STDIN)));
push_instruction(&log,INVOKE,build_params(READ,r->logXaddr));
push_instruction(&log,INVOKE,build_params(WRITE,external_xaddr(STDOUT)));
new_receptor(vr,&init,&log);



invoke on external address
0. child invokes WRITE on external addr
1. parent receptor takes element from stack & writes to xaddr for log of receiving receptor
2. data engine pokes receiving receptor to notify it of new log entry



*/



// add "ECHO" receptor
/*

  CODE_TREE -> (grouped intructions)
  INIT_CODE -> CODE_TREE
  ON_LOG_CHANGE_CODE -> CODE_TREE



  (op_invoke new_noun "noun name")
  (new_noun "noun name")

  ((env  (INIT_CODE) (ON_LOG_CHANGE_CODE) )
   (bodyf
     ((plant_listener stdin_xaddr))
     ((push_from_stream log_xaddr) (pop_to_stream stdout_xaddr))
 ))

 */
