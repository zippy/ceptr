#ifndef _CEPTR_TYPES_H
#define _CEPTR_TYPES_H

#include <stdint.h>
#include "uthash.h"

typedef uint32_t Symbol;
typedef uint32_t Process;
typedef uint32_t Structure;

// ** types for trees
struct Tstruct {
    struct Tnode *parent;
    int child_count;
    struct Tnode **children;
};
typedef struct Tstruct Tstruct;

struct Tcontents {
    Symbol symbol;
    size_t size;
    void *surface;
};
typedef struct Tcontents Tcontents;

struct Tcontext {
    int flags;
};
typedef struct Tcontext Tcontext;

struct Tnode {
    Tstruct structure;
    Tcontext context;
    Tcontents contents;
};
typedef struct Tnode Tnode;

typedef uint32_t TreeHash;

// ** types for labels
typedef uint32_t Label;

/**
 * An element in the label table.
 *
 * @todo convert path to variable sized item
 */
struct table_elem {
    UT_hash_handle hh;         ///< makes this structure hashable using the uthash library
    Label label;               ///< semantic key
    int path_s;                ///< first int of the path to the labeled item in the Receptor tree
};
typedef struct table_elem table_elem;
typedef table_elem *LabelTable;

/**
 * An element in the store of instances of one symbol type
 */
struct instance_elem {
    int addr;                  ///< key to this instance
    Tnode *instance;           ///< the stored instance
    UT_hash_handle hh;         ///< makes this structure hashable using the uthash library
};
typedef struct instance_elem instance_elem;
typedef instance_elem *Instance;

/**
 * An element in the store for all the instances of one symbol type
 */
struct instances_elem {
    Symbol s;                ///< key to store of this symbol type
    Instance instances;      ///< instances store
    int last_id;             ///< the last allocated id for instances
    UT_hash_handle hh;       ///< makes this structure hashable using the uthash library
};
typedef struct instances_elem instances_elem;
typedef instances_elem *Instances;

// ** types for defs

/**
   a collection for passing around pointers to the various def trees
*/
struct Defs {
    Tnode *structures;  ///< pointer for quick access to structures
    Tnode *symbols;     ///< pointer for quick access to symbols
    Tnode *processes;   ///< pointer for quick access to processes
    Tnode *scapes;      ///< pointer for quick access to scapes
 };
typedef struct Defs Defs;

// ** types for receptors
/**
   A Receptor is a semantic tree, pointed to by root, but we also create c struct for
   faster access to some parts of the tree, and to hold non-tree data, like the label
   table.
*/
struct Receptor {
    Tnode *root;        ///< root node of the semantic tree
    Defs defs;          ///< defs block
    Tnode *flux;        ///< pointer for quick access to the flux
    LabelTable table;   ///< the label table
    Instances instances;///< the instances store
};
typedef struct Receptor Receptor;

typedef long UUID;

/**
 * An eXistence Address consists of the semantic type (Symbol) and an address.
 */
struct Xaddr {
    Symbol symbol;
    int addr;
};
typedef struct Xaddr Xaddr;

typedef int Error;

// ** types for scapes

/**
 * An element in the scape key value pair store
 */
struct scape_elem {
    Xaddr value;
    UT_hash_handle hh;       ///< makes this structure hashable using the uthash library
};
typedef struct scape_elem scape_elem;
typedef scape_elem *ScapeData;

/**
 * A scape is a sematic tree, pointed to by a root, but we also create a c structure
 * for faster access to some parts of the tree and to hold the non-tree like date, like
 * hash tables for key/data access
 */
struct Scape {
    Tnode *root;         ///< root node of the semantic tree
    ScapeData data;      ///< the scape data store
};
typedef struct Scape Scape;

#endif
