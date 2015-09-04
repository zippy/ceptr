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
char *_d_get_symbol_name(T *symbols,Symbol s) {
    if (s.context != RECEPTOR_CONTEXT)
        symbols = G_contexts[s.context].defs.symbols;
    if (is_sys_symbol(s) && (s.id == 0))
        return "NULL_SYMBOL";
    if (symbols) {
        int c = _t_children(symbols);
        if (s.id > c || s.id < 1)  {
            raise_error("Bad symbol:%d--%d symbols in decl list",s.id,c);
        }
        T *def = _t_child(symbols,s.id);
        T *l = _t_child(def,1);
        return (char *)_t_surface(_t_child(def,1));
    }
    sprintf(__d_extra_buf,"<unknown symbol:%d.%d.%d>",s.context,s.semtype,s.id);
    return __d_extra_buf;
}

/**
 * get structure's label
 *
 * @param[in] structures a structre def tree containing structure definitions
 * @param[in] s the Structure to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testStructureGetName
 */
char *_d_get_structure_name(T *structures,Structure s) {
    if (s.context != RECEPTOR_CONTEXT)
        structures = G_contexts[s.context].defs.structures;
    if (is_sys_structure(s) && (s.id == 0))
        return "NULL_STRUCTURE";
    if (structures) {
        T *def = _t_child(structures,s.id);
        T *l = _t_child(def,1);
        return (char *)_t_surface(l);
    }
    sprintf(__d_extra_buf,"<unknown structure:%d.%d.%d>",s.context,s.semtype,s.id);
    return __d_extra_buf;
}

/**
 * get processes's label
 *
 * @param[in] processes a process def tree containing structure definitions
 * @param[in] p the Process to return the name for
 * @returns char * pointing to label
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testProcessGetName
 */
char *_d_get_process_name(T *processes,Process p) {
    if (p.context != RECEPTOR_CONTEXT)
        processes = G_contexts[p.context].defs.processes;
    if (is_sys_process(p) && (p.id == 0))
        return "NULL_PROCESS";
    if (processes) {
        T *def = _t_child(processes,p.id);
        if (def) {
            T *l = _t_child(def,1);
            return (char *)_t_surface(l);
        }
    }
    sprintf(__d_extra_buf,"<unknown process:%d.%d.%d>",p.context,p.semtype,p.id);
    return __d_extra_buf;
}

// internal check to see if a symbol is valid
void __d_validate_symbol(T *symbols,Symbol s,char *n) {
    if (!is_symbol(s)) raise_error("Bad symbol in %s def: semantic type not SEM_TYPE_SYMBOL",n);
    if (s.context != RECEPTOR_CONTEXT)
        symbols = G_contexts[s.context].defs.symbols;
    if (is_sys_symbol(s) && (s.id == 0)) return; // NULL_SYMBOL ok
    if (symbols) {
        if (!_t_child(symbols,s.id)) raise_error("Bad symbol in %s def: definition not found in context",n);
    }
    else raise_error("Bad symbol in %s def: context not found",n);
}

