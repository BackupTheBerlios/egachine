// DONTINSTALL

// prototype relationship test
function Base(){
}
Base.prototype.b=8;
Base.prototype.a=[6,5,3,77];

function Foo(f){
  this.f=f;
  this.d=new Date();
}
Foo.prototype=new Base;
Foo.prototype.p=10;
Foo.prototype.bar=function(){
  return(this.f);
}

function protoTest(fser,fdser) {
  var foo=new Foo(10);
  print("src: "+foo.toSource());
  var s=fser(foo);
  var d=new fdser(s);
  print("dst: "+d.toSource());
  var s2=fser(d);
  if (d.bar)
    return (foo.bar()==d.bar())&&(foo.b==d.b)&&(s==s2)&&(d.a[3]==77);
  return false;
}

function graphTest(fser,fdser) {
  var foo=new Foo(10);
  var graph={x:foo, y:foo};
  print("src: "+graph.toSource());
  var s=fser(graph);
  var d=fdser(graph);
  print("dst: "+d.toSource());
  var s2=fser(d);
  d.x.f=77;
  if (!((foo.bar()==d.x.bar())&&(d.y.f==77))) return false;
  if (d.x.b!=d.y.b) return false;
  if (s!=s2) return false;
  return d.x.f.bar!=foo.bar;
}

function circleTest(fser,fdser) {
  var foo=new Foo(10);
  var graph={x:{f:foo}, y:{f:foo}};
  graph.x.y=graph.y;
  graph.y.x=graph.x;
  print("src: "+graph.toSource());
  var s=fser(graph);
  var d=fdser(graph);
  print("dst: "+d.toSource());
  var s2=fser(d);
  d.x.f.f=77;
  if (d.x.b!=d.y.b) return false;
  if (s!=s2) return false;
  return (d.y.f.f==77)&&(d.y.x.f.f==77)&&(d.x.y.f.f==77);
}

//! call function for all properties of an object (and the object itself)
/*!
  \param obj the object
  \param func(x,depthFirst) the function to call 
         (for objects it is called twice - once before going into the depth,
	 and once after - depthFirst is set to reflect this)
  \param idfunc function returning a ID for an object

  \note this works recursively and handles cycles in the graph
  if you pass in a correct idfunc
*/
function forall(obj,func,idfunc){
  var m={};
  if (!idfunc) idfunc=function(x){return hashObject(x).toString();}
  if (!func) throw "need function";
  function _forall(x){
    if (typeof(x) != 'object') {
      func(x,false);
      return;
    }
    var hash=idfunc(x);
    if (m[hash]) return;
    m[hash]=true;
    func(x,false);
    for (var k in x) {
      _forall(x[k]);
    };
    func(x,true);
    return;
  }
  _forall(obj);
}

function isEmptyProto(p) {
  for (a in p) return false;
  return true;
}

function delp(x){
  forall(x,(function(x,depthFirst,debug){
    if (typeof(x) != 'object') return;
    if (!depthFirst) {
      if (x._p) x.__proto__=x._p;
    }else{
      if (x._p) delete x._p;
    }
  }));
}

//! serialize object
/*!
  \bug properties named _p are not allowed
  \note temporarily adds property _p as copy of __proto__
*/
function ser(x) {
  forall(x,(function(x,depthFirst){
    if (depthFirst||isEmptyProto(x.__proto__)) return;
    if (x._p) throw "TODO: property _p not allowed";
    x._p=x.__proto__;
  }));
  var r=x.toSource();
  delp(x);
  print("ser: "+r);
  return r;
}

//! deserialize object
/*
  \note This calls eval - which depending on your usage may be a security hole
*/
function dser(str) {
  var x=eval(str);
  delp(x);
  return x;
}

print (protoTest(ser,dser));
print (graphTest(ser,dser));
print (circleTest(ser,dser));
