graphviz={};

if (typeof ejs == 'object') {
  // we are executed by a ejs shell
  try{
    // load required module (stdout.write)
    ejs.ModuleLoader.load('Stream');
    // not required but helpful (util.getObjectID)
    ejs.ModuleLoader.load('util');
  }catch(e){};
};

//! default style used by graphviz.dot
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

/*!
  visualize object graph starting with object obj (labeled by label)
  using optional style
*/
graphviz.dot=function(obj,label,style)
{
  // println function
  var println;
  // getObjectID function
  var getObjectID;
  // objects already visited
  var m={};
  var i;
  var from;
  var to;
  var tmp;

  // println
  if (typeof print == 'function') {
    // assume we are using a shell where print already adds \n
    println=print;
  }else if (typeof ejs == 'object') {
    println=function(x){stdout.write(x+"\n");}
  }else throw new Error("Could not find print function");

  // getObjectID
  if ((typeof util != 'object') || (typeof util.getObjectID != 'function')) {
    // we don't have the native GetObjectID function
    // => provide a (slow/ugly) ECMAScript version
    getObjectID=function(obj) {
      var i;
      if (!getObjectID.hashed) getObjectID.hashed=[];
      for (i=0;i<getObjectID.hashed.length;++i) {
	if (getObjectID.hashed[i] === obj) return i;
      };
      getObjectID.hashed.push(obj);
      return getObjectID.hashed.length-1;
    };
  }else{
    getObjectID=util.getObjectID;
  };
  
  if (style && (!(typeof style == "object")))
    throw new Error("Object required as third argument");
  if (!style) {
    style=graphviz.defaultStyle;
  }else{
    for (i in graphviz.defaultStyle) {
      if (style[i] == undefined) style[i] = graphviz.defaultStyle[i];
    }
  }

  function getNodeName(obj) {
    return getObjectID(obj).toString();
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
    var nodeName;
    var i;
    var j;
    var label;
    var src;
    var tmp;
    // maximum string length for function body
    var maxlen=50;
    var objType;
    var children;
    var from;
    var to;

    if (!obj instanceof Object) return;
    nodeName=getNodeName(obj);
    if (m[nodeName]) return;
    m[nodeName]=true;
    label=((obj instanceof Array) ? "Array" : typeof(obj) );
    if (typeof(obj) == 'function') {
      label="";
      src=obj.toSource();
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
      objType=typeof(obj[i]);
      if (obj.hasOwnProperty(i)&&(objType!='function')
	  &&(objType!='object')) {
	if (objType=='string') 
	  label+="\\n"+i+":\\\""+obj[i]+"\\\"";
	else
	  label+="\\n"+i+":"+obj[i];
      }
    }

    println(nodeName+" [label=\""+label+"\"]");

    // calculate child nodes to visit
    children=[];
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
      from=nodeName;
      to=getNodeName(obj[i]);
      if (style.reverse == true) {var tmp=from;from=to;to=tmp;};
      println(from+" -> "+ to +" [label=\""+i+ "\""
	      + (obj.hasOwnProperty(i) ? style.owner : style.inherit)
	      + (myPropertyIsEnumerable(obj,i) ? "" : style.hidden)
	      + style.edge 
	      + "]");
      _dot(obj[i]);
    }
  }

  println("digraph jsgraph {");
  println("node [shape=box]");
  println("start [label=\"\",style=invis]");
  from="start";
  to=getNodeName(obj);
  if (style.reverse == true) {tmp=from;from=to;to=tmp;};
  println(from+" -> " + to + " [label=\""+label+"\"" + style.edge + "]");
  _dot(obj);
  println("}");
};
