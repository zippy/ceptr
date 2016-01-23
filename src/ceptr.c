/**
 * @file ceptr.c
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 * @brief implements main() entry point for the ceptr node
 */

#include "ceptr.h"
#include "accumulator.h"
#include <stdio.h>

int main(int argc, const char **argv) {

    def_sys();
    char *dname = "tmp/shell_vm";

    //    _a_boot(dname);

    // set up the vmhost
    G_vm = _v_new();
    _v_instantiate_builtins(G_vm);

    Stream *output_stream, *input_stream;
    Receptor *i_r,*o_r;

    makeShell(G_vm,stdin,stdout,&i_r,&o_r,&input_stream,&output_stream);
    output_stream->flags &= ~StreamCloseOnFree; // don't close the stdout on free...
    input_stream->flags &= ~StreamCloseOnFree; // don't close the stdin on free...

    //debug_enable(D_SIGNALS);
    _v_start_vmhost(G_vm);

    while (G_vm->r->state == Alive) {
        sleepms(100);
    };

    _v_join_thread(&G_vm->clock_thread);
    _v_join_thread(&G_vm->vm_thread);


}
