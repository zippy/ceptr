var LABEL_TABLE= {
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
for (var key in LABEL_TABLE) {
    var i = LABEL_TABLE[key];
    if (i.type == 'symbol') {SYMBOLS.push(key);}
    $("#label-table").append("<ltitem><ltlabel>"+key+"</ltlabel> <lttype>"+i.type+"</lttype></ltitem>");
}

function showLabelTable() {
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
    $("#label-table").html(h);

}

var JQ = $;  //jquery if needed for anything complicated, trying to not have dependency on jq
(function () {
    var _ = function (elem,o) {
        var me = this;
        var handle_keys = true;

	// Setup
        this.elem = $(elem);
        this.elem.te = this;

        o = o || {};
	configure.call(this, {
	    testProperty: 314
	}, o);

        // Create necessary elements
        this.caret = $.create("span",{
            id:"caret",
            className: "blinking-cursor",
            contenteditable:true,
            innerHTML: "|",
            inside: this.elem
         });
        // Bind events
	$.bind(this.caret, {
            "keypress": function(e) {
                if (!handle_keys) {
                    return;
                }
                var code = (e.keyCode ? e.keyCode : e.which);
                {
                    var c = String.fromCharCode(code);
                    if (/^[a-zA-Z_-]$/.test(c)) {
                        var new_sem = $.create("sem",{before: me.caret});
                        var new_label = $.create("label",{inside:new_sem});
                        var a = editLabel(me,new_label,false);
                        a.input.value=c;
                        a.evaluate();
                    }
                }
                e.preventDefault();
            },
            "keydown": function(e) {
                if (!handle_keys) {return;}
                if (e.keyCode==37) { // left arrow
                    var ref = me.caret;
                    ref.parentNode.insertBefore(me.caret,ref.previousElementSibling);
                    me.caret.focus();
                    e.preventDefault();
                }
                else if (e.keyCode==39) { //right arrow
                    var ref = me.caret.nextElementSibling;
                    ref.parentNode.insertBefore(me.caret,ref.nextElementSibling);
                    me.caret.focus();
                    e.preventDefault();
                }
            },
        });
        $.bind(this.elem, {
            "click": function(e) {
                if (e.target.nodeName == "LABEL" && e.target.getAttribute("locked") != "true")  {
                    editLabel(me,e.target,true);
                    e.preventDefault();
                }
            }
        });

        this.caret.focus();
    };
    _.prototype = {
        get fish() {return "cod";}

    };
    // Private functions
    function editLabel(te,label,select) {
        var v = label.innerHTML;
        label.style.display="none";
        var input = $.create("input",{
            before:label
        });
        var a = new Awesomplete(input, {
            className: "bb",
	    list: SYMBOLS,
            minChars: 1
        });
        input.value=v;
        input.focus();
        if (select) input.select();
        handle_keys=false;
        function close_a(e) {
            handle_keys=true;

            //if the new label is defined and different reset everything
            if (LABEL_TABLE[input.value] != undefined && label.innerHTML != input.value) {
                setupSem(input.value,label.parentNode);
            }
            label.style.display="inline-block";
            var p = input.parentNode;
            if (p.parentElement != null) {p.remove();}

            // move caret to after this node
            var ref = label.parentNode;
            ref.parentNode.insertBefore(te.caret,ref.nextElementSibling);
            te.caret.focus();
        }
        $.bind(input,{
            "blur":close_a,
        //    "awesomplete-close":close_a
        });
        return a;
    }

    function getOrCreate(tag,elem) {
        var e = $(tag,elem);
        if (!e) {
            e = $.create(tag,{inside:elem});
        }
        return e;
    }
    function kill(tag,elem) {
        var e = $(tag,elem);
        if (e) e.remove();
    }

    function setupSem(label_val,sem,lock) {
        var def = LABEL_TABLE[label_val];
        if (def.type == 'symbol') {
            var label = getOrCreate('label',sem);
            if (lock) label.setAttribute("locked","true");
            label.innerHTML=label_val;
            var struct = getOrCreate('struct',sem);
            struct.innerHTML=def.structure;
            var symbols = LABEL_TABLE[def.structure].symbols;
            // if structure has no symbols it's system defined so create a surface
            if (!symbols) {
                kill('children',sem);
                var surface = getOrCreate('surface',sem);
                surface.innerHTML = "";
                surface.setAttribute('contenteditable', 'true');
            }
            else {
                kill('surface',sem);
                var children = getOrCreate('children',sem);
                children.innerHTML = "";
                symbols.forEach(function(s){
                    var child_sem = $.create('sem',{inside:children});
                    setupSem(s,child_sem,true);
                });
            }
        }
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

    // Initialization

    function init() {
	$$(".TE").forEach(function (elem) {
	    new _(elem);
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

    _.$ = $;
    _.$$ = $$;

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
