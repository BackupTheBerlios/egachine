/*
 * Copyright (C) 2004 Jens Thiele <karme@berlios.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */


/*!
  \brief EGachine module
  \author Jens Thiele
*/

// load other required modules
ejs.ModuleLoader.load("Base64");
ejs.ModuleLoader.load("Zlib");
ejs.ModuleLoader.load("Timer");
ejs.ModuleLoader.load("Stream");
ejs.ModuleLoader.load("Util");
ejs.ModuleLoader.load("jsolait");
jsolait.lang=jsolait.importModule("lang");

// this script is evaluated from all programs in the egachine distribution

// global functions - todo: there perhaps should be none

function print(x){stdout.write(x);}
function println(s){print(s+"\n");};

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
  if (!idfunc) idfunc=function(x){return util.getObjectID(x).toString();};
  if (!func) throw new Error("need function");
  function _forall(x) {
    var hash,k;
    if (typeof x != 'object') {
      func(x,false);
      return;
    };
    hash=idfunc(x);
    if (m[hash]) return;
    m[hash]=true;
    func(x,false);
    for (k in x) {
      if (x.hasOwnProperty(k))
	_forall(x[k]);
    };
    func(x,true);
    return;
  };
  _forall(obj);
};

// find last element in sorted array-like object not greater than v
function findLastNotGreater(at,length,v){
  var last;
  if ((!at)||(!length)) throw new Error("invalid input");
  if (at(0)>v) return -1;
  last=length-1;
  if (at(last)<=v) return last;
  
  function _find(l,r) {
    var d,m;
    d=r-l;
    if (d<2) return l;
    m=l+(d>>1);
    if (at(m)>v)
      return _find(l,m);
    else
      return _find(m,r);
  };
  
  return _find(0,last);
};

// misc extensions
Number.prototype.convertTo=function(base,padTo){
  var s=this.toString(base);
  if (!padTo || s.length>=padTo) throw new Error("Does not fit");
  return Math.pow(10,padTo-s.length).toString().slice(1)+s;
};

if (!constructor)
  constructor=function(func){return func;};

//! Resource object
Resource=constructor(function(resname,resource) {
		       var comp,uncomp;
		       this.name=resname;
		       this.size=resource.length;

		       // try if the compressed version is smaller
		       comp=Base64.encode(Zlib.compress(resource));
		       uncomp=Base64.encode(resource);
		       if (comp.length<uncomp.length) {
			 this.z=true;
			 this.data=comp;
		       }else{
			 this.data=uncomp;
		       }
		     });

Resource.prototype.decode=function(){
  if (this.z) return Zlib.uncompress(Base64.decode(this.data),this.size);
  return Base64.decode(this.data);
};

Resource.prototype.toString=function(){
  //! wrap a string to fit into 80 columns
  function wrapString(str)
  {
    var result="",s=0,e,cols=80;
    while ((e=s+cols-1)<=str.length) {
      result+=str.substring(s,e)+"\\\n";
      s+=cols-1;
    }
    result+=str.substring(s,e);
    return result;
  };
  return "({name:\""+this.name+"\", size:"+this.size+","+(this.z ? " z:true, ":" ")+"data:\"\\\n"+wrapString(this.data)+"\\\n\"})";
};

//! EGachine object
EGachine={};
EGachine.r={};
EGachine.addResource=function(res){
  res.constructor=Resource.prototype.constructor;
  res.__proto__=Resource.prototype;
  EGachine.r[res.name]=res;
};
EGachine.getResource=function(name){
  if (!EGachine.r[name]) throw new Error("Resource '"+name+"' not found");
  return EGachine.r[name];
};

//! check required version
/*!
  \note the version field is set by native code
*/
EGachine.checkVersion=function(maj,min,mic)
{
  if (!this.version)          throw new Error("Could not determine version");
  if (maj<this.version.maj)   return true;
  if (maj>this.version.major) return false;
  if (min<this.version.min)   return true;
  if (min>this.version.min)   return false;
  if (mic<this.version.mic)   return true;
  if (mic>this.version.mic)   return false;
  return true;
};


