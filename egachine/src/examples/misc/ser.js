// prototype relationship test
Foo.prototype.bar=function(){
  return(this.f);
}
function Foo(f){
  this.f=f;
  this.d=new Date();
}

function protoTest(fser,fdser) {
  var foo=new Foo(10);
  print("src: "+foo.toSource());
  var s=fser(foo);
  var d=new fdser(s);
  print("dst: "+d.toSource());
  if (d.bar)
    return (foo.bar()==d.bar());
  return false;
}

function graphTest(fser,fdser) {
  var foo=new Foo(10);
  var graph={x:foo, y:foo};
  print("src: "+graph.toSource());
  var s=fser(graph);
  var d=fdser(graph);
  print("dst: "+d.toSource());
  d.x.f=77;
  if (!((foo.bar()==d.x.bar())&&(d.y.f==77))) return false;
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
  d.x.f.f=77;
  return (d.y.f.f==77)&&(d.y.x.f.f==77)&&(d.x.y.f.f==77);
}

//! call function for all properties of an object (and the object itself)
/*!
  \param obj the object
  \param func the function to call
  \param idfunc function returning a id for an object

  \note this works recursively and handles cycles in the graph
*/
function forall(obj,func,idfunc){
  var m={};
  if (!idfunc) idfunc=function(x){hashObject(x).toString();}
  function _forall(x){
    func(x);
    if (typeof(x) != 'object') return;
    var hash=idfunc(x);
    if (m[hash]) return;
    m[hash]=true;
    for (var k in x) _forall(x[k]);
  }
  _forall(obj);
}

function isEmptyProto(p) {
  for (a in p) return false;
  return true;
}

function delp(x){
  forall(x,(function(x){if (typeof(x) != 'object') return; if(x._p) {x.__proto__=x._p;delete x._p;};}));
}

//! serialize object
/*!
  \bug this destroys properties named _p
  \bug does not handle __proto__.__proto__
*/
function ser(x) {
  forall(x,(function(x){if (!isEmptyProto(x.__proto__)) x._p=x.__proto__;}));
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
