/**
 * @file receptor.h
 * @brief Receptors provide the fundamental coherence contexts for ceptr
 *
 */

#ifndef _CEPTR_RECEPTOR_H
#define _CEPTR_RECEPTOR_H

#include "tree.h"
#include "label.h"

/**
   A Receptor is a semantic tree, pointed to by root, but we also create c struct for
   faster access to some parts of the tree, and to hold non-tree data, like the label
   table.
*/
struct Receptor {
    Tnode *root;        ///< root node of the semantic tree
    Tnode *structures;  ///< pointer for quick access to structures
    Tnode *symbols;     ///< pointer for quick access to symbols
    Tnode *flux;        ///< pointer for quick access to the flux
    LabelTable table;   ///< the label table
};
typedef struct Receptor Receptor;

// for now aspects are just identified as the child index in the flux receptor
enum {DEFAULT_ASPECT=1};
typedef int Aspect;

/******************  create and destroy receptors */
Receptor * _r_new();
void _r_add_listener(Receptor *r,Aspect aspect,Symbol carrier,Tnode *semtrex,Tnode *action);
void _r_free(Receptor *r);

/*****************  receptor symbols and structures */
Symbol _r_def_symbol(Receptor *r,Structure s,char *label);
Symbol _r_get_symbol_by_label(Receptor *r,char *label);
Structure _r_def_structure(Receptor *r,char *label,int num_params,...);
Structure _r_get_structure_by_label(Receptor *r,char *label);
Structure __r_get_symbol_structure(Receptor *r,Symbol s);
size_t __r_get_symbol_size(Receptor *r,Symbol s,void *surface);

/*****************  receptor instances and xaddrs */
struct Instance {
    void *surface;
};
typedef struct Instance Instance;

/**
 * An eXistence Address consists of the semantic type (Symbol) and an address.
 */
struct Xaddr {
    Symbol symbol;
    int addr;
};
typedef struct Xaddr Xaddr;
Xaddr _r_new_instance(Receptor *r,Symbol s,void * surface);
Instance _r_get_instance(Receptor *r,Xaddr x);

/******************  receptor serialization */
size_t __t_serialize(Receptor *r,Tnode *t,void **bufferP,size_t offset,size_t current_size);
void _r_serialize(Receptor *r,void **surfaceP,size_t *lengthP);
Receptor * _r_unserialize(void *surface,size_t length);

/******************  receptor signaling */
//*TODO: for now the signal is appended directly to the flux.  Later it should probably be copied
Tnode * _r_send(Receptor *r,Receptor *from,Aspect aspect, Tnode *signal);

/******************  internal utilities */
Tnode *__r_get_aspect(Receptor *r,Aspect aspect);
Tnode *__r_get_listeners(Receptor *r,Aspect aspect);
Tnode *__r_get_signals(Receptor *r,Aspect aspect);


/*****************  Tree debugging utilities */
char *_td(Receptor *r,Tnode *t);

#define spec_is_symbol_equal(r,got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_s_get_symbol_name(r,expected),_s_get_symbol_name(r,got));}

#define spec_is_structure_equal(r,got, expected) spec_total++; if (expected==got){putchar('.');} else {putchar('F');sprintf(failures[spec_failures++],"%s:%d expected %s to be %s but was %s",__FUNCTION__,__LINE__,#got,_s_get_symbol_name(r,expected),_s_get_structure_name(r,got));}

char *_s_get_symbol_name(Receptor *r,Symbol s);
char *_s_get_structure_name(Receptor *r,Structure s);

char * _dump_semtrex(Tnode *s,char *buf);

#endif