//! restricted deserializer/eval
/*!
  does not deserialize functions
  function may only return one object
  function must not have any side-effects

  SIMPLE VARIANT:
  input looks like this: (each line is one input)
  x={a:10, b:20}
  x=[10, 20, 30]
  x=1;

  "grammar:"
  S->I=D
  D->O
  D->A
  D->V
  O->{o}
  O->{}
  o->P
  o->P, o
  P->I:D
  A->[a]
  A->[]
  a->D
  a->D, a
  V->number
  V->string
  I->identifier

  COMPLICATED VARIANT:
  this allows evaluation/deserialization of graphs

  input looks like this: (this is one input string including \n)
  x={a:10, b:20}
  x.c=[x.a, x.b]

  "grammar:" (not yet complete since access to R could be done by . and [])
  S->R=D
  S->R=D\ns
  s->r=D
  s->r=D\ns
  r->RM
  M->.I
  M->.IM
  M->[number]
  M->[number]M
  R->I
  D->O
  D->A
  D->V
  D->r
  O->{o}
  O->{}
  o->P
  o->P, o
  P->I:D
  A->[a]
  A->[]
  a->D
  a->D, a
  V->number
  V->string
  I->identifier


  TODO:
  implement it
  this is needed for the server which must not eval messages from clients
  other solutions?
  theoretically it would be enough to verify that the string satisfies
  a strict grammar and then use eval? this could cause trouble if the language
  changes and we do not update our parser

  function deserialize(str) {
  }
*/

// vector object
V2D=constructor(function(x,y){
			   this.x=x;
			   this.y=y;
			 });
V2D.middle=function(p1,p2) {
  return p1.add(p2).scale(0.5);
}
V2D.prototype.clone=function()
{
  return new V2D(this.x,this.y);
};
V2D.prototype.add=function(v)
{
  return new V2D(this.x+v.x,this.y+v.y);
};
V2D.prototype.inc=function(v)
{
  this.x+=v.x;
  this.y+=v.y;
  return this;
};
V2D.prototype.sub=function(v)
{
  return new V2D(this.x-v.x,this.y-v.y);
};
V2D.prototype.dec=function(v)
{
  this.x-=v.x;
  this.y-=v.y;
  return this;
};
V2D.prototype.scale=function(s)
{
  return new V2D(this.x*s,this.y*s);
};
V2D.prototype.rot90=function()
{
  return new V2D(this.y,-this.x);
};
V2D.prototype.length=function()
{
  return Math.sqrt(this.dot(this));
};
V2D.prototype.normalized=function()
{
  return this.scale(1/this.length());
};
V2D.prototype.dot=function(v)
{
  return this.x*v.x+this.y*v.y;
};

// degrees object
// this is mainly to get reference semantic / wrap primitive type
Degrees=constructor(function (deg){
			       this.value=deg;
			     });
Degrees.prototype.add=function(deg)
{
  return new Degrees(this.value+deg);
};
Degrees.prototype.inc=function(deg)
{
  this.value+=deg;
  return this;
};
Degrees.prototype.sub=function(deg)
{
  return new Degrees(this.value-deg);
};
Degrees.prototype.dec=function(deg)
{
  this.value-=deg;
  return this;
};

// devstate class
// this holds the state of joypad like input device
function DevState(dev,x,y,buttons){
  this.dev=dev;
  this.x=x;
  this.y=y;
  this.buttons=buttons;
};

// Node object
// curently prototype object of all nodes in the scenegraph
// (prototype of the prototypes)
Node=constructor(function() {});
Node.prototype.paint=function(time){
  var i;
  if (this.children)
    for (i=0;i<this.children;++i)
      this[i].paint(time);
};
// deprecated
Node.prototype.step=function(dt){
  var i;
  if (!this.children) return;
  for (i=0;i<this.children;++i)
    this[i].step(dt);
};
Node.prototype.add=function(n){
  if (!this.children) this.children=0;
  this[this.children++]=n;
  return this;
};

// derived object Rotate
Rotate=constructor(function (degrees) {
			      this.degrees=degrees;
			    });
