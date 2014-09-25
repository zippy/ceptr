/**
 * @defgroup receptor Receptors
 * @brief Receptors provide the fundamental coherence contexts for ceptr
 *
 * @{
 * @file receptor.h
 * @brief receptor implementation header file
 *
 * @copyright Copyright (C) 2013-2014, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "tree.h"
#include "def.h"
#include "label.h"
#include "process.h"
#include "scape.h"

// for now aspects are just identified as the child index in the flux receptor
enum {DEFAULT_ASPECT=1};
typedef int Aspect;

/******************  create and destroy receptors */
Receptor *_r_new(Symbol s);
Receptor *_r_new_receptor_from_package(Symbol s,Tnode *p,Tnode *bindings);
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *semtrex,Tnode *action);
void _r_free(Receptor *r);

/*****************  receptor symbols, structures, and processes */
Symbol _r_declare_symbol(Receptor *r,Structure s,char *label);
Symbol _r_get_symbol_by_label(Receptor *r,char *label);
Structure _r_define_structure(Receptor *r,char *label,int num_params,...);
Structure _r_get_structure_by_label(Receptor *r,char *label);
Structure __r_get_symbol_structure(Receptor *r,Symbol s);
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface);
Process _r_code_process(Receptor *r,Tnode *code,char *name,char *intention,Tnode *in,Tnode *out);

Tnode *_r_build_def_semtrex(Receptor *r,Symbol s);
int _r_def_match(Receptor *r,Symbol s,Tnode *t);

/*****************  receptor instances and xaddrs */

Xaddr _r_new_instance(Receptor *r,Tnode *t);
Tnode *_r_get_instance(Receptor *r,Xaddr x);

/******************  receptor serialization */
size_t __t_serialize(Tnode *t,void **bufferP,size_t offset,size_t current_size,int compact);
void _t_serialize(Tnode *t,void **surfaceP, size_t *lengthP);
void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP);
Receptor * _r_unserialize(void *surface);

/******************  receptor signaling */
/// @todo for now the signal is appended directly to the flux.  Later it should probably be copied
Tnode * _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal);

/******************  internal utilities */
Tnode *__r_get_aspect(Receptor *r,Aspect aspect);
Tnode *__r_get_listeners(Receptor *r,Aspect aspect);
Tnode *__r_get_signals(Receptor *r,Aspect aspect);


/*****************  Tree debugging utilities */
char *_td(Receptor *r,Tnode *t);

#define spec_is_symbol_equal(r,got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_symbol_name(r,expected),_r_get_symbol_name(r,got));}

#define spec_is_structure_equal(r,got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_structure_name(r,expected),_r_get_structure_name(r,got));}

#define spec_is_process_equal(r,got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_r_get_process_name(r,expected),_r_get_process_name(r,got));}

char *_r_get_structure_name(Receptor *r,Structure s);
char *_r_get_symbol_name(Receptor *r,Symbol s);
char *_r_get_process_name(Receptor *r,Process p);

Xaddr G_null_xaddr;

#define is_null_xaddr(x) ((x).symbol == 0 && (x).addr == 0)

#endif

/** @}*/
