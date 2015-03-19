var treeData = [
    {
        "name": "Top Level",
        "parent": "null",
        "children": [
            {
                "name": "COWS 2: A",
                //     "parent": "Top Level",
                "children": [
                    {
                        "name": "Son of A",
                        //         "parent": "Level 2: A"
                    },
                    {
                        "name": "Daughter of A",
                        //         "parent": "Level 2: A"
                    }
                ]
            },
            {
                "name": "Level 2: B",
                //      "parent": "Top Level"
            }
        ]
    }
];

function set_root() {
    root = treeData[0];
    root.x0 = height / 2;
    root.y0 = 0;
};

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

var TFLAG_ALLOCATED=0x0001,TFLAG_SURFACE_IS_TREE=0x0002,TFLAG_SURFACE_IS_RECEPTOR = 0x0004,TFLAG_SURFACE_IS_SCAPE=0x0008,TFLAG_DELETED=0x0010;

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
