/**
 * @defgroup receptor Receptors
 * @brief Receptors provide the fundamental coherence contexts for ceptr
 *
 * @{
 * @file receptor.h
 * @brief receptor implementation header file
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "tree.h"
#include "def.h"
#include "label.h"
#include "process.h"
#include "scape.h"


// structure position index enums to make code more readable
// @todo figure out a better way to handle this... like put defs like these
// into the symbol gen code (talk about a semantic muddle! If only I had ceptr...)
enum {EnvelopeFromIdx=1,EnvelopeToIdx,EnvelopeAspectIdx,EnvelopeCarrierIdx,EnvelopeUUIDIdx,EnvelopeExtraIdx};
enum {SignalEnvelopeIdx=1,SignalBodyIdx};
enum {PendingResponseUUIDIdx=1,PendingResponseCarrierIdx,PendingResponseWakeupIdx,PendingResponseEndCondsIdx};
// delivery errors
enum {noDeliveryErr};

/******************  create and destroy receptors */
Receptor *_r_new(Symbol s);
Receptor *_r_new_receptor_from_package(Symbol s,T *p,T *bindings);
void _r_add_expectation(Receptor *r,Aspect aspect,Symbol carrier,T *pattern,T* params,T *action);
void _r_remove_listener(Receptor *r,T *listener);
void _r_free(Receptor *r);

/*****************  receptor symbols, structures, and processes */
SemanticID __set_label_for_def(Receptor *r,char *label,T *def,int type);

Symbol _r_declare_symbol(Receptor *r,Structure s,char *label);
Symbol _r_get_symbol_by_label(Receptor *r,char *label);
Structure _r_define_structure(Receptor *r,char *label,int num_params,...);
Structure _r_get_structure_by_label(Receptor *r,char *label);
Structure __r_get_symbol_structure(Receptor *r,Symbol s);
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface);
Process _r_code_process(Receptor *r,T *code,char *name,char *intention,T *signature);

T *_r_build_def_semtrex(Receptor *r,Symbol s);
int _r_def_match(Receptor *r,Symbol s,T *t);

void _r_express_protocol(Receptor *r,int idx,Symbol sequence,Aspect aspect,T* handler);

/*****************  receptor instances and xaddrs */

Xaddr _r_new_instance(Receptor *r,T *t);
T *_r_get_instance(Receptor *r,Xaddr x);
T * _r_set_instance(Receptor *r,Xaddr x,T *t);
TreeHash _r_hash(Receptor *r,Xaddr t);

/******************  receptor serialization */
void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP);
Receptor * _r_unserialize(void *surface);

/******************  receptor signaling */

T * __r_make_signal(ReceptorAddress from,ReceptorAddress to,Aspect aspect,T *signal_contents,UUIDt *in_response_to,T* until);
T *__r_build_wakeup_info(T *code_point,int process_id);
T* __r_send(Receptor *r,T *signal);
T* _r_send(Receptor *r,T *signal);
T* _r_request(Receptor *r,T *signal,Symbol response_carrier,T *code_point,int process_id);
void __r_check_listener(T* processes,T *listener,T *signal,Q *q);
Error _r_deliver(Receptor *r, T *signal);

/******************  internal utilities */
T *__r_get_aspect(Receptor *r,Aspect aspect);
T *__r_get_listeners(Receptor *r,Aspect aspect);
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

#define spec_is_xaddr_equal(r,got,expected)  spec_total++; if (is_xaddr_eq(got,expected)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s.%d but was %s.%d",__FUNCTION__,__LINE__,#got,!is_null_symbol(expected.symbol)?_r_get_symbol_name(r,expected.symbol):"0",expected.addr,!is_null_symbol(got.symbol) ? _r_get_symbol_name(r,got.symbol):"0",got.addr);}

/*****************  Built-in core and edge receptors */
Receptor *_r_makeStreamReaderReceptor(Symbol receptor_symbol,Symbol stream_symbol,Stream *stream,ReceptorAddress to);
Receptor *_r_makeStreamWriterReceptor(Symbol receptor_symbol,Symbol stream_symbol,Stream *stream);
Receptor *_r_makeClockReceptor();
void *___clock_thread(void *arg);
#define __r_make_tick() __r_make_timestamp(TICK,00)
T *__r_make_timestamp(Symbol s,int delta);
void __r_kill(Receptor *r);
ReceptorAddress __r_get_self_address(Receptor *r);

void __r_dump_instances(Receptor *r);

#endif
/** @}*/
