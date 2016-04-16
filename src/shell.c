/**
 * @ingroup receptor
 *
 * @{
 * @file shell.c
 * @brief implementation of a command line shell receptor
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "shell.h"
#include "semtrex.h"
#include "protocol.h"


void addCommand(Receptor *r,ReceptorAddress ox,char *command,char *desc,T *code,T *bindings_handler) {
    T *expect = _t_new_root(PATTERN);
    T *s = _t_news(expect,SEMTREX_GROUP,SHELL_COMMAND);

    T *cm = _sl(s,SHELL_COMMAND);
    T *vl =  _t_newr(cm,SEMTREX_VALUE_LITERAL);
    T *vls = _t_newr(vl,SEMTREX_VALUE_SET);
    _t_new_str(vls,VERB,command);

    T *p = _t_new_root(SAY);
    __r_make_addr(p,TO_ADDRESS,ox);

    _t_news(p,ASPECT_IDENT,DEFAULT_ASPECT);
    _t_news(p,CARRIER,NULL_SYMBOL);

    // if code is actually an INITIATE then we will have a bindings handler
    // to which we want to add the SAY command as the ACTUAL_PROCESS
    // and we will replace the p with code which does the proper protocol
    // initiation.  Kinda weird, I know...
    if (bindings_handler) {
        char proc_name[255] = "handle ";
        strcpy(&proc_name[7],command);
        int pt1[] = {2,1,TREE_PATH_TERMINATOR};
        _t_new(p,PARAM_REF,pt1,sizeof(int)*3);
        Process proc = _r_define_process(r,p,proc_name,"long desc...",NULL,NULL);
        _t_news(bindings_handler,ACTUAL_PROCESS,proc);
        p = code;
    }
    else {
        _t_add(p,code);
    }

    Process proc = _r_define_process(r,p,desc,"long desc...",NULL,NULL);
    T *act = _t_newp(0,ACTION,proc);

    _r_add_expectation(r,DEFAULT_ASPECT,SHELL_COMMAND,expect,act,0,0,NULL,NULL);
}

void makeShell(VMHost *v,FILE *input, FILE *output,Receptor **irp,Receptor **orp,Stream **isp,Stream **osp) {
    // define and then create the shell receptor
    Symbol shell = _d_define_receptor(v->r->sem,"shell",__r_make_definitions(),DEV_COMPOSITORY_CONTEXT);
    Receptor *r = _r_new(v->sem,shell);
    Xaddr shellx = _v_new_receptor(v,v->r,shell,r);
    _v_activate(v,shellx);

    // create stdin/out receptors

    Stream *output_stream = *osp = _st_new_unix_stream(output,0);
    Stream *input_stream = *isp = _st_new_unix_stream(input,1);

    Receptor *i_r = *irp = _r_makeStreamEdgeReceptor(v->sem);
    _r_addReader(i_r,input_stream,r->addr,DEFAULT_ASPECT,parse_line,LINE,false);
    Xaddr ix = _v_new_receptor(v,v->r,STREAM_EDGE,i_r);
    _v_activate(v,ix);

    Receptor *o_r = *orp = _r_makeStreamEdgeReceptor(v->sem);
    _r_addWriter(o_r,output_stream,DEFAULT_ASPECT);
    Xaddr ox = _v_new_receptor(v,v->r,STREAM_EDGE,o_r);
    _v_activate(v,ox);

    // set up shell to express the line parsing protocol when it receives LINES from the stream reader
    Protocol clp;
    __sem_get_by_label(v->sem,"PARSE_COMMAND_FROM_LINE",&clp,DEV_COMPOSITORY_CONTEXT);
    T *bindings = _t_new_root(PROTOCOL_BINDINGS);
    T *res = _t_newr(bindings,RESOLUTION);
    T *w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,LINE_SENDER);
    __r_make_addr(w,ACTUAL_RECEPTOR,i_r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,COMMAND_RECEIVER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_SYMBOL);
    _t_news(w,USAGE,COMMAND_TYPE);
    _t_news(w,ACTUAL_SYMBOL,SHELL_COMMAND);

    _o_express_role(r,clp,COMMAND_RECEIVER,DEFAULT_ASPECT,bindings);
    _t_free(bindings);

    // set up shell to use the CLOCK TELL_TIME protocol for the time command
    Protocol time;
    __sem_get_by_label(v->sem,"time",&time,CLOCK_CONTEXT);
    T *code = _t_new_root(INITIATE_PROTOCOL);
    _t_news(code,PNAME,time);
    _t_news(code,WHICH_INTERACTION,tell_time);
    bindings = _t_newr(code,PROTOCOL_BINDINGS);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_HEARER);
    __r_make_addr(w,ACTUAL_RECEPTOR,r->addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_RECEPTOR);
    _t_news(w,ROLE,TIME_TELLER);
    ReceptorAddress clock_addr = {3}; // @todo bogus!!! fix getting clock address somehow
    __r_make_addr(w,ACTUAL_RECEPTOR,clock_addr);
    res = _t_newr(bindings,RESOLUTION);
    w = _t_newr(res,WHICH_PROCESS);
    _t_news(w,GOAL,REQUEST_HANDLER);

    addCommand(r,o_r->addr,"time","get time",code,w);

    // (expect (on flux SHELL_COMMAND:receptor) action (send std_out (convert_to_lines (send vmhost receptor-list))))

    code = _t_newi(0,MAGIC,MagicReceptors);
    addCommand(r,o_r->addr,"receptors","get receptor list",code,NULL);

    // (expect (on flux SHELL_COMMAND:receptor) action (send std_out (convert_to_lines (send vmhost shutdown)))
    code = _t_newi(0,MAGIC,MagicQuit);
    addCommand(r,o_r->addr,"quit","shut down the vmhost",code,NULL);

    // (expect (on flux SHELL_COMMAND:debug) action (send std_out (convert_to_lines (magic toggle debug)))
    code = _t_newi(0,MAGIC,MagicDebug);
    addCommand(r,o_r->addr,"debug","toggle debug mode",code,NULL);

}

/** @}*/
