// generated a dot file suitable for graphviz (dot)

function getNodeName(obj) {
  return hashObject(obj).toString();
}

function dot(obj,label)
{
  var m={};

  function _dot(obj) {
    if (!obj instanceof Object) return;
    var nodeName=getNodeName(obj);
    if (m[nodeName]) return;
    m[nodeName]=true;
    var i;
    var label=((obj instanceof Array) ? "Array" : typeof(obj) ); //+"("+nodeName+")";
    if (typeof(obj) == 'function') {
      label="";
      var src=obj.toSource();
      var tmp;
      var maxlen=50;
      if (src.length>maxlen) {
	tmp=src.substring(0,maxlen/2-3)+" ... "+src.substring(src.length-maxlen/2-2,src.length);
	src=tmp;
      }
      label+=src.replace(/\\/g,'\\\\').replace(/\"/g,'\\"');
    }
    // calculate node label
    for (i in obj) {
      // only use simple properties in the label
      var objType=typeof(obj[i]);
      if ((objType!='function')&&(objType!='object')) {
	if (objType=='string') 
	  label+="\\n"+i+":\\\""+obj[i]+"\\\"";
	else
	  label+="\\n"+i+":"+obj[i];
      }
    }
    // child nodes
    println(nodeName+" [label=\""+label+"\"]");
    for (i in obj) {
      if (obj[i] instanceof Object) {
	println(nodeName+" -> "+getNodeName(obj[i])+" [label=\""+i+ "\"" + (obj.hasOwnProperty(i) ? "" : ",style=dashed") + "]");
	_dot(obj[i]);
      }
    }
    // also print properties which aren't enumarated
    var hiddenProps=["__proto__","constructor"];
    for (i in hiddenProps) {
      if (obj[hiddenProps[i]]) {
	var p=hiddenProps[i];
	println(nodeName+" -> "+getNodeName(obj[p])+" [label=\""+p+"\",style=dotted]");
	_dot(obj[p]);
      }
    }
  }

  println("digraph jsgraph {");
  println("node [shape=box,fontsize=12]");
  println("start [style=invis]");
  println("start -> "+getNodeName(obj)+" [label=\""+label+"\"]");
  _dot(obj);
  println("}");
}

/*
x={a:#1={x:0,y:10},b:#1#,c:[10,20,"kjh",{foo:"bar"}],f:function(){
    var i=5;
    return i*i*i;
  }};
*/

function Foo(){
  this.x=10;
  this.s="Hello World";
}
Foo.prototype.bar=function(){println(this.s);};
x=new Foo();

try{
  dot(x,"x");
 }catch(error){
  println(error);
 }
