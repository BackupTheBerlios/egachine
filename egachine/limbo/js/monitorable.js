success=false;
ejs.ModuleLoader.load("Monitorable");
ejs.ModuleLoader.load("Stream");
function println(x){stdout.write(x+"\n");stdout.sync();}
function chopLine(x){
  return x.substring(x.indexOf("\n")+1,x.length);
}
function getStack(){
  return chopLine(chopLine(new Error().stack.toString()));
}
function debug(x){
  //println(getStack());
}

debug("test");
t=new Monitorable();
t.monitor={
  onAdd:function(obj,prop,val){
    debug();
    success=true;
    return val;
  },
  onDelete:function(obj,prop){
    debug();
    success=true;
  },
  onGet:function(obj,prop,val){
    debug();
    success=true;
    return val;
  },
  onSet:function(obj,prop,val){
    debug();
    return val;
  }
};
t.a=10;
if ((!success)||(t.a!=10))
  throw new Error("failed");
success=false;
delete t.a;
if ((!success)||(t.a)) throw new Error("failed");
success=false;
var foo=t.a;
if (!success) throw new Error("failed");

// hmm how to "subclass" Monitorable objects?
// similar problem to "subclass" error objects
function Sub(){
  // copy [[class]]
  var res=new Monitorable();
  // restore other properties
  res.constructor=Sub;
  res.__proto__=Sub.prototype;
  return res;
}
Sub.prototype.foo=function(){return true;};
Sub.prototype.__proto__=Monitorable.prototype;

sub=new Sub();
if (!(sub instanceof Sub)) throw new Error();
if (!(sub instanceof Monitorable)) throw new Error();
success=false;
sub.monitor={
  onAdd:function(obj,prop,val){
    debug();
    success=true;
    return val;
  }
};
sub.x=10;
if (!success) throw new Error();

/* does not work since [[class]] is wrong 
success=false;

function SubClassic(){
}
SubClassic.prototype=new Monitorable();
sub=new SubClassic();
if (!(sub instanceof SubClassic)) throw new Error();
if (!(sub instanceof Monitorable)) throw new Error();
success=false;
sub.monitor={
  onAdd:function(obj,prop,val){
    debug();
    success=true;
    return val;
  }
};
sub.x=10;
if (!success) throw new Error();
*/

ejs.ModuleLoader.load('graphviz');
graphviz.dot(sub,"sub");
