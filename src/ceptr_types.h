#ifndef _CEPTR_TYPES_H
#define _CEPTR_TYPES_H

#include <stdint.h>
#include "uthash.h"

enum SemanticContexts {SYS_CONTEXT,RECEPTOR_CONTEXT,TEST_CONTEXT};

enum SemanticTypes {SEM_TYPE_STRUCTURE=1,SEM_TYPE_SYMBOL,SEM_TYPE_PROCESS};
#define SEM_TYPE_MASK 0x00FF
#define is_symbol(s) (((s).flags & SEM_TYPE_MASK) == SEM_TYPE_SYMBOL)
#define is_process(s) (((s).flags & SEM_TYPE_MASK) == SEM_TYPE_PROCESS)
#define is_structure(s) (((s).flags & SEM_TYPE_MASK) == SEM_TYPE_STRUCTURE)

typedef uint16_t Context;
typedef struct SemanticID {
    Context context;
    uint16_t flags;
    uint32_t id;
} SemanticID;

typedef SemanticID Symbol;
typedef SemanticID Process;
typedef SemanticID Structure;

// ** types for trees
typedef struct Tstruct {
    struct T *parent;
    int child_count;
    struct T **children;
} Tstruct;

typedef struct Tcontents {
    Symbol symbol;
    size_t size;
    void *surface;
} Tcontents;

typedef struct Tcontext {
    int flags;
} Tcontext;

typedef struct T {
    Tstruct structure;
    Tcontext context;
    Tcontents contents;
} T;

typedef uint32_t TreeHash;

// ** types for labels
typedef uint32_t Label;

/**
 * An element in the label table.
 *
 */
typedef struct table_elem {
    UT_hash_handle hh;         ///< makes this structure hashable using the uthash library
    Label label;               ///< semantic key
    int path_s;                ///< first int of the path to the labeled item in the Receptor tree
} table_elem;
typedef table_elem *LabelTable;

/**
 * An element in the store of instances of one symbol type
 */
typedef struct instance_elem {
    int addr;                  ///< key to this instance
    T *instance;           ///< the stored instance
    UT_hash_handle hh;         ///< makes this structure hashable using the uthash library
} instance_elem;
typedef instance_elem *Instance;

/**
 * An element in the store for all the instances of one symbol type
 */
typedef struct instances_elem {
    Symbol s;                ///< key to store of this symbol type
    Instance instances;      ///< instances store
    int last_id;             ///< the last allocated id for instances
    UT_hash_handle hh;       ///< makes this structure hashable using the uthash library
} instances_elem;
typedef instances_elem *Instances;

// ** types for defs

/**
   a collection for passing around pointers to the various def trees
*/
typedef struct Defs {
    T *structures;  ///< pointer for quick access to structures
    T *symbols;     ///< pointer for quick access to symbols
    T *processes;   ///< pointer for quick access to processes
    T *protocols;   ///< pointer for quick access to protocols
    T *scapes;      ///< pointer for quick access to scapes
} Defs;

// ** types for receptors
/**
   A Receptor is a semantic tree, pointed to by root, but we also create c struct for
   faster access to some parts of the tree, and to hold non-tree data, like the label
   table.
*/
typedef struct Receptor {
    T *root;        ///< root node of the semantic tree
    Defs defs;          ///< defs block
    T *flux;        ///< pointer for quick access to the flux
    LabelTable table;   ///< the label table
    Instances instances;///< the instances store
} Receptor;

typedef long UUID;

/**
 * An eXistence Address consists of the semantic type (Symbol) and an address.
 */
typedef struct Xaddr {
    Symbol symbol;
    int addr;
} Xaddr;

typedef int Error;

// ** types for scapes

/**
 * An element in the scape key value pair store
 */
typedef struct scape_elem {
    TreeHash key;            ///< has of the key tree that maps to a given data value
    Xaddr value;             ///< instance of data_source pointed to by the key
    UT_hash_handle hh;       ///< makes this structure hashable using the uthash library
} scape_elem;
typedef scape_elem *ScapeData;

/**
 * A scape provides indexed, i.e. random access to data sources.  The key source is
 * usually a sub-portion of a the data source, i.e. if the data source is a PROFILE
 * the key_source might be a FIRST_NAME within the profile
 */
typedef struct Scape {
    Symbol key_source;
    Symbol data_source;
    ScapeData data;      ///< the scape data store (hash table)
} Scape;

#endif
