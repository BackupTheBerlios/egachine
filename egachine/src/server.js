// this script is evaluated by egaserver on startup
EGachine.server=true;

// todo remove those?
Video={};
Input={};

// extend network object
Net.connections={};
Net.queue="";
Net.dist={};

// watch all properties and distribute changes to clients
Net.genericwatch=function(fqprop) {
  return function(p,o,n){if ((n!=o)&&(!dontwatch)) Net.enqueue(fqprop+'='+n+';');return n;};
}

Net.distribute=function(obj,oid){
  this.dist[oid]=obj;
  watchall(obj,oid,this.genericwatch);
  this.broadcastObject(obj,oid);
}

Net.handleNewConnection=function(id,stream) {
  this.connections[id]=stream;
  var obj;
  for (obj in this.dist)
    this.sendObjectTo(id,this.dist[obj],obj);
  handleNewConnection(id,stream);
}

Net.handleDataAvailable=function(id) {
  var h=this.connections[id].recv(6);
  var len=Number("0x"+h);
  if ((h.length==0)||(len==0)) {
    this.closeConnection(id);
    delete this.connections[id];
    return;
  }
  var msg=this.connections[id].recv(len);
  // todo security hole - clients should not be allowed to execute code on server
  var i=deserialize(msg);
  Input.handleInput(i);
}

Net.handleConnectionClosed=function(id) {
}

Net.sendTo=function(id,msg) {
  var h=msg.length.convertTo(16,6);
  this.connections[id].send(h);
  this.connections[id].send(msg);
  this.connections[id].sync();
}

Net.sendObjectTo=function(cid,obj,oid) {
  var msg=oid+"="+serialize(obj)+";delp("+oid+");";
  this.sendTo(cid,msg);
}

Net.broadcast=function(msg) {
  var c;
  for (c in this.connections) {
    this.sendTo(c,msg);
  }
}

Net.broadcastObject=function(obj,oid) {
  var msg=oid+"="+serialize(obj)+";delp("+oid+");";
  this.broadcast(msg);
}

Net.enqueue=function(msg) {
  this.queue+=msg;
}

Net.update=function() {
  // broadcast changes to clients
  if (this.queue.length>0) this.broadcast(this.queue);
  this.queue="";
}

Net.distribute(EGachine.r,"EGachine.r");


//! watch all properties in the object graph 
/*
  on change of a property call a generated function 
  \param gf function which generates the watch function
  if gf is not passed genericwatch is used
  
  \note properties coming from a prototype aren't watched
  this is because this caused trouble with serialization
  TODO: inspect this situation and understand what happens
  if you set a watch on property which comes from a prototype
*/
function watchall(cobj,cname,gf){

  // return function which can act as watch callback and simply
  // print some debug info
  function genericwatch(y) {
    return function(p,o,n){print(y+'='+n+';');return n;};
  };

  function _watchall(pobj,cobj,cname,scope,gf){
    //  print(scope);
    if (!gf) throw new Error("need generic function");
    if (typeof cobj != 'object') {
      var f=gf(scope);
      //    print(f.toSource());
      pobj.watch(cname,f);
      return;
    }
    if (cobj instanceof Array) {
      var k;
      for (var k in cobj) {
	_watchall(cobj, cobj[k], k, scope+"["+k+"]", gf);
      }
    }else{
      var k;
      for (var k in cobj) {
	if ((k[0]!='_')&&(!isFromProto(cobj,k)))
	  _watchall(cobj, cobj[k], k, scope+"."+k,     gf);
      }
    }
  }

  if (!gf) gf=genericwatch;
  _watchall(undefined,cobj,cname,cname,gf);
}
