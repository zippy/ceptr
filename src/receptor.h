/**
 * @defgroup receptor Receptors
 * @brief Receptors provide the fundamental coherence contexts for ceptr
 *
 * @{
 * @file receptor.h
 * @brief receptor implementation header file
 *
 * @copyright Copyright (C) 2013-2016, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "semtable.h"
#include "tree.h"
#include "def.h"
#include "label.h"
#include "process.h"
#include "scape.h"

// delivery errors
enum {noDeliveryErr};

/******************  create and destroy receptors */
T *__r_make_definitions();
T *_r_make_state();
Receptor *_r_new(SemTable *sem,SemanticID r);
Receptor *_r_new_receptor_from_package(SemTable *sem,Symbol s,T *p,T *bindings);
T *__r_build_expectation(Symbol carrier,T *pattern,T *action,T *with,T *until,T *using);
void _r_add_expectation(Receptor *r,Aspect aspect,Symbol carrier,T *pattern,T *action,T *with,T *until, T *using);
void __r_add_expectation(Receptor *r,Aspect aspect,T *e);
void _r_remove_expectation(Receptor *r,T *expectation);
void _r_free(Receptor *r);

/*****************  receptor symbols, structures, and processes */

Symbol _r_define_symbol(Receptor *r,Structure s,char *label);
SemanticID _r_get_sem_by_label(Receptor *r,char *label);
Structure _r_define_structure(Receptor *r,char *label,int num_params,...);
Structure __r_define_structure(Receptor *r,char *label,T *structure_def);
Structure __r_get_symbol_structure(Receptor *r,Symbol s);
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface);
Process _r_define_process(Receptor *r,T *code,char *name,char *intention,T *signature,T *link);
Protocol _r_define_protocol(Receptor *r,T *p);

T *_r_build_def_semtrex(Receptor *r,Symbol s);
int _r_def_match(Receptor *r,Symbol s,T *t);

/*****************  receptor instances and xaddrs */

Xaddr _r_new_instance(Receptor *r,T *t);
T *_r_get_instance(Receptor *r,Xaddr x);
T * _r_set_instance(Receptor *r,Xaddr x,T *t);
T * _r_delete_instance(Receptor *r,Xaddr x);
TreeHash _r_hash(Receptor *r,Xaddr t);

/******************  receptor serialization */
void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP);
Receptor * _r_unserialize(SemTable *sem,void *surface);

/******************  receptor signaling */
#define __r_make_addr(p,t,a) ___r_make_addr(p,t,a,0)
T *___r_make_addr(T *parent,Symbol type,ReceptorAddress addr,bool is_run_node);
ReceptorAddress __r_get_addr(T *addr);

T * __r_make_signal(ReceptorAddress from,ReceptorAddress to,Aspect aspect,Symbol carrier,T *signal_contents,UUIDt *in_response_to,T* until,T *conversation);
T *__r_build_wakeup_info(T *code_point,int process_id);
T* __r_send(Receptor *r,T *signal);
T* _r_send(Receptor *r,T *signal);
T* _r_request(Receptor *r,T *signal,Symbol response_carrier,T *code_point,int process_id);
void evaluateEndCondition(T *ec,bool *cleanup,bool *allow);
void __r_test_expectation(Receptor *r,T *expectation,T *signal);
Error _r_deliver(Receptor *r, T *signal);

/******************  internal utilities */
T *__r_get_aspect(Receptor *r,Aspect aspect);
T *__r_get_expectations(Receptor *r,Aspect aspect);
T *__r_get_signals(Receptor *r,Aspect aspect);
Receptor * __r_get_receptor(T *installed_receptor);

/*****************  Tree debugging utilities */
char *__td(Receptor *r,T *t,char *buf);
char *_td(Receptor *r,T *t);
char *_r_get_structure_name(Receptor *r,Structure s);
char *_r_get_symbol_name(Receptor *r,Symbol s);
char *_r_get_process_name(Receptor *r,Process p);

Xaddr G_null_xaddr;
#define is_null_symbol(s) ((s).semtype == 0 && (s).context == 0 && (s).id == 0)
#define is_null_xaddr(x) (is_null_symbol(x.symbol) && (x).addr == 0)
#define is_xaddr_eq(x,y) (semeq(x.symbol,y.symbol) && (x).addr == (y).addr)

#define spec_is_symbol_equal(r,got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_symbol_name(r,expected),_r_get_symbol_name(r,got));}

#define spec_is_structure_equal(r,got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_structure_name(r,expected),_r_get_structure_name(r,got));}

#define spec_is_process_equal(r,got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_process_name(r,expected),_r_get_process_name(r,got));}

#define spec_is_xaddr_equal(sem,got,expected)  spec_total++; if (is_xaddr_eq(got,expected)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s.%d but was %s.%d",__FUNCTION__,__LINE__,#got,!is_null_symbol(expected.symbol)?_sem_get_name(sem,expected.symbol):"0",expected.addr,!is_null_symbol(got.symbol) ? _sem_get_name(sem,got.symbol):"0",got.addr);}

/*****************  Built-in core and edge receptors */
Receptor *_r_makeStreamEdgeReceptor(SemTable *sem);
SocketListener *_r_addListener(Receptor *r,int port,ReceptorAddress to,Aspect aspect,Symbol carrier,Symbol result_symbol);
void _r_addReader(Receptor *r,Stream *st,ReceptorAddress to,Aspect aspect,Symbol carrier,Symbol result_symbol);
void _r_addWriter(Receptor *r,Stream *st,Aspect aspect);
void _r_defineClockReceptor(SemTable *sem);
Receptor *_r_makeClockReceptor(SemTable *sem);
void *___clock_thread(void *arg);
#define __r_make_tick() __r_make_timestamp(TICK,00)
T *__r_make_timestamp(Symbol s,int delta);
void __r_kill(Receptor *r);
ReceptorAddress __r_get_self_address(Receptor *r);

void __r_dump_instances(Receptor *r);

#endif
/** @}*/
