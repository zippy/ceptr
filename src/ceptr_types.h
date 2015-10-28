#ifndef _CEPTR_TYPES_H
#define _CEPTR_TYPES_H

#include <stdint.h>
#include <stdio.h>
#include "uthash.h"

enum SemanticContexts {SYS_CONTEXT,COMPOSITORY_CONTEXT,LOCAL_CONTEXT,TEST_CONTEXT,_NUM_CONTEXTS,RECEPTOR_CONTEXT=0xff};

// NOTE: the actual values of the types matter because they must match the order they show
// up in the definition trees
enum SemanticTypes {SEM_TYPE_STRUCTURE=1,SEM_TYPE_SYMBOL,SEM_TYPE_PROCESS};
#define is_symbol(s) ((s).semtype == SEM_TYPE_SYMBOL)
#define is_process(s) ((s).semtype == SEM_TYPE_PROCESS)
#define is_structure(s) ((s).semtype == SEM_TYPE_STRUCTURE)

typedef uint8_t Context;
typedef uint8_t SemanticType;

typedef struct SemanticID {
    Context context;
    SemanticType semtype;
    uint32_t id;
    uint16_t _reserved; // add an extra 16 bits to make this a 64bit structure.
} SemanticID;

typedef SemanticID Symbol;
typedef SemanticID Process;
typedef SemanticID Structure;

typedef uint16_t Mlevel;
typedef uint32_t Mindex;
typedef uint32_t Mmagic;

// ** types for matrix trees
typedef struct N {
    Symbol symbol;
    Mindex parenti;
    uint32_t flags;
    size_t size;
    void *surface;  // this item must be last!!!
} N;

typedef struct L {
    Mindex nodes;
    N *nP;
} L;

typedef struct M {
    Mmagic magic;
    Mlevel levels;
    L *lP;
} M;

// node entries are fixed size but the surface when serialized is an offset
// in the blob not a pointer
#define SERIALIZED_NODE_SIZE (sizeof(N)-sizeof(void *)+sizeof(size_t))
#define SERIALIZED_LEVEL_SIZE(l) (sizeof(Mindex)+SERIALIZED_NODE_SIZE*l->nodes)
#define SERIALIZED_HEADER_SIZE(levels) (sizeof(S)+sizeof(uint32_t)*(levels))

typedef struct S {
    Mmagic magic;
    size_t total_size;
    Mlevel levels;
    uint32_t blob_offset;
    uint32_t level_offsets[];
} S;

#define NULL_ADDR -1
typedef struct Maddr {
    Mlevel l;
    Mindex i;
} Maddr;

// ** generic tree type defs
typedef struct H {
    M *m;
    Maddr a;
} H;

enum treeImplementations {ptrImpl=0xfffffffe,matrixImpl=0xffffffff};
#define FIRST_TREE_IMPL_TYPE ptrImpl
#define LAST_TREE_IMPL_TYPE matrixImpl


// ** types for pointer trees
typedef struct Tstruct {
    uint32_t child_count;
    struct T *parent;
    struct T **children;
} Tstruct;

typedef struct Tcontents {
    Symbol symbol;
    size_t size;
    void *surface;
} Tcontents;

typedef struct Tcontext {
    uint32_t flags;
} Tcontext;

/**
 * A tree node
 *
 */
typedef struct T {
    Tstruct structure;
    Tcontext context;
    Tcontents contents;
} T;

#define RUN_TREE_NOT_EVAULATED       0
#define RUN_TREE_EVALUATED           0xffffffff

/**
 * A run tree node
 *  just like T nodes but with run state data appended
 * used in rclone
 **/
typedef struct rT {
    Tstruct structure;
    Tcontext context;
    Tcontents contents;
    uint32_t cur_child;
} rT;

// macro helper to get at the cur_child element of a run-tree node when given a regular
// node (does the casting to make code look cleaner)
#define rt_cur_child(tP) (((rT *)tP)->cur_child)

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

// ** types for processing
// run-tree context
typedef struct R R;
struct R {
    int id;           ///< the process id this context exists in
    int err;          ///< process error value
    int state;        ///< process state machine state
    T *run_tree;      ///< pointer to the root of the run_tree
    T *node_pointer;  ///< pointer to the tree node to execute next
    T *parent;        ///< node_pointer's parent      (cached here for efficiency)
    int idx;          ///< node pointers child index  (cached here for efficiency)
    R *caller;        ///< a pointer to the context that invoked this run-tree/context
    R *callee;        ///< a pointer to the context we've invoked
};

// ** structure to hold in process accounting
typedef struct Accounting Accounting;
struct Accounting {
    uint64_t elapsed_time;
};

// Processing Queue element
typedef struct Qe Qe;
struct Qe {
    int id;
    R *context;
    Accounting accounts;
    Qe *next;
    Qe *prev;
};

typedef int ReceptorAddress;

typedef struct Receptor Receptor;

// Processing Queue structure
typedef struct Q Q;
struct Q {
    Receptor *r;         ///< back-pointer to receptor in which this Q is running (for defs and more)
    int contexts_count;  ///< number of active processes
    Qe *active;          ///< active processes
    Qe *completed;       ///< completed processes (pending cleanup)
    Qe *blocked;         ///< blocked processes
    pthread_mutex_t mutex;
};


// ** types for receptors
enum ReceptorStates {Alive=0,Dead};

/**
   A Receptor is a semantic tree, pointed to by root, but we also create c struct for
   faster access to some parts of the tree, and to hold non-tree data, like the label
   table.
*/
struct Receptor {
    T *root;             ///< root node of the semantic tree
    Defs defs;           ///< defs block
    T *flux;             ///< pointer for quick access to the flux
    T *pending_signals;
    T *pending_responses;
    pthread_mutex_t pending_signals_mutex;
    pthread_mutex_t pending_responses_mutex;
    LabelTable table;    ///< the label table
    Instances instances; ///< the instances store
    Q *q;                ///< process queue
    int state;           ///< state information about the receptor that the vmhost manages
};

typedef struct UUIDt {
    uint64_t data;
    uint64_t time;
} UUIDt;

// receptors receive signals on aspects
// for now aspects are just identified as the child index in the flux receptor
enum {DEFAULT_ASPECT=1};
// aspects appear on either side of the membrane
enum AspectType {EXTERNAL_ASPECT=0,INTERNAL_ASPECT};
typedef int Aspect;

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
