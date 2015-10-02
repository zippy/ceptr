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
    var _SYSDEFS = {};
    var LABEL_TABLE = {
        "integer": {type:'structure'},
        "float": {type:'structure'},
        "shoe size": {type:'symbol',structure:'float'},
        "street number": {type:'symbol',structure:'integer'},
        "latitude": {type:'symbol',structure:'float'},
        "longitude": {type:'symbol',structure:'float'},
        "latlong": {type:'structure',symbols:['latitude','longitude']},
        "home location": {type:'symbol',structure:'latlong'}
    };
    var SYMBOLS = [];

    var _ = function (elem,o) {

        // instance variables
        var me = this;
        var handle_keys = true;

        // methods
        this.showCaret = function() {
            handle_keys=true;
            caret.style.display="inline-block";
            caret.focus();
        }
        this.hideCaret = function() {
            handle_keys=false;
            caret.style.display="none";
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
    };

    // Private functions

    function setupSem(label_val,sem,lock) {
        var def = LABEL_TABLE[label_val];
        if (def.type == 'symbol') {
            var label = $.getOrCreate('label',sem);
            if (lock) label.setAttribute("locked","true");
            label.innerHTML=label_val;
            var struct = $.getOrCreate('struct',sem);
            struct.innerHTML=def.structure;
            var symbols = LABEL_TABLE[def.structure].symbols;
            // if structure has no symbols it's system defined so create a surface
            // or an unknown treenode in the case of LIST or TREE structures
            if (!symbols) {
                if (def.structure == "TREE" || def.structure == "LIST") {
                    var children = $.getOrCreate('children',sem);
                    children.innerHTML="<sem><label>--unknown--</label></sem>";
                }
                else {
                    $.remove('children',sem);
                    var surface = $.getOrCreate('surface',sem);
                    surface.innerHTML = "";
                    surface.setAttribute('contenteditable', 'true');
                }
            }
            else {
                $.remove('surface',sem);
                var children = $.getOrCreate('children',sem);
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
        label.style.display="none";
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
            label.style.display="inline-block";
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

    var SEM_TYPE_STRUCTURE=1;
    var SEM_TYPE_SYMBOL=2;
    var SEM_TYPE_PROCESS=3;
    var type_names= ["","STRUCTURE","SYMBOL","PROCESS"];

    function getSemName(id){
        if (id.id == 0) {
            return "NULL_"+type_names[id.type];
        }
        return _SYSDEFS.children[id.type-1].children[id.id-1].children[0].surface;
    }
    _.getSemName = getSemName;
    function getSymbolStruct(id){
        if (id.type != SEM_TYPE_SYMBOL) {throw "expecting symbol";}
        return _SYSDEFS.children[SEM_TYPE_SYMBOL-1].children[id.id-1].children[1].surface;
    }
    _.getSymbolStruct = getSymbolStruct;
    function getStructSymbols(id){
        if (id.type != SEM_TYPE_STRUCTURE) {throw "expecting structure";}
        return _SYSDEFS.children[SEM_TYPE_STRUCTURE-1].children[id.id-1].children[1].children;
    }
    _.getStructSymbols = getStructSymbols;
    // Initialization
    function init() {
	$$(".TE").forEach(function (elem) {
	    new _(elem);
	});
        loadTree("sysdefs",function(d){
            _SYSDEFS = d;
            _.DEFS = _SYSDEFS;

            var struct_defs = _SYSDEFS.children[SEM_TYPE_STRUCTURE-1].children;
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
                var def = {type:'structure'};
                if (structures.length>0) def.symbols = structures;
                LABEL_TABLE[struct_defs[i].children[0].surface] = def;
            }

            var symbol_defs = _SYSDEFS.children[SEM_TYPE_SYMBOL-1].children;
            var symbols = symbol_defs.length;
            for (var i=0;i<symbols;i++) {
                var struct = symbol_defs[i].children[1].surface;
                LABEL_TABLE[symbol_defs[i].children[0].surface] = {type:'symbol',structure:getSemName(struct)};
            }
            for (var key in LABEL_TABLE) {
                var i = LABEL_TABLE[key];
                if (i.type == 'symbol') {SYMBOLS.push(key);}
            }
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

    _.buildLabelTable = function(elem) {
        var h = "";
        for (var key in LABEL_TABLE) {
            var i = LABEL_TABLE[key];
            h += "<ul>";
            h += "<ltlabel>"+key+"</ltlabel> <lttype>"+i.type+"</lttype>";
            if (i.type == "structure") {
                if (i.symbols) {
                    h += "<ltsymbols>" + i.symbols.join(",")+ "</ltsymbols>"
                }
            }
            h += "</ul>";
        }
        elem.innerHTML=h;

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
