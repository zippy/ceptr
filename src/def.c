/**
 * @ingroup def
 *
 * @{
 * @file def.c
 * @brief implementation file for symbol and structure definition functions
 * @todo refactor symbol/structure/process definition mechanisms into a general definitional form
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 */

#include "tree.h"
#include "stream.h"
#include "def.h"
#include "semtrex.h"
char __d_extra_buf[100];

int semeq(SemanticID s1,SemanticID s2) {
    return (memcmp(&s1,&s2,sizeof(SemanticID))==0);
}

/**
 * get symbol's label
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the Symbol to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testSymbolGetName
 */
char *__d_get_sem_name(T *defs,SemanticID s) {
    char *n = NULL;
    T *def = _t_child(defs,s.id);
    if (def) {
        T *t = _t_child(def,DefLabelIdx);  // label is always first child
        n = (char *)_t_surface(t);
    }
    return n;
}

// internal check to see if a symbol is valid
void __d_validate_symbol(SemTable *sem,Symbol s,char *n) {
    if (!is_symbol(s)) raise_error("Bad symbol in %s def: semantic type not SEM_TYPE_SYMBOL",n);
    if (is_sys_symbol(s) && (s.id == 0)) return; // NULL_SYMBOL ok
    T *symbols = _sem_get_defs(sem,s);
    if (!_t_child(symbols,s.id)) raise_error("Bad symbol in %s def: definition not found in context",n);
}

// internal check to see if a structure is valid
void __d_validate_structure(SemTable *sem,Structure s,char *n) {
    if (!is_structure(s)) raise_error("Bad structure in %s def: semantic type not SEM_TYPE_STRUCTURE",n);
    if (is_sys_structure(s) && (s.id == 0)) return; // NULL_STRUCTURE ok
    T *structures = _sem_get_defs(sem,s);
    if(s.id && !_t_child(structures,s.id)) {raise_error("Unknown structure <%d.%d.%d> in declaration of %s",s.context,s.semtype,s.id,n);}
}

/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @returns the new symbol def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
T *__d_declare_symbol(T *symbols,Structure s,char *label){
    T *def = _t_newr(symbols,SYMBOL_DECLARATION);
    _t_new(def,SYMBOL_LABEL,label,strlen(label)+1);
    _t_news(def,SYMBOL_STRUCTURE,s);
    return def;
}

// this is used to reset the structure of a symbol that has been pre declared as NULL_SYMBOL
// to it's actual value.
void __d_set_symbol_structure(T *symbols,Symbol sym,Structure s) {
    T *t = _t_child(symbols,sym.id);
    if (!t) raise_error("def not found!");
    T * structure_def = _t_child(t,SymbolDefStructureIdx);
    if (!semeq(NULL_STRUCTURE,*(Symbol *)_t_surface(structure_def)))
        raise_error("Symbol already defined");
    *(Symbol *)_t_surface(structure_def) = s;
}

// this is used to set the structure definition of a declared but undefined strcture
void __d_set_structure_def(T *structures,Structure s,T *def) {
    T *d = _t_child(structures,s.id);
    if (_t_children(d) > 1) raise_error("Structure already defined");
    _t_add(d,def);
}

// used to find the semantic address for a def when building a SemanticID
// @todo this is kinda backwards perhaps because it's all based on the fact
// that the SemanticAddr is the child index in the def tree.  This may not
// need to be changed
SemanticAddr  _d_get_def_addr(T *def) {
    int *path = _t_get_path(def);
    return path[_t_path_depth(path)-1];
}

/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] sem is the semantic table to which to add the symbol
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @param[in] the context in which this symbol is being declared
 * @returns the new symbol
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
Symbol _d_declare_symbol(SemTable *sem,Structure s,char *label,Context c){
    __d_validate_structure(sem,s,label);
    T *symbols = __sem_get_defs(sem,SEM_TYPE_SYMBOL,c);
    T *def = __d_declare_symbol(symbols,s,label);
    Symbol sym = {c,SEM_TYPE_SYMBOL,_d_get_def_addr(def)};
    return sym;
}

