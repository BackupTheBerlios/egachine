(function(mod){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsstream.la");
  if (!fname) throw new Error("Could not find module: 'ejsstream.la'");
  ejs.ModuleLoader.loadNative.call(mod,"ejsstream",fname.substring(0,fname.lastIndexOf(".")));

  mod.StringStream=function()
    {
      this.buffer="";
      this.pos=0;
    };

  mod.StringStream.prototype.write=function(x){
    this.buffer+=x;
  };

  mod.StringStream.prototype.inAvailable=function(){
    return this.buffer.length-this.pos;
  };

  mod.StringStream.prototype.read=function(n){
    var ret;
    if ((n === undefined)||(n<0)) 
      throw Error("Number expected");
    if (n==0) return "";
    ret=this.buffer.substring(this.pos,this.pos+n);
    this.pos+=ret.length;
    return ret;
  };

  mod.StringStream.prototype.str=function(){
    return this.buffer;
  };

  mod.StringStream.prototype.clear=function(){
    this.buffer="";
    this.pos=0;
  };

  //! write objects to stream (similar to java ObjectOutputStream?)
  /*!
    \todo:
    - sparse arrays aren't serialized correctly
    could be done like this: (#1=([]),#1#[10]=1,#1#)
    - the "simple" objects shouldn't be treated specially
    at the moment there properties are possibly lost
    (example: new String("kjhkjh").foo=10;)
    - RegExp objects don't work (caused by typeof new RegExp() == 'function' ?)
  */
  mod.ObjectWriter=function(stream)
    {
      var i, toignore=[{}.constructor, Object.prototype, Function, Function.prototype];
      if (typeof Monitorable != typeof undefined)
	toignore.push(Monitorable.prototype);

      // remember already serialized objects
      this._serialized={};
      // how often write was called
      this._written=0;
      // underlying stream object
      this.stream=stream;
      // objects to ignore completely (todo: shit)
      this._ignore={};
      for (i=0;i<toignore.length;++i) this._ignore[util.getObjectID(toignore[i])]=true;
      // "hidden" properties to use
      // (properties we are interested in which aren't enumarated)
      this._hiddenProps=["__proto__","constructor"];
    };

  // todo: put this somewhere suitable
  mod.ObjectWriter.toHex=function(n){
    var s=Number(n).toString(16), padTo=6, z="000000";
    if (s.length>=padTo) throw new Error("out of range");
    return z.slice(-6+s.length)+s;
  }

  mod.ObjectWriter.prototype.sync=function()
    {
      this.stream.sync();
    };

  mod.ObjectWriter.prototype._remoteEval=function(x)
    {
      this.stream.write(""+ObjectWriter.toHex(x.length+1)+"e"+x);
    };

  //! object already written?
  mod.ObjectWriter.prototype.written=function(obj)
    {
      return this._serialized[util.getObjectID(obj)] != undefined;
    };

  //! write object/value
  mod.ObjectWriter.prototype.write=function(x)
    {
      var oo=this;
      // sharp variables within object graph
      var sharp={};
      var sharps=0;
      var root="this.o["+ (this._written++) +"]";
      var stream=new StringStream();

      function myPropertyIsEnumerable(obj,p) {
	// why this function?
	// since the for in loop and the propertyIsEnumerable method 
	// don't agree s.a.: sm source jsobj.c:obj_propertyIsEnumerable
	//    if ((obj.hasOwnProperty(p))&&(obj.propertyIsEnumerable(p))) return true;
	var i;
	for (i in obj) if (i==p) return true;
	return false;
      }

      //! simple objects? we don't take them apart (and use uneval below)
      /*
	\todo: take them apart - leave only native functions untouched ;-)
	[[class]] needs special care then
	(deserialization must invoke the native constructor at some time
	otherwise the class will be wrong)
      */
      function simpleObject(x) {
	return ( (    x instanceof String  )
		 || ( x instanceof Boolean )
		 || ( x instanceof Number  )
		 || ( x instanceof Error   )
		 || ( x instanceof Date    ) );
      }

      // todo: shit
      function ignoreProperty(p) {
	return p=="monitor";
      }

      // walk through the object graph and call callback functions stored in cbs
      function walk(px, x, fqname, name, cnum, cbs) {
	var tx=typeof x;
	var p, q, c, i;

	if ((tx != 'object')&&(tx!='function')) {
	  // primitives
	  cbs.primitive && cbs.primitive(px, x, fqname, name, cnum);
	  return true;
	}

	if (x === null) {
	  // null
	  cbs.nullObject && cbs.nullObject(px, fqname, name, cnum);
	  return true;
	}

	// some non-primitive type

	if (!cbs.ignore) throw Error("cbs.ignore required");
	if ((c=cbs.ignore(px, x, fqname, name, cnum))) return (c==1);

	if (tx == 'function') {
	  cbs.startFunction && cbs.startFunction(px, x, fqname, name, cnum);
	  x.prototype       && walk(x, x.prototype, fqname+'["prototype"]', "prototype", 0, cbs);
	  cbs.endFunction   && cbs.endFunction(px, x, fqname, name, cnum);
	  return true;
	}

	// objects

	if (x instanceof Array) {
	  // arrays
	  cbs.startArray && cbs.startArray(px, x, fqname, name, cnum);
	  for (p=0;p<x.length;++p) {
	    walk(x,x[p],fqname+"["+p+"]",p,p,cbs);
	  }
	  cbs.endArray && cbs.endArray(px, x, fqname, name, cnum);
	  return true;
	}

	cbs.startObject && cbs.startObject(px, x, fqname, name, cnum);
	if (!simpleObject(x)) {
	  // "normal" objects
	  c=[];
	  for (p=0;p<oo._hiddenProps.length;++p)
	    if (!myPropertyIsEnumerable(x,oo._hiddenProps[p])) c.push(oo._hiddenProps[p]);
	  for (p in x)
	    c.push(p);
      
	  q=0;
	  for (i=0;i<c.length;++i) {
	    p=c[i];
	    if ((x.hasOwnProperty(p))&&(!ignoreProperty(p)))
	      if (walk(x,x[p],fqname+"['"+p+"']",p,q,cbs))
		++q;
	  }
	}
	cbs.endObject && cbs.endObject(px, x, fqname, name, cnum);
	return true;
      };

      // find aliases (calculate sharp variables) within this object graph
      walk(undefined, x, root, root, 0,
	   {visited:{}, ignore:function(px,obj){
	       var key=util.getObjectID(obj);
	       if (oo._ignore[key]) return 2;
	       if (this.visited[key]) {
		 sharp[key]=++sharps;
		 return true;
	       }
	       if (oo._serialized[key]) return true;
	       this.visited[key]=true;
	       return false;
	     }});

      // do real serialization
      walk(undefined, x, root, root, 0,
	   {visited:{},
	       ignore:function(px, obj, fqname, name, cnum){
	       var ser, key=util.getObjectID(obj);
	       if (oo._ignore[key]) return 2;
	       if (this.visited[key]) {
		 this.visited[key]++;
		 this.name(px,obj,name,cnum);
		 stream.write("#"+sharp[key]+"#");
		 return 1;
	       }
	   
	       if ((ser=oo._serialized[key])) {
		 this.name(px,obj,name,cnum);
		 stream.write(ser.n);
		 return 1;
	       }

	       this.visited[key]=1;
	       oo._serialized[key]={n:fqname, o:obj};
	       return false;
	     },name:function(px, x, name, cnum) {
	       if (typeof px === 'function') return;
	       if (typeof px == 'object') {
		 if (cnum>0)
		   stream.write(",");
		 if (!(px instanceof Array))
		   stream.write(name+":");
	       }
	       if (((typeof x == 'object')&&(x!=null))||(typeof x == 'function')) {
		 var key=util.getObjectID(x);
		 if (sharp[key]&&(this.visited[key]<2)){
		   if (typeof x == 'function')
		     stream.write("((");
		   stream.write("#"+sharp[key]+"=");
		 }
	       }
	     },
		 primitive:function(px, x, fqname, name, cnum) {
	       this.name(px, x, name, cnum);
	       // todo: for which x is typeof x == 'undefined'
	       if ((px instanceof Array)&&(typeof x == 'undefined')) return;
	       stream.write(uneval(x));
	     },
	       nullObject:function(px, fqname, name, cnum) {
	       this.primitive(px, null ,fqname ,name, cnum);
	     },
	       startFunction:function(px, x, fqname, name, cnum) {
	       var key=util.getObjectID(x);
	       var s=x.toSource()
		 .replace(/^\(/,"")
		 .replace(/\)$/,"")
		 //	     .replace(/function ([a-zA-Z0-9])*\(/,"function(")
		 .replace(/\) \{/,"){");

	       if ((x.prototype)&&(!(sharp[key]))) {
		 // we now mark all functions with prototypes sharp
		 // why do we have to do this?
		 // since we serialize functions like this:
		 // ((#12=function (){}).prototype=#1#,#12#)
		 sharp[key]=++sharps;
		 this.visited[key]=1;
	       }
	       this.name(px, x, name, cnum);
	       stream.write(s+").prototype=");
	       this.inFunction=true;
	     },
	       endFunction:function(px, x, fqname, name, cnum) {
	       stream.write(",#"+sharp[util.getObjectID(x)]+"#)");
	       this.inFunction=false;
	     },
	       startArray:function(px, x, fqname, name, cnum) {
	       this.name(px, x, name, cnum);
	       stream.write("[");
	     },
	       endArray:function(px, x, fqname, name, cnum) {
	       if (x[x.length-1]===undefined){
		 // special case since [,] means [undefined]
		 // and not [undefined,undefined]
		 stream.write(",");
	       }
	       stream.write("]");
	     },
	       startObject:function(px, x, fqname, name, cnum) {
	       this.name(px, x, name, cnum);
	       //	   if (!px) stream.write("(");
	       if (!simpleObject(x))
		 stream.write("{");
	       else
		 stream.write(uneval(x));
	     },
	       endObject:function(px, x, fqname, name, cnum) {
	       if (!simpleObject(x))
		 stream.write("}");
	       //	   if (!px) stream.write(")");
	     }
	   });

      oo.stream.write(""+ObjectWriter.toHex(stream.str().length+1)+"o"+stream.str());
      return root;
    };

  //! update property of already written object
  mod.ObjectWriter.prototype.updateProperty=function(obj,pname,pval)
    {
      var valt, objo, valo, fqpname, rpval;

      if (!(objo=this._serialized[util.getObjectID(obj)]))
	throw new Error("not yet serialized");
      fqpname=objo.n+"['"+pname+"']";
      valt=typeof pval;
      if ((valt != 'object')&&(valt!='function')) {
	// update primitive property
	this._remoteEval(fqpname+"="+uneval(pval));
      }else{
	// complex property
	if ((valo=this._serialized[util.getObjectID(pval)])) {
	  // set to already serialized property
	  rpval=valo.n;
	}else{
	  // new complex property
	  rpval=this.write(pval);
	}
	if (!rpval) throw Error(rpval);
	this._remoteEval(fqpname+"="+rpval);
      }
    };

  mod.ObjectReader=function(stream)
    {
      // remember already deserialized objects
      this.o=[];
      // how often read was called
      this._read=0;
      // underlying stream object
      this.stream=stream;
    };

  //! read object
  mod.ObjectReader.prototype.read=function()
    {
      var h="0x"+this.stream.read(6);
      var msg=this.stream.read(Number(h));
      var msgtype=msg[0];
      var ret;

      function debug(x){
	//    stderr.write(x+"\n");
      }

      //  debug("h:"+h);
      debug("msgtype:"+msgtype);

      if (msgtype=="o") {
	try{
	  ret=eval("this.o["+this._read+"]="+msg.slice(1));
	  debug("got this.o["+this._read+"]");
	}catch(e){
	  debug("error reading this.o["+this._read+"]: "+e);
	  debug("this.o: "+uneval(this.o));
	  debug("msg:\n"+msg.slice(1));
	  throw new Error("abort");
	}
	this._read++;
      }else if (msgtype=="e"){
	ret=eval(msg.slice(1));
	debug("evaluated:"+msg.slice(1)+" to:"+uneval(ret));
      }else
	throw new Error("received unknown message type: '"+msgtype+"'");
      return ret;
    };
 })(this);