Rotate.prototype=new Node();
Rotate.prototype.paint=function(time){
  Video.pushMatrix();
  Video.rotate(this.degrees.value);
  //  Node.prototype.paint.call(this,time);
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
};

// derived object Texture
Texture=constructor(function(resname){
			       this.resname=resname;
			     });
Texture.prototype=new Node();
Texture.prototype.paint=function(time){
  Video.drawTexture(Video.getTextureID(this.resname));
  //  Node.prototype.paint.call(this,time);
  Node.prototype.paint.call(this,time);
};

// derived object Scale
Scale=constructor(function(v) {
			     this.v=v;
			   });
Scale.prototype=new Node();
Scale.prototype.paint=function(time){
  Video.pushMatrix();
  Video.scale(this.v.x,this.v.y);
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
};

// derived object Translate
Translate=constructor(function(v) {
				 this.v=v;
			       });
Translate.prototype=new Node();
Translate.prototype.paint=function(time){
  Video.pushMatrix();
  Video.translate(this.v.x,this.v.y);
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
};

// derived object Sprite
Sprite=constructor(function(resname,size,pos,degrees) {
			      this.size=size;
			      this.pos=pos;
			      this.degrees=degrees;
			      this.resname=resname;
			    });
Sprite.prototype=new Node();
Sprite.prototype.paint=function(time){
  Video.pushMatrix();
  Video.translate(this.pos.x,this.pos.y);
  if (this.degrees) Video.rotate(this.degrees.value);
  Video.pushMatrix();
  Video.scale(this.size.x,this.size.y);
  Video.drawTexture(Video.getTextureID(this.resname));
  Video.popMatrix();
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
};

// derived object Color
Color=constructor(function(r,g,b,a) {
			     if (r.length)
			       this.c=r;
			     else
			       this.c=[r,g,b,a];
			   });
Color.prototype=new Node();
Color.prototype.paint=function(time){
  Video.pushColor();
  Video.setColor4v(this.c);
  Node.prototype.paint.call(this,time);
  Video.popColor();
};

// derived object Text
Text=constructor(function(text,hcenter,vcenter) {
			    this.text=text;
			    this.hcenter=hcenter;
			    this.vcenter=vcenter;
			  });
Text.prototype=new Node();
Text.prototype.paint=function(time){
  Video.drawText(this.text,this.hcenter,this.vcenter);
  Node.prototype.paint.call(this,time);
};

// derived object Quad
Quad=constructor(function(size,pos,degrees) {
			    this.size=size;
			    this.pos=pos;
			    this.degrees=degrees;
			  });
Quad.prototype=new Node();
Quad.prototype.paint=function(time){
  Video.pushMatrix();
  if (this.pos) Video.translate(this.pos.x,this.pos.y);
  if (this.degrees) Video.rotate(this.degrees.value);
  Video.drawQuad(this.size.x,this.size.y);
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
};

// derived object Mover (deprecated)
Mover=constructor(function(speed, rotspeed) {
			     this.speed=speed;
			     this.rotspeed=rotspeed;
			     this.time=0;
			     this.last=0;
			   });
Mover.prototype=new Node();
Mover.prototype.step=function(dt){
  var i,ct=this.time+dt;
  if (ct-this.last<2) {
    dontwatch=true;
    this.time=ct;
  }else{
    this.time=ct;
    this.last=this.time;
  }
  if (this.children)
    for (i=0;i<this.children;++i){
      this[i].pos.x+=this.speed.x*dt;
      this[i].pos.y+=this.speed.y*dt;
      if (this.rotspeed) {
	this[i].degrees.value+=this.rotspeed.value*dt;
	if (this[i].degrees.value>360) this[i].degrees.value-=360;
	if (this[i].degrees.value<0) this[i].degrees.value+=360;
      }
    }
  dontwatch=false;
  Node.prototype.step.call(this,dt);
};

(function(){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsegachine.la");
  if (!fname) throw new Error("Could not find module: 'ejsegachine.la'");
  ejs.ModuleLoader.loadNative.call(EGachine,"ejsegachine",fname.substring(0,fname.lastIndexOf(".")));
 })();