// low level structure definition helper.  You can pass in a null structure
// def to simply declare the existence of the structure without defining it.
T *__d_define_structure(T *structures,char *label,T *structure_def) {
    T *def = _t_newr(structures,STRUCTURE_DEFINITION);
    T *l = _t_new(def,STRUCTURE_LABEL,label,strlen(label)+1);
    if (structure_def) _t_add(def,structure_def);
    return def;
}

/**
 * helper to add a STRUCTURE_SEQUENCE of symbols structure definition to a semantic table
 *
 * @param[in] sem is the semantic table to which to add the structure
 * @param[in] label a c-string label for this structures
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new structure def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefStructure
 */
Structure _d_define_structure(SemTable *sem,char *label,Context c,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    T *def = _dv_define_structure(sem,label,c,num_params,params);
    va_end(params);
    Structure s = {c,SEM_TYPE_STRUCTURE,_d_get_def_addr(def)};
    return s;
}

/// va_list version of _d_define_structure
T * _dv_define_structure(SemTable *sem,char *label,Context c,int num_params,va_list params) {
    T *structures = __sem_get_defs(sem,SEM_TYPE_STRUCTURE,c);
    T *p,*seq = 0;
    if (num_params > 1)
        seq = _t_newr(0,STRUCTURE_SEQUENCE);
    int i;
    for(i=0;i<num_params;i++) {
        Symbol s = va_arg(params,Symbol);
        __d_validate_symbol(sem,s,label);
        p = _t_news(seq,STRUCTURE_SYMBOL,s);
    }
    return __d_define_structure(structures,label,seq ? seq : p);
}

/**
 * get the structure for a given symbol
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the symbol
 * @returns a Structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSymbolStructure
 */
Structure __d_get_symbol_structure(T *symbols,Symbol s) {
    T *def = _t_child(symbols,s.id);
    if (!def) {
        raise_error("Bad symbol:%d.%d.%d-- only %d symbols in decl list",s.context,s.semtype,s.id,_t_children(symbols));
    }
    T *t = _t_child(def,SymbolDefStructureIdx);
    return *(Structure *)_t_surface(t);
}

/**
 * get the size of a symbol
 *
 * @param[in] sem is the semantic table where symbols and structures are define
 * @param[in] s the symbol
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_symbol_size(SemTable *sem,Symbol s,void *surface) {
    Structure st = _sem_get_symbol_structure(sem,s);
    return _d_get_structure_size(sem,st,surface);
}

size_t _sys_structure_size(int id,void *surface) {
    switch(id) {
    case TREE_ID:
    case NULL_STRUCTURE_ID: return 0;
        //      case SEMTREX: return
    case SYMBOL_ID: return sizeof(Symbol);
    case BIT_ID:
    case INTEGER_ID: return sizeof(int);
    case FLOAT_ID: return sizeof(float);
    case CSTRING_ID: return strlen(surface)+1;
    case XADDR_ID: return sizeof(Xaddr);
    case STREAM_ID:
    case RECEPTOR_ID:
    case SCAPE_ID:
        return sizeof(void *);
    default: return -1;
    }
}

/**
 * get the size of a structure
 *
 * @param[in] sem is the semantic table where symbols and structures are define
 * @param[in] s the structure
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_structure_size(SemTable *sem,Structure s,void *surface) {
    size_t size = 0;
    T *structures = _sem_get_defs(sem,s);

    if (is_sys_structure(s)) {
        size = _sys_structure_size(s.id,surface);
        if (size == -1) {
            raise_error("DON'T HAVE A SIZE FOR STRUCTURE '%s' (%d)",_sem_get_name(sem,s),s.id);
        }
    }
    else {
        T *structure = _t_child(structures,s.id);
        T *parts = _t_child(structure,2);
        if (semeq(_t_symbol(parts),STRUCTURE_SEQUENCE)) {
            DO_KIDS(parts,
                    T *p = _t_child(parts,i);
                    if (!semeq(_t_symbol(p),STRUCTURE_SYMBOL)) {
                        raise_error("CAN'T GET SIZE FOR VARIABLE STRUCTURES '%s' (%d)",_sem_get_name(sem,s),s.id);
                    }
                    size += _d_get_symbol_size(sem,*(Symbol *)_t_surface(p),surface +size);
                    );
        }
        else if (semeq(_t_symbol(parts),STRUCTURE_SYMBOL)) {
            size = _d_get_symbol_size(sem,*(Symbol *)_t_surface(parts),surface);
        }
    }
    return size;
}

/**
 * add a new process coding to the processes tree
 *
 * @param[inout] processes a process def tree containing process codings which will be added to
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] signature the signature for the process
 * @returns the newly defined process
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
T *__d_code_process(T *processes,T *code,char *name,char *intention,T *signature) {
    T *def = _t_newr(processes,PROCESS_CODING);
    _t_new_str(def,PROCESS_NAME,name);
    _t_new(def,PROCESS_INTENTION,intention,strlen(intention)+1);
    if (code) _t_add(def,code);
    // @todo what indicator do we add for system processes that are defined in C?
    //    else if(processes != G_contexts[SYS_CONTEXT].defs.processes) raise_error("missing code");
    if (signature) _t_add(def,signature);
    return def;
}

/**
 * add a new process coding to the processes tree
 *
 * @param[inout] processes a process def tree containing process codings which will be added to
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] signature the signature for the process
 * @returns the process identifier
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
Process _d_code_process(SemTable *sem,T *code,char *name,char *intention,T *signature,Context c) {
    T *processes = __sem_get_defs(sem,SEM_TYPE_PROCESS,c);
    T *def = __d_code_process(processes,code,name,intention,signature);
    Process p = {c,SEM_TYPE_PROCESS,_d_get_def_addr(def)};
    return p;
}

/**
 * add a new protocol to a protocols tree
 *
 * @param[inout] protocols a protocols def tree containing protocols which will be added to
 * @param[in] label the name of the protocol
 * @param[in] def the protocol definition
 *
 */
