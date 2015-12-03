/**
 * @defgroup def Symbol and Structure Definition
 * @brief Data has both meaningful and structural aspects, both of which must be defined.
 *
 * Structures are definitons of how data is organized; Symbols are a numeric semantic indicator (like a type) of what a given structure means.  For example an "integer"" defines a structures of bits and how to interpret them, whereas a "shoe size" is a symbol for what a use of an integer means.
 *
 * @{
 * @file def.h
 * @brief header file for symbol and structure definition functions
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_DEF_H
#define _CEPTR_DEF_H

#include "tree.h"
#include <stdarg.h>

// structure position index enums to make code more readable
// @todo figure out a better way to handle this... like put defs like these
// into the symbol gen code (talk about a semantic muddle! If only I had ceptr...)
// Note, that I tried that and it's hard!
enum {ReceptorDefsIdx=1,ReceptorFluxIdx,ReceptorStateIdx,ReceptorPendingSignalsIdx,ReceptorPendingResponsesIdx};
enum {EnvelopeFromIdx=1,EnvelopeToIdx,EnvelopeAspectIdx,EnvelopeCarrierIdx,EnvelopeUUIDIdx,EnvelopeExtraIdx};
enum {SignalEnvelopeIdx=1,SignalBodyIdx};
enum {PendingResponseUUIDIdx=1,PendingResponseCarrierIdx,PendingResponseWakeupIdx,PendingResponseEndCondsIdx};
enum {WakeupReferenceProcessIdentIdx=1,WakeupReferenceCodePathIdx};
enum {ExpectationCarrierIdx=1,ExpectationPatternIdx,ExpectationActionIdx,ExpectationParamsIdx,ExpectationEndCondsIdx};
enum DATEIndexes {dateYearIdx=1,dateMonthIdx,dateDayIdx};
enum TIMEIndexes {timeHourIdx=1,timeMinuteIdx,timeSecondIdx};
enum TIMESTAMPIndexes {timestampTodayIdx=1,timestampNowIdx};
enum ASPECTIndexes {aspectExpectationsIdx=1,aspectSignalsIdx};
enum {ExpectRoleIdx=1,ExpectSourceIdx,ExpectPatternIdx,ExpectActionIdx};
enum {InitiateRoleIdx=1,InitiateDestinationIdx,InitiateActionIdx};
enum {SourceRoleIdx=1};
enum {DefLabelIdx=1,SymbolDefStructureIdx};
enum {ProcessDefNameIdx=1,ProcessDefIntentionIdx,ProcessDefCodeIdx,ProcessDefSignatureIdx};
enum {RuntreeCodeIdx=1,RunTreeParamsIdx,RunTreeErrorCodeIdx};
enum {ResolutionWhichIdx=1};
enum {SemtrexMatchSymbolIdx=1,SemtrexMatchPathIdx,SemtrexMatchSibsIdx};
enum {InclusionPnameIdx=1};

#define ST(r,name,num,...) name = _r_define_structure(r,"" #name "",num,__VA_ARGS__)
#define SY(r,name,str) name = _r_define_symbol(r,str,"" #name "")
#define SP(r,code,name,intention,signature) name = _r_define_process(r,code,"" #name "",intention,signature)

int semeq(SemanticID s1,SemanticID s2);
SemanticID _d_define(SemTable *sem,T *def,SemanticType semtype,Context c);
char *__d_get_sem_name(T *defs,SemanticID s);
void __d_validate_symbol(SemTable *sem,Symbol s,char *n);
void __d_validate_structure(SemTable *sem,Structure s,char *n);
T *_d_make_symbol_def(Structure s,char *label);
void __d_set_symbol_structure(T *symbols,Symbol sym,Structure s);
SemanticAddr  _d_get_def_addr(T *def);
Symbol _d_define_symbol(SemTable *sem,Structure s,char *label,Context c);
T *_d_make_structure_def(char *label,T *structure_def);
Structure _d_define_structure(SemTable *sem,char *label,T *structure_def,Context c);
Structure _d_define_structure_v(SemTable *sem,char *label,Context c,int num_params,...);
T * _d_make_vstruc_def(SemTable *sem,char *label,int num_params,va_list params);
Structure __d_get_symbol_structure(T *symbols,Symbol s);
size_t _d_get_symbol_size(SemTable *sem,Symbol s,void *surface);
size_t _d_get_structure_size(SemTable *sem,Symbol s,void *surface);
T *_d_make_process_def(T *code,char *name,char *intention,T *signature);
Process _d_define_process(SemTable *sem,T *code,char *name,char *intention,T *signature,Context c);
Protocol _d_define_protocol(SemTable *sem,T *def,Context c);
T *_d_make_protocol_def(SemTable *sem,char *label,...);
T * _d_build_def_semtrex(SemTable *sem,Symbol s,T *parent);
size_t _sys_structure_size(int id,void *surface);

enum{NO_INDENT=0,INDENT=-1};
#define t2s(t) _t2s(G_sem,t)
#define t2sp(t) __t2s(G_sem,t,4)
#define _t2s(d,t) __t2s(d,t,NO_INDENT)
char * __t2s(SemTable *sem,T *t,int indent);
char *_indent_line(int level,char *buf);
char * __t_dump(SemTable *sem,T *t,int level,char *buf);

#define _d_get_def(defs,s) _t_child(defs,(s).id);
#define _d_get_process_code(processes,p) _d_get_def(processes,p)
#define _d_get_symbol_def(symbols,s) _d_get_def(symbols,s)
#define _d_get_structure_def(structures,s) _d_get_def(structures,s)

#define spec_is_sem_equal(got, expected) spec_total++; if (semeq(expected,got)){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %d.%d.%d but was %d.%d.%d",__FUNCTION__,__LINE__,#got,(expected).semtype,(expected).context,(expected).id,(got).semtype,(got).context,(got).id);}

void def_sys();
void sys_free();
#endif
/** @}*/
