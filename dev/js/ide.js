function loadTree(file_name,completedFN) {
    var fn = file_name+".json";
    var tree;
    $.get(fn, {},function(data,status) {
        console.log("loading:"+fn);
        completedFN(data);
    }).fail(function(x) {
        console.log("failed to load:"+fn);
        console.log(JSON.stringify(x));
    });
}
var JQ = $;  //jquery if needed for anything complicated, trying to not have dependency on jq
(function () {
    var LABEL_TABLE = {};
    var SYMBOLS = [];
    var STRUCTURES = [];

    var _ = function (elem,o) {

        // instance variables
        var me = this;
        var handle_keys = true;

        // methods
        this.showCaret = function() {
            handle_keys=true;
            $.show(caret);
            caret.focus();
        }
        this.hideCaret = function() {
            handle_keys=false;
            $.hide(caret);
        }

	// instance properties
        this.elem = $(elem);
        this.elem.te = this;  // stick a copy of this in the element for reference

        o = o || {};
	configure.call(this, {
	    // add properties here testProperty: 314
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
            },
        });
        $.bind(this.elem, {
            "click": function(e) {
                if (e.target.nodeName == "LABEL" && e.target.getAttribute("locked") != "true")  {
                    editLabel.call(me,e.target,true);
                    e.preventDefault();
                }
                else if (e.target.nodeName == "SURFACE") {
                    // don't prevent default because we want the click to go through to
                    // the editable content of the surface
                    me.hideCaret();
                }
                else {
                    // add some way to move the caret to the current clicked point
                    me.showCaret();
                    e.preventDefault();
                }
            }
        });

        caret.focus();
    };
    _.prototype = {
        // add method function defs here...like: get fish() {return "cod";}
        insert: function(label,surface) {
            var n = $.create('sem',{inside:this.elem});
            setupSem(label,n,false);
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
        }
    };

    // Private functions

    function setupSem(label_val,sem_elem,lock) {
        var def = LABEL_TABLE[label_val];
        if (def.type == 'symbol') {
            var label = $.getOrCreate('label',sem_elem);
            if (lock) label.setAttribute("locked","true");
            semid=def.sem;
            sem_elem.setAttribute('semid',""+semid.ctx+'.'+semid.type+'.'+semid.id);
            label.innerHTML=label_val;
            var struct = $.getOrCreate('struct',sem_elem);
            struct.innerHTML=def.structure;
            var symbols = LABEL_TABLE[def.structure].symbols;
            // if structure has no symbols it's system defined so create a surface
            // or an unknown treenode in the case of LIST or TREE structures
            if (!symbols) {
                if (def.structure == "TREE" || def.structure == "LIST") {
                    var children = $.getOrCreate('children',sem_elem);
                    children.innerHTML="<sem><label>--unknown--</label></sem>";
                }
                else {
                    $.remove('children',sem_elem);
                    var surface = $.getOrCreate('surface',sem_elem);
                    surface.innerHTML = "";
                    surface.setAttribute('contenteditable', 'true');
                }
            }
            else {
                $.remove('surface',sem_elem);
                var children = $.getOrCreate('children',sem_elem);
                children.innerHTML = "";
                symbols.forEach(function(s){
                    var child_sem = $.create('sem',{inside:children});
                    setupSem(s,child_sem,true);
                });
            }
        }
    }

    function editLabel(label,select) {
        var v = label.innerHTML;
        $.hide(label);
        var input = $.create("input",{
            before:label
        });
        var a = new Awesomplete(input, {
	    list: SYMBOLS,
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
                setupSem(input.value,label.parentNode);
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
            "blur":close_a,
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
    }

    $.remove = function(tag,elem) {
        var e = $(tag,elem);
        if (e) e.remove();
    }

    $.hide = function(e) {
        var elem  = $(e);
        var display = elem.style.display;
        if (display !== "none") {
            elem.__TEolddisplay = display;
            elem.style.display="none";
        }
    }

    $.show = function(e) {
        var elem  = $(e);
        elem.style.display= elem.__TEolddisplay || "";
    }

    var SEM_TYPE_STRUCTURE=1;
    var SEM_TYPE_SYMBOL=2;
    var SEM_TYPE_PROCESS=3;
    var type_names= ["","STRUCTURE","SYMBOL","PROCESS"];
    var SYS_CONTEXT=0;
    var COMPOSITORY_CONTEXT=1;
    var LOCAL_CONTEXT=2;
    var TEST_CONTEXT=3;
    var RECEPTOR_CONTEXT=0xff;
    var CONTEXTS = [{},{},{
        // set up the defs tree for the local context (have to do this manually)
        sem:{ctx:0,type:2,id:1},
        children:[
            {sem:{ctx:0,type:2,id:2},children:[]},
            {sem:{ctx:0,type:2,id:7},children:[]},
            {sem:{ctx:0,type:2,id:61},children:[]}
        ]
    },{}];

    function getSemName(id){
        if (id.id == 0) {
            return "NULL_"+type_names[id.type];
        }
        return CONTEXTS[id.ctx].children[id.type-1].children[id.id-1].children[0].surface;
    }
    _.getSemName = getSemName;
    function getSymbolStruct(id){
        if (id.type != SEM_TYPE_SYMBOL) {throw "expecting symbol";}
        return CONTEXTS[id.ctx].children[SEM_TYPE_SYMBOL-1].children[id.id-1].children[1].surface;
    }
    _.getSymbolStruct = getSymbolStruct;
    function getStructSymbols(id){
        if (id.type != SEM_TYPE_STRUCTURE) {throw "expecting structure";}
        return CONTEXTS[id.ctx].children[SEM_TYPE_STRUCTURE-1].children[id.id-1].children[1].children;
    }
    _.getStructSymbols = getStructSymbols;

    function Tnew(parent,symbol,surface) {
        var n = {sem:symbol};
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
        var symbols = CONTEXTS[LOCAL_CONTEXT].children[SEM_TYPE_SYMBOL-1];
        var def = Tnew(symbols,LABEL_TABLE["SYMBOL_DECLARATION"].sem);
        Tnew(def,LABEL_TABLE["SYMBOL_LABEL"].sem,label);
        Tnew(def,LABEL_TABLE["SYMBOL_STRUCTURE"].sem,struct);
        var sem = {ctx:LOCAL_CONTEXT,type:SEM_TYPE_SYMBOL,id:symbols.children.length};
        LABEL_TABLE[label]= {"sem":sem,type:'symbol',structure:getSemName(struct)};
        SYMBOLS.push(label);
        if (_.lt_elem) {
            var lt = makeLabelTableElem(label,LABEL_TABLE[label]);
            _.lt_elem.insertBefore(lt,_.lt_elem.firstChild)
        }
        return sem;
    }

    function newStructure(label,symbols) {
        var structures = CONTEXTS[LOCAL_CONTEXT].children[SEM_TYPE_STRUCTURE-1];
        var def = Tnew(structures,LABEL_TABLE["STRUCTURE_DEFINITION"].sem);
        Tnew(def,LABEL_TABLE["STRUCTURE_LABEL"].sem,label);
        var parts = Tnew(def,LABEL_TABLE["STRUCTURE_PARTS"].sem);
        var sem = {ctx:LOCAL_CONTEXT,type:SEM_TYPE_STRUCTURE,id:structures.children.length};
        var s = [];
        for(var i=0;i<symbols.length;i++) {
            Tnew(parts,LABEL_TABLE["STRUCTURE_PART"].sem,symbols[i]);
            s.push(getSemName(symbols[i]));
        }
        LABEL_TABLE[label]= {"sem":sem,type:'structure',symbols:s};
        STRUCTURES.push(label);
        if (_.lt_elem) {
            var lt = makeLabelTableElem(label,LABEL_TABLE[label]);
            _.lt_elem.insertBefore(lt,_.lt_elem.firstChild)
        }
        return sem;
    }

    // Initialization
    function init() {
	$$(".TE").forEach(function (elem) {
	    new _(elem);
	});
        loadTree("sysdefs",function(d){
            CONTEXTS[SYS_CONTEXT] = d;
            var struct_defs = d.children[SEM_TYPE_STRUCTURE-1].children;
            var structs = struct_defs.length;
            for (var i=0;i<structs;i++) {
                var symbols = struct_defs[i].children[1].children;
                var structures = [];
                for (var j=0;j<symbols.length;j++) {
                    var n = getSemName(symbols[j].surface);
                    if (n != "NULL_SYMBOL") {
                        structures.push(n);
                    }
                }
                var def = {sem:{ctx:SYS_CONTEXT,type:SEM_TYPE_STRUCTURE,id:i+1},type:'structure'};
                if (structures.length>0) def.symbols = structures;
                LABEL_TABLE[struct_defs[i].children[0].surface] = def;
            }

            var symbol_defs = d.children[SEM_TYPE_SYMBOL-1].children;
            var symbols = symbol_defs.length;
            for (var i=0;i<symbols;i++) {
                var struct = symbol_defs[i].children[1].surface;
                LABEL_TABLE[symbol_defs[i].children[0].surface] = {
                    sem:{ctx:SYS_CONTEXT,type:SEM_TYPE_SYMBOL,id:i+1},
                    type:'symbol',structure:getSemName(struct)
                };
            }
            for (var key in LABEL_TABLE) {
                var i = LABEL_TABLE[key];
                if (i.type == 'symbol') {SYMBOLS.push(key);}
                if (i.type == 'structure') {STRUCTURES.push(key);}
            }
            _.DEFS = d;
            var evt = new CustomEvent('sysdefsloaded');
            alert('dispatching');

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
                new Awesomplete($('#newsem-symstruct'), {
	            list: STRUCTURES,
                    minChars: 1
                });
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
            }
        });
        $.bind($('#newsem-morestructsym'), {
            "click": function(e) {
                var i=1;
                var ss;
                while (ss=$('#newsem-structsym'+i)) i++;
                var e = $.create('input', {id:'newsem-structsym'+i});
                var b = $('#newsem-morestructsym');
                new Awesomplete(b.parentNode.insertBefore(e,b),{list:SYMBOLS,minChars:1});
            }
        });

        $.bind($('#newsem-ok'), {
            "click": function(e) {
                switch($('#newsem-type').value) {
                case "symbol":
                    var s = $('#newsem-symstruct').value;
                    if (!LABEL_TABLE[s]) {
                        alert("unknown structure:"+s);
                        return;
                    }
                    var label = $('#newsem-symlabel').value;
                    newSymbol(label,LABEL_TABLE[s].sem);
                    $('#newsem-symlabel').value = "";
                    $('#newsem-symstruct').value = "";
                    break;
                case "structure":
                    var symbols = [];
                    var i=1;
                    var ss;
                    while (ss=$('#newsem-structsym'+i)) {
                        i++;
                        var s =ss.value;
                        if (!LABEL_TABLE[s]) {
                            alert("unknown symbol:"+s);
                            return;
                        }
                        symbols.push(LABEL_TABLE[s].sem);
                    }
                    var label = $('#newsem-structlabel').value;
                    newStructure(label,symbols);
                    $('#newsem-structlabel').value = "";
                    var i=2;var ss;
                    while (ss=$('#newsem-structsym'+i)) {i++;ss.remove()};
                    $('#newsem-structsym1').value="";
                    break;
                }
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

    }
    function makeLabelTableElem(key,i) {
        var e = $.create('ul');
        var h = "<ltlabel>"+key+"</ltlabel> <lttype>"+i.type+"</lttype>";
        if (i.type == "structure") {
            if (i.symbols) {
                h += "<ltsymbols>" + i.symbols.join(",")+ "</ltsymbols>"
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


function init() {

    $("#tree-editor").on('click','#x',function(e){x=false;});
    $("#tree-editor").on('blur','#x',function(e){
        x=true;
        var v = $(this).val();
        $(".awesomplete").replaceWith("<label>"+v+"</label>");
    });
    var x = false;
    $(window).bind('keypress', function(e) {
        if (!x) return;
        var code = (e.keyCode ? e.keyCode : e.which);
        if(code == 13) {
            alert("Fish");
            //Enter keycode
            //Do something
        }
        else {
            $("#caret").before(String.fromCharCode(code));
            event.preventDefault();
        }
    }).on('keydown', function(e) {
        if (!x) return;
        if (e.keyCode==8) {
            event.preventDefault();
            caretElemBefore().remove();
        }
        else if (e.keyCode==37) {
            event.preventDefault();
            x = caretElemBefore().remove();
            $("#caret").after($(x));
        }
        else if (e.keyCode==39) {
            event.preventDefault();
            x = caretElemAfter().remove();
            $("#caret").before($(x));
        }
            //alert(e.keyCode);
    });
    $("#tree-editor").on('click','label',function(e){
        var v = this.innerHTML;
        $(this).replaceWith("<input id=\"x\" class=\"awesomplete\">");
        var input = document.getElementById("x");
        new Awesomplete(input, {
	    list: SYMBOLS,
            minChars: 1
        });
        input.value=v;
        input.focus();
        input.select();
        x=false;
    });
}


function caretElemAfter() {
    return $("#caret").map(function(){return this.nextSibling;});
}
function caretElemBefore() {
    return $("#caret").map(function(){return this.previousSibling;});
}

// from https://gist.github.com/shadybones/9816763
function createClass(name,rules){
    var style = document.createElement('style');
    style.type = 'text/css';
    document.getElementsByTagName('head')[0].appendChild(style);
    if(!(style.sheet||{}).insertRule)
        (style.styleSheet || style.sheet).addRule(name, rules);
    else
        style.sheet.insertRule(name+"{"+rules+"}",0);
}

function applyClass(name,element,doRemove){
    if(typeof element.valueOf() == "string"){
        element = document.getElementById(element);
    }
    if(!element) return;
    if(doRemove){
        element.className = element.className.replace(new RegExp("\\b" + name + "\\b","g"));
    }else{
        element.className = element.className + " " + name;
    }
}