T *__d_define_protocol(T *protocols,T *def) {
    _t_add(protocols,def);
    return def;
}

/**
 * add a protocol definition to a protocol defs tree
 *
 * @param[in] sem is the semantic table to which to add the protocol
 * @param[in] def the protocol definition
 * @param[in] the context in which this protocol is being defined
 * @param[out] the semantic id of the new protocol
 *
 */
Protocol _d_define_protocol(SemTable *sem,T *def,Context c) {
    //__d_validate_protocol(sem,def); //@todo
    T *protocols = __sem_get_defs(sem,SEM_TYPE_PROTOCOL,c);
    T *pdef = __d_define_protocol(protocols,def);
    Protocol p = {c,SEM_TYPE_PROTOCOL,_d_get_def_addr(pdef)};
    return p;
}

/**
 * Walks the definition of a symbol to build a semtrex that would match that definiton
 *
 * @param[in] defs the definition context
 * @param[in] s the symbol to build a semtrex for
 * @param[in] parent the parent semtrex node because this function calls itself recursively.  Pass in 0 to start.
 * @returns the completed semtrex
 * @todo currently this won't detect an incorrect strcture with extra children.
 This is because we don't haven't yet implemented the equivalent of "$" for semtrex.
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSemtrex
*/
T * _d_build_def_semtrex(SemTable *sem,Symbol s,T *parent) {
    T *stx = _sl(parent,s);

    Structure st = _sem_get_symbol_structure(sem,s);
    if (!(is_sys_structure(st))) {
        T *structure = _d_get_structure_def(_sem_get_defs(sem,st),st);
        T *parts = _t_child(structure,2);
        int i,c = _t_children(parts);
        if (c > 0) {
            T *seq = _t_newr(stx,SEMTREX_SEQUENCE);
            for(i=1;i<=c;i++) {
                T *p = _t_child(parts,i);
                _d_build_def_semtrex(sem,*(Symbol *)_t_surface(p),seq);
            }
        }
    }
    return stx;
}


/*****************  Tree debugging utilities */

/**
 * tree 2 string: returns a string representation of a tree
 * @TODO make this actually not break on large trees!
 */