// internal check to see if a structure is valid
void __d_validate_structure(T *structures,Structure s,char *n) {
    if (!is_structure(s)) raise_error("Bad structure in %s def: semantic type not SEM_TYPE_STRUCTURE",n);

    if (s.context != RECEPTOR_CONTEXT)
        structures = G_contexts[s.context].defs.structures;
    if (is_sys_structure(s) && (s.id == 0)) return; // NULL_STRUCTURE ok
    if (structures) {
        if(s.id && !_t_child(structures,s.id)) {raise_error("Unknown structure <%d.%d.%d> in declaration of %s",s.context,s.semtype,s.id,n);}
    }
    else raise_error("Bad structure in %s def: context not found",n);
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

/**
 * add a symbol definition to a symbol defs tree
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] s the structure type for this symbol
 * @param[in] label a c-string label for this symbol
 * @param[in] the context in which this symbol is being declared
 * @returns the new symbol
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefSymbol
 */
Symbol _d_declare_symbol(T *symbols,T *structures,Structure s,char *label,Context c){
    __d_validate_structure(structures,s,label);
    __d_declare_symbol(symbols,s,label);
    Symbol sym = {c,SEM_TYPE_SYMBOL,_t_children(symbols)};
    return sym;
}

/**
 * add a structure definition to a structure defs tree
 *
 * @param[in] structures a structre def tree containing structure definitions
 * @param[in] label a c-string label for this symbol
 * @param[in] num_params number of symbols in the structure
 * @param[in] ... variable list of Symbol type symbols
 * @returns the new structure def
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testDefStructure
 */
Structure _d_define_structure(T *symbols, T *structures,char *label,Context c,int num_params,...) {
    va_list params;
    va_start(params,num_params);
    T *def = _dv_define_structure(symbols,structures,label,num_params,params);
    va_end(params);
    Symbol sym = {c,SEM_TYPE_STRUCTURE,_t_children(structures)};
    return sym;
}

/// va_list version of _d_define_structure
T * _dv_define_structure(T *symbols,T *structures,char *label,int num_params,va_list params) {
    T *def = _t_newr(structures,STRUCTURE_DEFINITION);
    T *l = _t_new(def,STRUCTURE_LABEL,label,strlen(label)+1);
    T *p = _t_newr(def,STRUCTURE_PARTS);
    int i;
    for(i=0;i<num_params;i++) {
        Symbol s = va_arg(params,Symbol);
        __d_validate_symbol(symbols,s,label);
        _t_news(p,STRUCTURE_PART,s);
    }
    return def;
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
Structure _d_get_symbol_structure(T *symbols,Symbol s) {
    if (!is_symbol(s)) raise_error("Bad symbol: semantic type not SEM_TYPE_SYMBOL");
    if (s.context != RECEPTOR_CONTEXT)
        symbols = G_contexts[s.context].defs.symbols;

    // NULL_SYMBOL has NULL_STRUCTURE ??
    if (is_sys_symbol(s) && (s.id == 0)) return NULL_STRUCTURE;

    if (symbols) {
        T *def = _t_child(symbols,s.id);
        T *t = _t_child(def,2); // second child of the def is the structure
        return *(Structure *)_t_surface(t);
    }
    raise_error("Context %d not found!",s.context);
}

/**
 * get the size of a symbol
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] structures a structure def tree containing structure definitions
 * @param[in] s the symbol
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_symbol_size(T *symbols,T *structures,Symbol s,void *surface) {
    Structure st = _d_get_symbol_structure(symbols,s);
    return _d_get_structure_size(symbols,structures,st,surface);
}

size_t _sys_structure_size(int id,void *surface) {
    switch(id) {
    case LIST_ID:
    case TREE_ID:
    case NULL_STRUCTURE_ID: return 0;
        //      case SEMTREX: return
    case SYMBOL_ID: return sizeof(Symbol);
    case BIT_ID:
    case INTEGER_ID: return sizeof(int);
    case FLOAT_ID: return sizeof(float);
    case CSTRING_ID: return strlen(surface)+1;
    case XADDR_ID: return sizeof(Xaddr);
    case STREAM_ID: return sizeof(Stream);
    default: return -1;
    }
}

/**
 * get the size of a structure
 *
 * @param[in] symbols a symbol def tree containing symbol definitions
 * @param[in] structures a structure def tree containing structure definitions
 * @param[in] s the structure
 * @param[in] surface the surface of the structure (may be necessary beause some structures have length info in the data)
 * @returns size of the structure
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testGetSize
 */
size_t _d_get_structure_size(T *symbols,T *structures,Structure s,void *surface) {
    size_t size = 0;
    if (is_sys_structure(s)) {
        size = _sys_structure_size(s.id,surface);
        if (size == -1) {
            raise_error("DON'T HAVE A SIZE FOR STRUCTURE '%s' (%d)",_d_get_structure_name(structures,s),s.id);
        }
    }
    else {
        T *structure = _t_child(structures,s.id);
        T *parts = _t_child(structure,2);
        DO_KIDS(parts,
            T *p = _t_child(parts,i);
            size += _d_get_symbol_size(symbols,structures,*(Symbol *)_t_surface(p),surface +size);
                );
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
 * @param[in] in the input signature for the process
 * @param[in] out the output signature for the process
 * @returns the newly defined process
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
T *__d_code_process(T *processes,T *code,char *name,char *intention,T *in,T *out) {
    T *def = _t_newr(processes,PROCESS_CODING);
    _t_new(def,PROCESS_NAME,name,strlen(name)+1);
    _t_new(def,PROCESS_INTENTION,intention,strlen(intention)+1);
    if (code) _t_add(def,code);
    if (in) _t_add(def,in);
    if (out) _t_add(def,out);
    return def;
}

/**
 * add a new process coding to the processes tree
 *
 * @param[inout] processes a process def tree containing process codings which will be added to
 * @param[in] code the code tree for this process
 * @param[in] name the name of the process
 * @param[in] intention a description of what the process intends to do/transform
 * @param[in] in the input signature for the process
 * @param[in] out the output signature for the process
 * @returns the process identifier
 * @todo this is not thread safe!
 *
 * <b>Examples (from test suite):</b>
 * @snippet spec/def_spec.h testCodeProcess
 */
Process _d_code_process(T *processes,T *code,char *name,char *intention,T *in,T *out,Context c) {
    __d_code_process(processes,code,name,intention,in,out);
    Symbol sym = {c,SEM_TYPE_PROCESS,_t_children(processes)};
    return sym;
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
T * _d_build_def_semtrex(Defs defs,Symbol s,T *parent) {
    T *stx = _sl(parent,s);

    Structure st = _d_get_symbol_structure(defs.symbols,s);
    if (!(is_sys_structure(st))) {
        T *structure = _d_get_structure_def(defs.structures,st);
        T *parts = _t_child(structure,2);
        int i,c = _t_children(parts);
        if (c > 0) {
            T *seq = _t_newr(stx,SEMTREX_SEQUENCE);
            for(i=1;i<=c;i++) {
                T *p = _t_child(parts,i);
                _d_build_def_semtrex(defs,*(Symbol *)_t_surface(p),seq);
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
char * __t2s(Defs *defs,T *t,int indent) {
    static char buf[100000];
    buf[0] = 0;
    return __t_dump(defs,t,indent,buf);
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

char * __t_dump(Defs *defs,T *t,int level,char *buf) {
    T *structures = defs ? defs->structures : 0;
    T *symbols = defs ? defs->symbols : 0;
    T *processes = defs ? defs->processes : 0;
    if (!t) return "";
    Symbol s = _t_symbol(t);
    char b[255];
    char tbuf[2000];
    int i;
    char *c;
    Xaddr x;
    buf = _indent_line(level,buf);

    if (is_process(s)) {
        sprintf(buf,"(process:%s",_d_get_process_name(processes,s));
    }
    else {
        char *n = _d_get_symbol_name(symbols,s);
        Structure st = _d_get_symbol_structure(symbols,s);
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
                c = _d_get_symbol_name(symbols,*(Symbol *)_t_surface(t));
                sprintf(buf,"(%s:%s",n,c?c:"<unknown>");
                break;
            case STRUCTURE_ID:
                c = _d_get_structure_name(structures,*(Structure *)_t_surface(t));
                sprintf(buf,"(%s:%s",n,c?c:"<unknown>");
                break;
            case PROCESS_ID:
                c = _d_get_process_name(processes,*(Process *)_t_surface(t));
                sprintf(buf,"(%s:%s",n,c?c:"<unknown>");
                break;
            case TREE_PATH_ID:
                sprintf(buf,"(%s:%s",n,_t_sprint_path((int *)_t_surface(t),b));
                break;
            case XADDR_ID:
                x = *(Xaddr *)_t_surface(t);
                sprintf(buf,"(%s:%s.%d",n,_d_get_symbol_name(symbols,x.symbol),x.addr);
                break;
            case STREAM_ID:
                sprintf(buf,"(%s:%p",n,_t_surface(t));
                break;
            case TREE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_TREE) {
                    c = __t_dump(defs,(T *)_t_surface(t),0,tbuf);
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
            case RECEPTOR_ID:
                if (t->context.flags & (TFLAG_SURFACE_IS_TREE+TFLAG_SURFACE_IS_RECEPTOR)) {
                    c = __t_dump(defs,((Receptor *)_t_surface(t))->root,0,tbuf);
                    sprintf(buf,"(%s:{%s}",n,c);
                    break;
                }
            case SCAPE_ID:
                if (t->context.flags & TFLAG_SURFACE_IS_SCAPE) {
                    Scape *sc = (Scape *)_t_surface(t);
                    sprintf(buf,"(%s:key %s,data %s",n,_d_get_symbol_name(symbols,sc->key_source),_d_get_symbol_name(symbols,sc->data_source));
                    break;
                }
            default:
                if (semeq(s,SEMTREX_MATCH_CURSOR)) {
                    c = __t_dump(defs,*(T **)_t_surface(t),0,tbuf);
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
    DO_KIDS(t,__t_dump(defs,_t_child(t,i),level < 0 ? level-1 : level+1,buf+strlen(buf)));
    sprintf(buf+strlen(buf),")");
    return buf;
}

/** @}*/
