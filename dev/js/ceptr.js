/**
 * @defgroup js
 *
 * @brief Javascript tools for interfacing with ceptr
 *
 * @{
 * @file ceptr.js
 * @brief javascript
 *
 * @copyright Copyright (C) 2013-2015, The MetaCurrency Project (Eric Harris-Braun, Arthur Brock, et. al).  This file is part of the Ceptr platform and is released under the terms of the license contained in the file LICENSE (GPLv3).
 *
 */

// convert a string path like "/1/2/5/3" to an array
function p2a(path) {
    var p = path.split("/");
    p.shift();
    var r = [];
    p.forEach(function(x){r.push(parseInt(x))});
    return r;
}

// given a string path get the element out of the tree
function t_get(tree,path) {
    var p = p2a(path);
    return _t_get(tree,p);
}

// given an array path get the element out of the tree
function _t_get(tree,p) {
    while(p.length > 0) {tree = tree.children[p[0]-1];p.shift();}
    return tree;
}

// return an array path to the given node
// recursively searches back up the tree storing the path values as it goes
function _t_get_path(node,path) {
    if (!node.parent) {
        if (!path) return [];
        return path;
    }
    else {
        var i,c = node.parent.children.length;
        for (i=0;i<c;i++) {
            if (node.parent.children[i] == node) {
                if (path) path.unshift(i+1);
                else path = [i+1];
                return _t_get_path(node.parent,path);
            }
        }
        console.log("failed to find node in parent's children");
    }
}

// return a string path to the given node
function t_get_path(node) {
    var p = _t_get_path(node);
    return "/"+p.join("/");
}

// return the root of the tree the given node is a part of
function _t_root(node) {
    if (!node.parent) return node;
    return _t_root(node.parent);
}

// add a tree as the last child of the given tree
function t_add(t,n) {
    if (!t.children) t.children = [];
    t.children[t.children.length] = n;
}

// applies a function to a tree node and all it's children recursively
function t_apply(tree,f) {
    f(tree);
    var i,nodes = tree.children ? tree.children.length : 0;
    for(i = 0;i<nodes;i++) {
        t_apply(tree.children[i],f);
    }
}

// give a semtrex results tree and the source tree, hilight the matched portion
function t_stx_hilight(tree,stxresults,color) {
    var path = p2a(stxresults.children[1].surface);
    var count = stxresults.children[2].surface;
    while(count>0) {
        var pp = path.slice(0);
        t_apply(_t_get(tree,path),function(t){t.color = color});
        var p = pp.pop();
        p+=1;
        pp.push(p);
        path = pp;
        count-=1;
    }
    svg.selectAll("g.node text").style("fill",function(d){return d.color})
}

/////////////////////////////////////////////////////////////////////////////////////////////
////  mtree unserialization routines

var sizeof_S = 12;
var Mlevel_size = 2;
var Mindex_size = 4;
var Symbol_size = 8;
var size_t_size = 8;
var serialized_node_size = 32;

function serialized_header_size(levels) {
    return sizeof_S+4*levels;
}

function serialzed_level_size(nodes) {
    return Mindex_size+serialized_node_size*nodes;
}

function unserialize(serialzed_mtree) {
    var t = jDataView(serialzed_mtree);
    var magic = t.getUint32(0,true);
    var levels = t.getUint16(4,true);
    var blob_offset = t.getUint32(8,true);
    var r = [];
    var i,l;

    var s_size = serialized_header_size(levels);
    for (l =0;l<levels;l++) {
        var level_offset = s_size+t.getUint32(sizeof_S+l*4,true);
        var nodes = t.getUint32(level_offset,true);
        var c;
        if (!(c = r[l])) {c = r[l] = [];}
        var node_offset = level_offset+Mindex_size;
        for(i = 0;i<nodes;i++) {
            c[i] = {};
            var x = 0;
            c[i].symbol = {};
            c[i].symbol.context =  t.getUint16(node_offset+x,true); x+=2;
            c[i].symbol.flags =  t.getUint16(node_offset+x,true); x+=2;
            c[i].symbol.id =  t.getUint16(node_offset+x,true); x+=4;
            c[i].parenti = t.getUint32(node_offset+x,true); x+= Mindex_size;
            c[i].flags =   t.getUint32(node_offset+x,true); x+= 4;
            c[i].cur_child =   t.getUint32(node_offset+x,true); x+= 4;
            c[i].size =    t.getUint64(node_offset+x,true).valueOf(); x+=size_t_size;
            if (c[i].size == 4) {
                t.seek(node_offset+x);
                c[i].value = t.getString(4);
            }
            else {
                if (c[i].size) {
                    t.seek(blob_offset+t.getUint64(node_offset+x,true).valueOf());
                    c[i].value = t.getString(c[i].size);
                }
                else c[i].value = "";
            }
            node_offset += serialized_node_size;
        }
    }
    return r;
};

var TFLAG_ALLOCATED=0x0001,TFLAG_SURFACE_IS_TREE=0x0002,TFLAG_SURFACE_IS_RECEPTOR = 0x0004,TFLAG_SURFACE_IS_SCAPE=0x0008,TFLAG_SURFACE_IS_STREAM=0x0010,TFLAG_DELETED=0x0020,TFLAG_RUN_NODE=0x0040,TFLAG_REFERENCE=0x8000;

function getnode(mtree,l,i) {
    var level = mtree[l];
    return level ? level[i] : undefined;
}
var null_addr = 4294967295;
function walk(mtree,fn,data) {
    var levels = mtree.length;
    var state = [];
    var x;
    for (x=0;x<levels;x++) {state[x] = {};};
    var backup,i=0,l=0,done = false;
    var pi=null_addr,pl=null_addr;
    var nodes = i+1;
    var root = l;
    var level = mtree[root];
    while(!done) {
        backup = false;
        var n = getnode(mtree,l,i);
        // look for child of parent at this level (may be node at current handle address)
        while((i<nodes) && ((n.flags&TFLAG_DELETED) || n.parenti != pi )) {
            n=getnode(mtree,l,++i);
        }
        // if we got one, then call the walk function
        if (i != nodes) {
            fn(l,i,n,data,state,pl,pi);
            // and go down looking for children
            if (l+1 < levels) {
                state[l].i = i;
                pi = i;pl = l;
                l++;
                i = 0;
                level = mtree[l];
                nodes = level.length;
            }
            else {
                // if no more levels, then backup if no more nodes
                if (++i == nodes)
                    backup = true;
            }
        }
        else backup = true;
        while(backup) {
            // if current node is at root level we are done
            if (l == root) {backup = false;done = true;}
            else {
                // otherwise move up a level
                l--;
                // new node index is next node at that level from stored state
                i = state[l].i+1;
                level = mtree[l];
                nodes = level.length;
                // if we still have nodes to check then we have finished backing up otherwise
                // we'll loop to go back up another level
                if (i < nodes) {
                    backup = false;
                    pl = l -1;
                    pi = state[pl].i;
                }
            }
        }
    }
}

function  m_2tfn(l,i,n,data,state,pl,pi) {
    var t = l ? state[l-1].t : null;
    var x = {"value": n.value,"symbol":n.symbol.id};
    if (data.length == 0) {data.push(x);}
    if (t) {
        var c = t.children;
        if (!c) c = t.children = [];
        c.push(x);
    }
//        *tP = _t_new(t,n->symbol,(n->flags & TFLAG_ALLOCATED)?n->surface:&n->surface,n->size);
    state[l].t = x;
}
/** @}*/