char * __t2s(SemTable *sem,T *t,int indent) {
    static char buf[100000];
    buf[0] = 0;
    return __t_dump(sem,t,indent,buf);
}

char *_indent_line(int level,char *buf) {
    if (level < -1) {
        int j = (-level - 1)*3;
        *buf++ = '\n';
        while(j--) *buf++ = ' ';
        *buf = 0;
    }
    else if (level > 0) {
        *buf++ = ' ';
    }
    return buf;
}

char * __t_dump(SemTable *sem,T *t,int level,char *buf) {
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *c;
    Xaddr x;
    buf = _indent_line(level,buf);

    // use this to mark all run nodes with a %
    // if (t->context.flags & TFLAG_RUN_NODE) *buf++ = '%';

    char *n = _sem_get_name(sem,s);

    if (is_process(s)) {
        sprintf(buf,"(process:%s",n);
    }
    else {
        Structure st = _sem_get_symbol_structure(sem,s);
        if (!is_sys_structure(st)) {
            // if it's not a system structure, it's composed, so all we need to do is
            // print out the symbol name, and the reset will take care of itself
            sprintf(buf,"(%s",n);
        }
        else {
            switch(st.id) {
            case ENUM_ID: // for now enum surfaces are just strings so we can see the text value
            case CSTRING_ID:
                sprintf(buf,"(%s:%s",n,(char *)_t_surface(t));
                break;
            case BLOB_ID:
                sprintf(buf,"(%s:%ld-byte-blob",n,_t_size(t));
                break;
            case CHAR_ID:
                sprintf(buf,"(%s:'%c'",n,*(char *)_t_surface(t));
                break;
            case BIT_ID:
            case INTEGER_ID:
                sprintf(buf,"(%s:%d",n,*(int *)_t_surface(t));
                break;
            case FLOAT_ID:
                sprintf(buf,"(%s:%f",n,*(float *)_t_surface(t));
                break;
            case SYMBOL_ID:
            case STRUCTURE_ID:
            case PROCESS_ID:
            case PROTOCOL_ID:
                c = _sem_get_name(sem,*(SemanticID *)_t_surface(t));
                sprintf(buf,"(%s:%s",n,c?c:"<unknown>");
                break;
            case TREE_PATH_ID:
                sprintf(buf,"(%s:%s",n,_t_sprint_path((int *)_t_surface(t),b));
                break;
            case XADDR_ID:
                x = *(Xaddr *)_t_surface(t);
                sprintf(buf,"(%s:%s.%d",n,_sem_get_name(sem,x.symbol),x.addr);
                break;
            /* case STREAM_ID: */
            /*     sprintf(buf,"(%s:%p",n,_t_surface(t)); */
            /*     break; */
            /* case UUID_ID: */
            /*     { */
            /*         UUIDt x = *(UUIDt *)_t_surface(t); */
            /*         sprintf(buf,"%s:%ld.%ld",n,x.data,x.time); */
            /*     } */
            /*     break; */
            case TREE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
                    c = __t_dump(sem,(T *)_t_surface(t),0,tbuf);
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
            case RECEPTOR_ID:
                if (t->context.flags & (TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR)) {
                    c = __t_dump(sem,((Receptor *)_t_surface(t))->root,0,tbuf);
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
            case SCAPE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_SCAPE) {
                    Scape *sc = (Scape *)_t_surface(t);
                    sprintf(buf,"(%s:key %s,data %s",n,_sem_get_name(sem,sc->key_source),_sem_get_name(sem,sc->data_source));
                    break;
                }
            default:
                if (semeq(s,SEMTREX_MATCH_CURSOR)) {
                    c = __t_dump(sem,*(T **)_t_surface(t),0,tbuf);
                    //c = "null";
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
                if (n == 0)
                    sprintf(buf,"(<unknown:%d.%d.%d>",s.context,s.semtype,s.id);
                else
                    sprintf(buf,"(%s",n);
            }
        }
    }
    DO_KIDS(t,__t_dump(sem,_t_child(t,i),level < 0 ? level-1 : level+1,buf+strlen(buf)));
    sprintf(buf+strlen(buf),")");
    return buf;
}

/** @}*/
