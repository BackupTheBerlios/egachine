graphviz={};
try{
  ejs.ModuleLoader.load('util');
}catch(){}

// graphviz.getObjectID
if (!util.getObjectID) {
  // we don't have the native hashObject function
  // => provide a (slow/ugly) ECMAScript version
  graphviz.getObjectID=function(obj) {
    var i;
    if (!graphviz.getObjectID.hashed) graphviz.getObjectID.hashed=[];
    for (i=0;i<graphviz.getObjectID.hashed.length;++i) {
      if (graphviz.getObjectID.hashed[i] === obj) return i;
    };
    graphviz.getObjectID.hashed.push(obj);
    return graphviz.getObjectID.hashed.length-1;
  };
}else{
  graphviz.getObjectID=util.getObjectID;
};

// graphviz.println
if (this.println) {
  graphviz.println=this.println;
}elseif (this.print) {
  // assume we are using spidermonkey shell where print already adds \n
  graphviz.println=this.print;
}elsif (ejs.ModuleLoader.load) {
  ejs.ModuleLoader.load('Stream');
  graphviz.println=function(x){stdout.write(x+"\n");}
}

graphviz.defaultStyle={
  reverse:#1=(new Boolean(true)),
  // style of edges to properties that are own properties and enumerated
  owner:"",
  // style of edges to properties inherited from a prototype
  inherit:",style=dashed",
  // style of edges to properties that aren't enumerated and somehow special
  hidden:",fontcolor=blue",
  // common style
  edge:((#1# == true) ?  ",dir=back" : ""),
  // objects to ignore
  ignore:[],
  // "hidden" properties to use
  // (properties we are interested in which aren't enumarated)
  hiddenProps:["__proto__","constructor","prototype"]
};

graphviz.dot=function(obj,label,style)
{
  var m={};

  if (style && (!(typeof style == "object")))
    throw new Error("Object required as third argument");
  if (!style) {
    style=graphviz.defaultStyle;
  }else{
    var i;
    for (i in graphviz.defaultStyle) {
      if (style[i] == undefined) style[i] = graphviz.defaultStyle[i];
    }
  }

  function getNodeName(obj) {
    return graphviz.getObjectID(obj).toString();
  };

  function ignore(obj) {
    var i;
    for (i in style.ignore) if (style.ignore[i] === obj) return true;
    return false;
  }

  function myPropertyIsEnumerable(obj,p) {
    // why this function?
    // since the for in loop and the propertyIsEnumerable method 
    // don't agree s.a.: sm source jsobj.c:obj_propertyIsEnumerable
    var i;
    for (i in obj) if (i==p) return true;
    return false;
  }

  function _dot(obj) {
    if (!obj instanceof Object) return;
    var nodeName=getNodeName(obj);
    if (m[nodeName]) return;
    m[nodeName]=true;
    var i;
    var j;
    var label=((obj instanceof Array) ? "Array" : typeof(obj) );
    if (typeof(obj) == 'function') {
      label="";
      var src=obj.toSource();
      var tmp;
      var maxlen=50;
      if (src.length>maxlen) {
	tmp=src.substring(0,maxlen/2-3)+" ... "
	  +src.substring(src.length-maxlen/2-2,src.length);
	src=tmp;
      }
      label+=src.replace(/\\/g,'\\\\').replace(/\"/g,'\\"');
    }
    // calculate node label
    for (i in obj) {
      // only use simple properties in the label
      // and don't show inherited ones
      var objType=typeof(obj[i]);
      if (obj.hasOwnProperty(i)&&(objType!='function')
	  &&(objType!='object')) {
	if (objType=='string') 
	  label+="\\n"+i+":\\\""+obj[i]+"\\\"";
	else
	  label+="\\n"+i+":"+obj[i];
      }
    }

    graphviz.println(nodeName+" [label=\""+label+"\"]");

    // calculate child nodes to visit
    var children=[];
    for (i in obj)
      if (obj[i] instanceof Object && (!ignore(obj[i])))
	children.push(i);
    for (j in style.hiddenProps) {
      i=style.hiddenProps[j];
      if (obj[i] && (!myPropertyIsEnumerable(obj,i)) && (!ignore(obj[i])))
	children.push(i);
    }

    // visit children
    for (j in children) {
      i=children[j];
      var from=nodeName;
      var to=getNodeName(obj[i]);
      if (style.reverse == true) {var tmp=from;from=to;to=tmp;};
      graphviz.println(from+" -> "+ to +" [label=\""+i+ "\""
	      + (obj.hasOwnProperty(i) ? style.owner : style.inherit)
	      + (myPropertyIsEnumerable(obj,i) ? "" : style.hidden)
	      + style.edge 
	      + "]");
      _dot(obj[i]);
    }
  }

  graphviz.println("digraph jsgraph {");
  graphviz.println("node [shape=box]");
  graphviz.println("start [label=\"\",style=invis]");
  var from="start";
  var to=getNodeName(obj);
  if (style.reverse == true) {var tmp=from;from=to;to=tmp;};
  graphviz.println(from+" -> " + to + " [label=\""+label+"\"" + style.edge + "]");
  _dot(obj);
  graphviz.println("}");
};
