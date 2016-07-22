function addParents(tree,parent) {
    tree.parent = parent;
    if (tree.children) {
        tree.children.forEach(function(child){
            addParents(child,tree);
        });
    }
}

function loadTree(file_name,completedFN) {
    var fn = file_name+".json";
    var tree;
    $.get(fn, {},function(data,status) {
        console.log("loading:"+fn);
        addParents(data);
        completedFN(data);
    }).fail(function(x) {
        console.log("failed to load:"+fn);
        console.log(JSON.stringify(x));
    });
}
function getSymbolByID(id) {
    Object.keys(TE.LABEL_TABLE).forEach(function(k) {var sem = TE.LABEL_TABLE[k].sem; if (sem.id == id &&sem.type==2) console.log(k);});
}
var JQ = $;  //jquery if needed for anything complicated, trying to not have dependency on jq
(function () {
    var LABEL_TABLE = {};
    var SYMBOLS = [];
    var STRUCTURES = [];
    var PROCESSES = [];
    var PROTOCOLS = [];

    var _ = function (elem,o) {

        // instance variables
        var me = this;
        var handle_keys = true;

        // instance properties
        this.elem = $(elem);
        this.elem.te = this;  // stick a copy of this in the element for reference

        o = o || {};
        configure.call(this, {
            structVisible : false
        }, o);

        // Create necessary elements
        var caret = $.create("span",{
            id:"caret",
            className: "blinking-cursor",
            contenteditable:true,
            innerHTML: "",
            inside: this.elem
        });
        this.caret = caret;

        // Bind events
        $.bind(caret, {
            "keypress": function(e) {
                if (!handle_keys) {
                    return;
                }
                var code = (e.keyCode ? e.keyCode : e.which);
                {
                    var c = String.fromCharCode(code);
                    if (/^[a-zA-Z_-]$/.test(c)) {
                        var new_sem = $.create("sem",{before: caret});
                        var new_label = $.create("label",{inside:new_sem});
                        var a = editLabel.call(me,new_label,false);
                        a.input.value=c;
                        a.evaluate();
                    }
                }
                e.preventDefault();
            },
            "keydown": function(e) {
                if (!handle_keys) {return;}
                if (e.keyCode==37) { // left arrow
                    var ref = caret;
                    ref.parentNode.insertBefore(caret,ref.previousElementSibling);
                    caret.focus();
                    e.preventDefault();
                }
                else if (e.keyCode==39) { //right arrow
                    var ref = caret.nextElementSibling;
                    ref.parentNode.insertBefore(caret,ref.nextElementSibling);
                    caret.focus();
                    e.preventDefault();
                }
            }
        });
        $.bind(this.elem, {
            "click": function(e) {
                if (e.target.nodeName == "LABEL")  {
                    if (e.target.getAttribute("locked") != "true") {
                        editLabel.call(me,e.target,true);
                        e.preventDefault();
                    }
                }
                else if ((e.target.nodeName == "SURFACE")||(e.target.nodeName == "INPUT")) {
                    // don't prevent default because we want the click to go through to
                    // the editable content of the surface
                    me.hideCaret();
                }
                else {
                    // @todo add some way to move the caret to the current clicked point
                    me.showCaret();
                    e.preventDefault();
                }
            }
        });

        caret.focus();
    };
    _.prototype = {
        showCaret : function() {
            handle_keys=true;
            $.show(this.caret);
            this.caret.focus();
        },
        hideCaret : function() {
            handle_keys=false;
            $.hide(this.caret);
        },
        // change the visibility of the Structure tags
        toggleStructVisibility: function() {
            var vis = this.structVisible = !this.structVisible;
            $$('struct',this.elem).forEach(function(s) {
                vis ? $.show(s) : $.hide(s);

            });
        },
        // insert a node at the current cursor position
        insert: function(label,surface) {
            var n = $.create('sem',{inside:this.elem});
            setupSem.call(this,label,n,false,getParentSem(n));
            if (surface) {
                if (Array.isArray(surface)) {
                    var i;
                    var s = $$('surface',n);
                    for (i=0;i<surface.length;i++) {
                        s[i].innerHTML = surface[i];
                    }
                }
                else {
                    $('surface',n).innerHTML = surface;
                }
            }
            return n;
        },
        // insert a full semantic tree at the current cursor position
        insertTree: function(tree) {
            _insertTree.call(this,tree,this.elem);
        }
    };

    // Private functions

    function _setupSem(sem,sem_elem,lock,parent_sem) {
        sem_elem.setAttribute('semid',sem.ctx+'.'+sem.type+'.'+sem.id);
        sem_elem.setAttribute("type",type_names[sem.type].toLowerCase());
        if (!parent_sem || parent_sem.type != sem.type) {
            sem_elem.setAttribute("class","type-changed");
        }
        var label = $.getOrCreate('label',sem_elem);
        if (lock) label.setAttribute("locked","true");
        label.innerHTML=getSemName(sem);
        if (sem.type == SEM_TYPE_SYMBOL) {
            var struct = $.getOrCreate('struct',sem_elem);
            this.structVisible ? $.show(struct) : $.hide(struct);
            struct.innerHTML=getSemName(getSymbolStruct(sem));
            label.setAttribute("title",struct.innerHTML);
        }
    }

    function _insertTree(tree,elem) {
        var me = this;
        var s = $.create('sem',{inside:elem});
        _setupSem.call(this,tree.sem,s,true,tree.parent ? tree.parent.sem : undefined);

        if (tree.children) {
            tree.children.forEach(function(child){
                _insertTree.call(me,child,s);
            });
        }
        if (tree.surface) {
            var surface = $.getOrCreate('surface',s);
            if (typeof tree.surface === "object") {
                var o = tree.surface;
                //@todo implement general way to represent base objects here

                // this represents a SemanticID by it's name:
                if (o.hasOwnProperty("ctx") && o.hasOwnProperty("id") && o.hasOwnProperty("type")) {
                    surface.innerHTML = "&lt;"+getSemName(tree.surface)+"&gt;";
                }
                else {
                    surface.innerHTML = "some-object";
                }
            }
            else {
                surface.innerHTML = tree.surface;
            }
        }
    }

    // given a semantic element label, create a semtree of that
    // type according to its structure.
    function setupSem(label_val,sem_elem,lock,parent_sem) {
        // for now we are getting the def from the label table,
        // we should probably get it from DEFS?  maybe not because it's slower
        var def = LABEL_TABLE[label_val];
        var me = this;
        var sem = def.sem;
        _setupSem.call(this,sem,sem_elem,lock,parent_sem);
        if (sem.type === SEM_TYPE_PROCESS) {
            var params = LABEL_TABLE[label_val].params;
            if (params) {
                $.remove('surface',sem_elem);
                var children = $.getOrCreate('children',sem_elem);
                var h = "";
                params.forEach(function(param){
                    console.log(param);
                    h += "<sem><sig>"+param+"</sig><label>--unknown--</label></sem>";
                });
                children.innerHTML = h;
            }
        }
        else if (sem.type === SEM_TYPE_SYMBOL) {
            var sdef = LABEL_TABLE[def.structure].def;
            var symbols;
            var structure_type;
            // the simple cases are just symbols or sequences of symbols
            if (getSemName(sdef.sem) === "STRUCTURE_SYMBOL") {
                var s_name = getSemName(sdef.surface);
                if (s_name === "NULL_SYMBOL") {
                    structure_type = "base";
                }
                else {
                    symbols = [getSemName(sdef.surface)];
                    structure_type = "fixed";
                }
            }
            else if (getSemName(sdef.sem) === "STRUCTURE_SEQUENCE") {
                var s = [];
                structure_type = "fixed";
                sdef.children.forEach(function(c){
                    if (getSemName(c.sem) === "STRUCTURE_SYMBOL") {
                        s.push(getSemName(c.surface));
                    }
                    else {structure_type = "complex";};
                });
                if (structure_type === "fixed") symbols = s;
            }
            else {
                structure_type = "complex";
            }

            if (structure_type == "base") {
                $.remove('children',sem_elem);
                var surface = $.getOrCreate('surface',sem_elem);
                surface.innerHTML = "";
                surface.setAttribute('contenteditable', 'true');
            }
            else if (structure_type === "fixed") {
                $.remove('surface',sem_elem);
                var children = $.getOrCreate('children',sem_elem);
                children.innerHTML = "";
                symbols.forEach(function(s){
                    var child_sem = $.create('sem',{inside:children});
                    setupSem.call(me,s,child_sem,true,sem);
                });
            } else if (structure_type === "complex") {
                var children = $.getOrCreate('children',sem_elem);
                children.innerHTML="<sem><label>--unknown--</label></sem>";
            }
        }
    }

    // given a sem tree element return the sem id of the the parent node
    function getParentSem(sem_elem) {
        var parent_sem_id = sem_elem.parentNode.parentNode.getAttribute("semid");
        var parent_sem;
        if (parent_sem_id) {
            parent_sem_id = parent_sem_id.split('.');
            parent_sem = {};
            parent_sem.ctx = parent_sem_id[0];
            parent_sem.type = parent_sem_id[1];
            parent_sem.ctx = parent_sem_id[2];
        }
        return parent_sem;
    }

    // given the label element of a semtree element this pop up an awesomeplete
    // selectlist, and if the label changed, rebuild the internals of the semantic
    // element according to the new symbol type.
    function editLabel(label,select) {
        var v = label.innerHTML;
        $.hide(label);
        var input = $.create("input",{
            before:label
        });
        var a = new Awesomplete(input, {
            list: SYMBOLS.concat(PROCESSES),
            minChars: 1
        });
        input.value=v;
        input.focus();
        if (select) input.select();
        var me = this;
        this.hideCaret();
        function close_a(e) {
            //if the new label is defined and different reset everything
            if (LABEL_TABLE[input.value] != undefined && label.innerHTML != input.value) {
                var parent_sem = getParentSem(label.parentNode);
                setupSem.call(me,input.value,label.parentNode,false,parent_sem);
            }
            $.show(label);
            var p = input.parentNode;
            if (p.parentElement != null) {p.remove();}

            // move caret to after this node
            var ref = label.parentNode;
            ref.parentNode.insertBefore(me.caret,ref.nextElementSibling);
            me.showCaret();
        }
        $.bind(input,{
            "blur":close_a
        });
        return a;
    }

    function configure(properties, o) {
        for (var i in properties) {
            var initial = properties[i],
                attrValue = this.elem.getAttribute("data-" + i.toLowerCase());

            if (typeof initial === "number") {
                this[i] = parseInt(attrValue);
            }
            else if (initial === false) { // Boolean options must be false by default anyway
                this[i] = attrValue !== null;
            }
            else if (initial instanceof Function) {
                this[i] = null;
            }
            else {
                this[i] = attrValue;
            }

            if (!this[i] && this[i] !== 0) {
                this[i] = (i in o)? o[i] : initial;
            }
        }
    }

    // Helpers

    var slice = Array.prototype.slice;

    function $(expr, con) {
        return typeof expr === "string"? (con || document).querySelector(expr) : expr || null;
    }

    function $$(expr, con) {
        return slice.call((con || document).querySelectorAll(expr));
    }

    $.bind = function(element, o) {
        if (element) {
            for (var event in o) {
                var callback = o[event];

                event.split(/\s+/).forEach(function (event) {
                    element.addEventListener(event, callback);
                });
            }
        }
    };

    $.create = function(tag, o) {
        var element = document.createElement(tag);

        for (var i in o) {
            var val = o[i];

            if (i === "inside") {
                $(val).appendChild(element);
            }
            else if (i === "before") {
                var ref = $(val);
                ref.parentNode.insertBefore(element, ref);
            }
            else if (i === "around") {
                var ref = $(val);
                ref.parentNode.insertBefore(element, ref);
                element.appendChild(ref);
            }
            else if (i in element) {
                element[i] = val;
            }
            else {
                element.setAttribute(i, val);
            }
        }

        return element;
    };

    $.getOrCreate = function(tag,elem) {
        var e = $(tag,elem);
        if (!e) {
            e = $.create(tag,{inside:elem});
        }
        return e;
    };

    $.remove = function(tag,elem) {
        var e = $(tag,elem);
        if (e) e.remove();
    };

    $.hide = function(e) {
        var elem  = $(e);
        var display = elem.style.display;
        if (display !== "none") {
            elem.__TEolddisplay = display;
            elem.style.display="none";
        }
    };

    $.show = function(e) {
        var elem  = $(e);
        elem.style.display= elem.__TEolddisplay || "";
    };

    var SEM_TYPE_STRUCTURE=1;
    var SEM_TYPE_SYMBOL=2;
    var SEM_TYPE_PROCESS=3;
    var SEM_TYPE_RECEPTOR=4;
    var SEM_TYPE_PROTOCOL=5;
    var type_names= ["","STRUCTURE","SYMBOL","PROCESS","RECEPTOR","PROTOCOL"];
    var SYS_CONTEXT=0;
    var COMPOSITORY_CONTEXT=1;
    var DEV_COMPOSITORY_CONTEXT=2;
    var TEST_CONTEXT=3;
    var CLOCK_CONTEXT=4;
    var STREAM_READER_CONTEXT=5;
    var STREAM_WRITER_CONTEXT=6;

    var CONTEXTS = [];
    var Context_count = 0;

    function getSemName(id){
        if (id.id == 0) {
            return "NULL_"+type_names[id.type];
        }
        return CONTEXTS[id.ctx].children[id.type-1].children[id.id-1].children[0].children[0].surface;
    }
    _.getSemName = getSemName;
    function getSymbolStruct(id){
        if (id.type != SEM_TYPE_SYMBOL) {throw "expecting symbol";}
        if (id.id == 0) return {ctx:0,type:SEM_TYPE_STRUCTURE,id:0};
        return CONTEXTS[id.ctx].children[SEM_TYPE_SYMBOL-1].children[id.id-1].children[1].surface;
    }
    _.getSymbolStruct = getSymbolStruct;
    function getStructSymbols(id){
        if (id.type != SEM_TYPE_STRUCTURE) {throw "expecting structure";}
        return CONTEXTS[id.ctx].children[SEM_TYPE_STRUCTURE-1].children[id.id-1].children[1].children;
    }
    _.getStructSymbols = getStructSymbols;

    function Tnew(parent,symbol,surface) {
        var n = {sem:symbol,parent:parent};
        if (surface) n.surface = surface;
        if (!parent.children) {
            parent.children = [n];
        }
        else {
            parent.children.push(n);
        }
        return n;
    }

    function newSymbol(label,struct) {
        var symbols = CONTEXTS[DEV_COMPOSITORY_CONTEXT].children[SEM_TYPE_SYMBOL-1];
        var def = Tnew(symbols,LABEL_TABLE["SYMBOL_DEFINITION"].sem);
        var sym_label = Tnew(def,LABEL_TABLE["SYMBOL_LABEL"].sem);
        Tnew(sym_label,LABEL_TABLE["ENGLISH_LABEL"].sem,label);
        Tnew(def,LABEL_TABLE["SYMBOL_STRUCTURE"].sem,struct);
        var sem = {ctx:DEV_COMPOSITORY_CONTEXT,type:SEM_TYPE_SYMBOL,id:symbols.children.length};
        LABEL_TABLE[label]= {"sem":sem,type:'symbol',structure:getSemName(struct)};
        SYMBOLS.push(label);
        if (_.lt_elem) {
            var lt = makeLabelTableElem(label,LABEL_TABLE[label]);
            _.lt_elem.insertBefore(lt,_.lt_elem.firstChild);
        }
        return sem;
    }

    function getSemIDText(label) {
        var sem = LABEL_TABLE[label].sem;
        return sem.ctx+'.'+sem.type+'.'+sem.id;
    }

    function newStructure(label,symbols) {
        var structures = CONTEXTS[DEV_COMPOSITORY_CONTEXT].children[SEM_TYPE_STRUCTURE-1];
        var def = Tnew(structures,LABEL_TABLE["STRUCTURE_DEFINITION"].sem);
        var st_label = Tnew(def,LABEL_TABLE["STRUCTURE_LABEL"].sem);
        Tnew(st_label,LABEL_TABLE["ENGLISH_LABEL"].sem,label);
        var parts = Tnew(def,LABEL_TABLE["STRUCTURE_SEQUENCE"].sem);
        var sem = {ctx:DEV_COMPOSITORY_CONTEXT,type:SEM_TYPE_STRUCTURE,id:structures.children.length};
        var s = [];
        for(var i=0;i<symbols.length;i++) {
            Tnew(parts,LABEL_TABLE["STRUCTURE_SYMBOL"].sem,symbols[i]);
            s.push(getSemName(symbols[i]));
        }
        LABEL_TABLE[label]= {"sem":sem,type:'structure',symbols:s,def:def.children[1]};
        STRUCTURES.push(label);
        if (_.lt_elem) {
            var lt = makeLabelTableElem(label,LABEL_TABLE[label]);
            _.lt_elem.insertBefore(lt,_.lt_elem.firstChild);
        }
        return sem;
    }

    // add defs to the label table
    function _doDefs(context,type,func) {
        var defs = CONTEXTS[context].children[type-1].children;
        if (defs) {
            var num_defs = defs.length;
            for (var i=0;i<num_defs;i++) {
                // get the label from the first element which is the default.
                var label = defs[i].children[0].children[0].surface;
                var lt_entry = func(i,defs[i]);
                LABEL_TABLE[label] = lt_entry;
            }
        }
    }

    function setupContext(context,defs) {
        CONTEXTS[context] = defs;
        _doDefs(context,SEM_TYPE_RECEPTOR,function(i,receptor_def){
            Context_count++;
            setupContext(Context_count,receptor_def.children[1]);
            return {
                sem:{ctx:context,type:SEM_TYPE_RECEPTOR,id:i+1},
                type:'receptor'
                // @todo add defs for label table here.
            };
        });
        _doDefs(context,SEM_TYPE_STRUCTURE,function(i,struct_def){
            var def = {sem:{ctx:context,type:SEM_TYPE_STRUCTURE,id:i+1},
                       type:'structure',
                       def:struct_def.children[1]
                      };
            return def;
        });
        _doDefs(context,SEM_TYPE_SYMBOL,function(i,symbol_def){
            var struct = symbol_def.children[1].surface;
            return {
                sem:{ctx:context,type:SEM_TYPE_SYMBOL,id:i+1},
                type:'symbol',
                structure:getSemName(struct)
            };
        });
        _doDefs(context,SEM_TYPE_PROCESS,function(i,process_def){
            var def = {
                sem:{ctx:context,type:SEM_TYPE_PROCESS,id:i+1},
                type:'process'
                //                    intention:symbol_def.children[1].surface;
            };
            var signatures = process_def.children[3].children;
            var params = [];
            for (var j=1;j<signatures.length;j++) {
                var n = signatures[j].children[0].children[0].surface;
                params.push(n);
            }
            if (params.length>0) def.params = params;

            return def;
        });
        _doDefs(context,SEM_TYPE_PROTOCOL,function(i,protocol_def){
            return {
                sem:{ctx:context,type:SEM_TYPE_PROTOCOL,id:i+1},
                type:'protocol'
                // @todo add defs for label table here.
            };
        });
   }
    // Initialization
    function init() {
        $$(".TE").forEach(function (elem) {
            new _(elem);
        });
        loadTree("sysdefs",function(d){
            setupContext(SYS_CONTEXT,d);

            for (var key in LABEL_TABLE) {
                var i = LABEL_TABLE[key];
                if (i.type == 'symbol') {SYMBOLS.push(key);}
                if (i.type == 'structure') {STRUCTURES.push(key);}
                if (i.type == 'process') {PROCESSES.push(key);}
            }
            _.DEFS = d;
            var evt = new CustomEvent('sysdefsloaded');
            window.dispatchEvent(evt);
        });
    }

    // Are we in a browser? Check for Document constructor
    if (typeof Document !== 'undefined') {
        // DOM already loaded?
        if (document.readyState !== "loading") {
            init();
        }
        else {
            // Wait for it
            document.addEventListener("DOMContentLoaded", init);
        }
    }

    // static methods and variables
    //export some utilities
    _.$ = $;
    _.$$ = $$;
    _.LABEL_TABLE = LABEL_TABLE;

    // set up a label table element to work
    // requires that it have various buttons and such
    _.setLabelTableContainer = function(elem) {
        elem.innerHTML="";
        _.lt_elem = elem;

        //@todo clean these up, currently just here as some local examples...
        newSymbol("shoe size",LABEL_TABLE["FLOAT"].sem);
        newSymbol("street number",LABEL_TABLE["INTEGER"].sem);
        var lat = newSymbol("latitude",LABEL_TABLE["FLOAT"].sem);
        var lon = newSymbol("longitude",LABEL_TABLE["FLOAT"].sem);
        var ll = newStructure("latlong",[lat,lon]);
        newSymbol("home location",ll);

        buildLabelTable(elem);

        $.bind($('#newsem'), {
            "click": function(e) {
                $.hide($('#newsem'));
                $.show($('#newsem-form'));

                // create a SYMBOL_DEFINITION tree
                var elem = $.create('div',{inside:$('#newsem-sym'),className:'TE'});
                var x = new _(elem);
                var sem = x.insert('SYMBOL_DEFINITION');
                $('label',sem).setAttribute("locked","true");

                var s = $('.TE [semid="'+getSemIDText('SYMBOL_STRUCTURE')+'"] surface',elem);
                s.removeAttribute("contenteditable");
                //var i = $.create('input',{inside:s});
                // @todo for now manually use an awesomeplete/select list as the widget for the surface
                // when we implement widgets (#51) this should just work automatically

                var sel = $.create('select',{inside:s});
                STRUCTURES.forEach(function(s) {
                    var o = $.create('option',{inside:sel});
                    o.innerHTML=s.length >30 ? s.substring(0,30)+"...":s;
                    o.setAttribute("value",s);
                });
                // for some reason the awesomeplete didn't work
                // new Awesomplete(i, {
                //     list: STRUCTURES,
                //     minChars: 1
                // });

                //@todo, covert this to work similarly for defining structures where
                // it just works because you can just edit a STRUCTURE_DEFINITION element
                new Awesomplete($('#newsem-structsym1'), {
                    list: SYMBOLS,
                    minChars: 1
                });
            }
        });
        $.bind($('#newsem-cancel'), {
            "click": function(e) {
                $.show($('#newsem'));
                $.hide($('#newsem-form'));
                $('#newsem-sym .TE').remove();
            }
        });
        $.bind($('#newsem-morestructsym'), {
            "click": function(e) {
                var i=1;
                var ss;
                while (ss=$('#newsem-structsym'+i)) i++;
                var e = $.create('input', {id:'newsem-structsym'+i});
                var b = $('#newsem-morestructsym');
                debugger;
                new Awesomplete(b.parentNode.insertBefore(e,b),{list:SYMBOLS,minChars:1});
            }
        });

        $.bind($('#newsem-ok'), {
            "click": function(e) {
                var label,s;
                switch($('#newsem-type').value) {
                case "symbol":
                    s = $('#newsem-sym [semid="'+getSemIDText('SYMBOL_STRUCTURE')+'"] surface select').value;
                    if (!LABEL_TABLE[s]) {
                        alert("unknown structure:"+s);
                        return;
                    }

                    label = $('#newsem-sym [semid="'+getSemIDText('SYMBOL_LABEL')+'"] surface').innerHTML;
                    newSymbol(label,LABEL_TABLE[s].sem);
                    break;
                case "structure":
                    var symbols = [];
                    var i=1;
                    var ss;
                    while ((ss=$('#newsem-structsym'+i))) {
                        i++;
                        s =ss.value;
                        if (!LABEL_TABLE[s]) {
                            alert("unknown symbol:"+s);
                            return;
                        }
                        symbols.push(LABEL_TABLE[s].sem);
                    }
                    label = $('#newsem-structlabel').value;
                    newStructure(label,symbols);
                           $('#newsem-structlabel').value = "";
                    i=2;
                    while ((ss=$('#newsem-structsym'+i))) {
                        i++;ss.remove();};
                    $('#newsem-structsym1').value="";
                    break;
                }
                $('#newsem-sym .TE').remove();
                $.show($('#newsem'));
                $.hide($('#newsem-form'));

            }
        });
        $.bind($('#newsem-type'), {
            "click": function(e) {
                switch($('#newsem-type').value) {
                case "symbol":
                    $.show('#newsem-sym');$.hide('#newsem-struct');

                    break;
                case "structure": $.show('#newsem-struct');$.hide('#newsem-sym');break;
                }
            }
        });
    };
    function makeLabelTableElem(key,i) {
        var e = $.create('ul');
        e.setAttribute("type",i.type);
        var h = "<ltlabel>"+key+"</ltlabel>";
        if (i.type == "symbol") {
            h += "<ltstructure>" + i.structure+ "</ltstructure>";
        }
        if (i.type === "structure") {
            if (i.symbols) {
                h += "<ltsymbols>" + i.symbols.join(",")+ "</ltsymbols>";
            }
        }
        if (i.type == "process") {
            if (i.params) {
                h += "<ltparams>" + i.params.join(",")+ "</ltparams>";
            }
        }
        e.innerHTML=h;
        return e;
    }
    function buildLabelTable(elem) {
        elem.innerHTML="";
        var h = "";
        for (var key in LABEL_TABLE) {
            var i = LABEL_TABLE[key];
            elem.appendChild(makeLabelTableElem(key,i));
        }
    }
    // Make sure to export TE on self when in a browser
    if (typeof self !== 'undefined') {
        self.TE = _;
    }


    return _;

}());
