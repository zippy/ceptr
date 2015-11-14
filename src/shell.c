/**
 * @ingroup receptor
 *
 * @{
 * @file shell.c
 * @brief implementation of a command line shell receptor
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "shell.h"
#include "semtrex.h"


void addCommand(Receptor *r,Xaddr ox,char *command,char *desc,T *code) {
    T *expect = _t_new_root(PATTERN);
    T *s = _t_news(expect,SEMTREX_GROUP,SHELL_COMMAND);

    T *cm = _sl(s,SHELL_COMMAND);
    T *vl =  _t_newr(cm,SEMTREX_VALUE_LITERAL);
    T *vls = _t_newr(vl,SEMTREX_VALUE_SET);
    _t_new_str(vls,VERB,command);

    T *p = _t_new_root(SAY);
    _t_newi(p,RECEPTOR_ADDRESS,ox.addr);
    _t_newi(p,ASPECT,DEFAULT_ASPECT);
    _t_add(p,code);

    Process proc = _r_code_process(r,p,desc,"long desc...",NULL);
    T *act = _t_newp(0,ACTION,proc);

    _r_add_expectation(r,DEFAULT_ASPECT,SHELL_COMMAND,expect,act,0,0);
}

void makeShell(VMHost *v,FILE *input, FILE *output,Receptor **irp,Receptor **orp,Stream **isp,Stream **osp) {
    // create the shell receptor
    // @todo fix the naming paradox that the "shell" symbol is defined in the
    // vmhost context here, but used in the context of the receptor itself
    // see bug #31
    Symbol shell = _r_declare_symbol(G_vm->r,RECEPTOR,"shell");
    Receptor *r = _r_new(shell);
    Xaddr shellx = _v_new_receptor(G_vm,G_vm->r,shell,r);
    _v_activate(G_vm,shellx);

    // create stdin/out receptors

    Stream *output_stream = *osp = _st_new_unix_stream(output,0);
    Stream *input_stream = *isp = _st_new_unix_stream(input,1);

    Symbol std_in = _r_declare_symbol(G_vm->r,RECEPTOR,"std_in");
    Receptor *i_r = *irp = _r_makeStreamReaderReceptor(std_in,TEST_STREAM_SYMBOL,input_stream,shellx.addr);
    Xaddr ix = _v_new_receptor(G_vm,G_vm->r,std_in,i_r);
    _v_activate(G_vm,ix);

    Symbol std_out = _r_declare_symbol(G_vm->r,RECEPTOR,"std_out");
    Receptor *o_r = *orp = _r_makeStreamWriterReceptor(std_out,TEST_STREAM_SYMBOL,output_stream);
    Xaddr ox = _v_new_receptor(G_vm,G_vm->r,std_out,o_r);
    _v_activate(G_vm,ox);

    // create expectations for commands
    // (expect (on std_in LINE) action (send self (shell_command parsed from LINE))
    T *expect = _t_new_root(PATTERN);
    T *s = _t_news(expect,SEMTREX_GROUP,VERB);
    _sl(s,LINE);
    T *p = _t_new_root(SAY);
    ReceptorAddress to =  __r_get_self_address(r);

    _t_newi(p,RECEPTOR_ADDRESS,to);
    _t_newi(p,ASPECT,DEFAULT_ASPECT);
    T *x = _t_newr(p,SHELL_COMMAND);
    int pt1[] = {2,1,TREE_PATH_TERMINATOR};
    _t_new(x,PARAM_REF,pt1,sizeof(int)*4);

    Process proc = _r_code_process(r,p,"send self command","long desc...",NULL);
    T *act = _t_newp(0,ACTION,proc);
    T* params = _t_new_root(PARAMS);
    _t_news(params,INTERPOLATE_SYMBOL,VERB);
    _r_add_expectation(r,DEFAULT_ASPECT,LINE,expect,act,params,0);

    // (expect (on flux SHELL_COMMAND:time) action(send std_out (convert_to_lines (send clock get_time))))

    T *code = _t_new_root(REQUEST);
    _t_newi(code,RECEPTOR_ADDRESS,2); // @todo bogus!!! fix clock address
    _t_newi(code,ASPECT,DEFAULT_ASPECT);
    _t_newr(code,CLOCK_TELL_TIME);
    _t_news(code,RESPONSE_CARRIER,TICK);

    addCommand(r,ox,"time","get time",code);

    // (expect (on flux SHELL_COMMAND:receptor) action (send std_out (convert_to_lines (send vmhost receptor-list))))

    code = _t_newi(0,MAGIC,MagicReceptors);
    addCommand(r,ox,"receptors","get receptor list",code);

    // (expect (on flux SHELL_COMMAND:receptor) action (send std_out (convert_to_lines (send vmhost shutdown)))
    code = _t_newi(0,MAGIC,MagicQuit);
    addCommand(r,ox,"quit","shut down the vmhost",code);

}

/** @}*/
