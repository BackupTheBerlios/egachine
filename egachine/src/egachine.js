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

/* base64 stuff:
 * Copyright (C) 1999 Masanao Izumo <mo@goice.co.jp>
 * Version: 1.0-karme
 * LastModified: 2004 by Jens Thiele
 */

/*!
   \file egachine.js
   \brief egachine common script library
   \author Jens Thiele
*/

// this script is evaluated on egachine and egaserver startup

// global functions - todo: there perhaps should be none

// is the passed object an empty prototype? (empty object)
function isEmptyProto(p) {
  return (p=={}.__proto__);
}

// is this property from a prototype?
function isFromProto(o,prop) {
  var p=o.__proto__;
  while (p!=undefined) {
    if (p[prop]&&(o[prop]==p[prop])) return true;
    p=p.__proto__;
  }
  return false;
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
  if (!idfunc) idfunc=function(x){return hashObject(x).toString();};
  if (!func) throw new Error("need function");
  function _forall(x) {
    if (typeof(x) != 'object') {
      func(x,false);
      return;
    };
    var hash=idfunc(x);
    if (m[hash]) return;
    m[hash]=true;
    func(x,false);
    for (var k in x) {
      if (!isFromProto(x,k))
	_forall(x[k]);
    };
    func(x,true);
    return;
  };
  _forall(obj);
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
function serialize(x) {
  forall(x,(function(x,depthFirst){
    if (typeof(x) != 'object') return;
    if ((depthFirst)||(isEmptyProto(x.__proto__))) return;
    if ((x._p)&&(!isFromProto(x,"_p")))
      throw new Error("TODO: property _p not allowed");
    x._p=x.__proto__;
  }));
  var r=x.toSource();
  delp(x);
  //  print("ser: "+r);
  return r;
}

//! deserialize object
/*
  \note This calls eval - which depending on your usage may be a security hole
*/
function deserialize(str) {
  var x=eval(str);
  delp(x);
  return x;
}

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
    if (typeof(cobj) != 'object') {
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



// end global functions --------------------------------------------------------


// misc extensions
Number.prototype.convertTo=function(base,padTo){
    var s=this.toString(base);
    if (!padTo || s.length>=padTo) throw new Error("Does not fit");
    return Math.pow(10,padTo-s.length).toString().slice(1)+s;
}

//! EGachine object
EGachine={};
EGachine.r={};
EGachine.addResource=function(name,res){
  EGachine.r[name]=res;
}
EGachine.getResource=function(name){
  if (!EGachine.r[name]) throw new Error("Resource '"+name+"' not found");
  return EGachine.r[name];
}

//! check required version
/*!
  \note the version field is set by native code
*/
EGachine.checkVersion=function(maj,min,mic)
{
  if (!this.version) throw new Error("Could not determine version");
  if (maj<this.version.maj) return true;
  if (maj>this.version.major) return false;
  if (min<this.version.min) return true;
  if (min>this.version.min) return false;
  if (mic<this.version.mic) return true;
  if (mic>this.version.mic) return false;
  return true;
}

//! Audio object
if (this.Audio) {
  Audio._playMusic=Audio.playMusic;
  Audio.playMusic=function(resname) {
    var res=EGachine.getResource(resname);
    var dec=Base64.decode(res);
    Audio._playMusic(dec);
  };

  Audio.samples={};

  Audio._loadSample=Audio.loadSample;
  Audio._playSample=Audio.playSample;

  Audio.loadSample=function(resname) {
    var res=EGachine.getResource(resname);
    var dec=Base64.decode(res);
    var sid=Audio._loadSample(dec);
    Audio.samples[resname]=sid;
    return sid;
  };
  Audio.playSample=function(resname) {
    var sid=Audio.samples[resname];
    if (sid==undefined)
      sid=Audio.loadSample(resname);
    return Audio._playSample(sid);
  }
}

//! Video object
if (!this.Video) {
  // the server does not have native Video and Input objects
  // todo: he does not need them? - perhaps add a client.js
  // corresponding to the server.js
  Video={};
  Input={};
  EGachine.server=true;
}else{
  EGachine.client=true;
}

// map resname to texture id
Video.textures={};
// color stack
Video.colors=[];

//! map resname to texture id
/*!
  create texture from resource if necessary
*/
Video.getTextureID=function(resname){
  var tid=Video.textures[resname];
  if (tid) return tid;
  var res=EGachine.getResource(resname);
  var dec=Base64.decode(res);
  tid=Video.textures[resname]=Video.createTexture(dec);
  return tid;
};

Video.pushColor=function() {
  Video.colors.push(gl.GetFloatv(GL_CURRENT_COLOR));
}

Video.popColor=function() {
  var c=Video.colors.pop();
  Video.setColor(c[0],c[1],c[2],c[3]);
}

// vector object - TODO: operator +,-,... - perhaps native code
function V2D(x,y){
  this.x=x;
  this.y=y;
}

// degrees object - TODO: operator +,-
// this is mainly to get reference semantic / wrap primitive type
function Degrees(deg){
  this.value=deg;
}

// devstate class
// this holds the state of joypad like input device
function DevState(dev,x,y,buttons){
  this.dev=dev;
  this.x=x;
  this.y=y;
  this.buttons=buttons;
}

// Node object
// curently prototype object of all nodes in the scenegraph
// (prototype of the prototypes)
function Node() {
}
Node.prototype.paint=function(){
  if (!this.children) return;
  for (var i=0;i<this.children.length;++i)
    if (!this.children[i].disabled) {
      this.children[i].paint();
    }
}
Node.prototype.step=function(dt){
  if (!this.children) return;
  for (var i=0;i<this.children.length;++i)
    this.children[i].step(dt);
}
Node.prototype.addNode=function(n){
  if (!this.children) this.children=[];
  this.children.push(n);
  return this;
}

// derived object Rotate
function Rotate(degrees) {
  this.degrees=degrees;
}
Rotate.prototype=new Node();
Rotate.prototype.paint=function(){
  Video.pushMatrix();
  Video.rotate(this.degrees.value);
  Node.prototype.paint.call(this);
  Video.popMatrix();
}

// derived object Texture
function Texture(resname){
  this.resname=resname;
}
Texture.prototype=new Node();
Texture.prototype.paint=function(){
  Video.drawTexture(Video.getTextureID(this.resname));
  Node.prototype.paint.call(this);
}

// derived object Scale
function Scale(v) {
  this.v=v;
}
Scale.prototype=new Node();
Scale.prototype.paint=function(){
  Video.pushMatrix();
  Video.scale(this.v.x,this.v.y);
  Node.prototype.paint.call(this);
  Video.popMatrix();
}

// derived object Translate
function Translate(v) {
  this.v=v;
}
Translate.prototype=new Node();
Translate.prototype.paint=function(){
  Video.pushMatrix();
  translate(this.v.x,this.v.y);
  Node.prototype.paint.call(this);
  Video.popMatrix();
}

// derived object Sprite
function Sprite(resname,size,pos,degrees) {
  this.size=size;
  this.pos=pos;
  this.degrees=degrees;
  this.resname=resname;
}
Sprite.prototype=new Node();
Sprite.prototype.paint=function(){
  Video.pushMatrix();
  Video.translate(this.pos.x,this.pos.y);
  if (this.degrees) Video.rotate(this.degrees.value);
  Video.pushMatrix();
  Video.scale(this.size.x,this.size.y);
  Video.drawTexture(Video.getTextureID(this.resname));
  Video.popMatrix();
  Node.prototype.paint.call(this);
  Video.popMatrix();
}

// derived object Mover
function Mover(speed, rotspeed) {
  this.speed=speed;
  this.rotspeed=rotspeed;
  this.time=0;
  this.last=0;
}
Mover.prototype=new Node();
Mover.prototype.step=function(dt){
  var ct=this.time+dt;
  if (ct-this.last<1) {
    dontwatch=true;
    this.time=ct;
  }else{
    this.time=ct;
    this.last=this.time;
  }
  for (var i=0;i<this.children.length;++i){
    this.children[i].pos.x+=this.speed.x*dt;
    this.children[i].pos.y+=this.speed.y*dt;
    if (this.rotspeed) {
      this.children[i].degrees.value+=this.rotspeed.value*dt;
      if (this.children[i].degrees.value>360) this.children[i].degrees.value-=360;
      if (this.children[i].degrees.value<0) this.children[i].degrees.value+=360;
    }
  }
  dontwatch=false;
  Node.prototype.step.call(this,dt);
}

// derived object Color
function Color(r,g,b,a) {
  this.c=[r,g,b,a];
}
Color.prototype=new Node();
Color.prototype.paint=function(){
  Video.pushColor();
  Video.setColor(this.c[0],this.c[1],this.c[2],this.c[3]);
  Node.prototype.paint.call(this);
  Video.popColor();
}

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

// begin base 64 stuff:
/* Copyright (C) 1999 Masanao Izumo <mo@goice.co.jp>
 * Version: 1.0
 * LastModified: Dec 25 1999
 * This library is free.  You can redistribute it and/or modify it.
 *
 * modified by Jens Thiele, 2004
 */

Base64={};
Base64._encodeChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
Base64._decodeChars = new Array(
				     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
				     -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
				     52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
				     -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
				     15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
				     -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
				     41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1);

Base64.encode=function(str) {
  var out, i, len;
  var c1, c2, c3;

  len = str.length;
  i = 0;
  out = "";
  while(i < len) {
    c1 = str.charCodeAt(i++) & 0xff;
    if(i == len)
      {
	out += this._encodeChars.charAt(c1 >> 2);
	out += this._encodeChars.charAt((c1 & 0x3) << 4);
	out += "==";
	break;
      }
    c2 = str.charCodeAt(i++);
    if(i == len)
      {
	out += this._encodeChars.charAt(c1 >> 2);
	out += this._encodeChars.charAt(((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4));
	out += this._encodeChars.charAt((c2 & 0xF) << 2);
	out += "=";
	break;
      }
    c3 = str.charCodeAt(i++);
    out += this._encodeChars.charAt(c1 >> 2);
    out += this._encodeChars.charAt(((c1 & 0x3)<< 4) | ((c2 & 0xF0) >> 4));
    out += this._encodeChars.charAt(((c2 & 0xF) << 2) | ((c3 & 0xC0) >>6));
    out += this._encodeChars.charAt(c3 & 0x3F);
  }
  return out;
}

Base64.decode=function(str) {
  var c1, c2, c3, c4;
  var i, len, out;

  len = str.length;
  i = 0;
  out = "";
  while(i < len) {
    /* c1 */
    do {
      c1 = this._decodeChars[str.charCodeAt(i++) & 0xff];
    } while(i < len && c1 == -1);
    if(c1 == -1)
      break;

    /* c2 */
    do {
      c2 = this._decodeChars[str.charCodeAt(i++) & 0xff];
    } while(i < len && c2 == -1);
    if(c2 == -1)
      break;

    out += String.fromCharCode((c1 << 2) | ((c2 & 0x30) >> 4));

    /* c3 */
    do {
      c3 = str.charCodeAt(i++) & 0xff;
      if(c3 == 61)
	return out;
      c3 = this._decodeChars[c3];
    } while(i < len && c3 == -1);
    if(c3 == -1)
      break;

    out += String.fromCharCode(((c2 & 0XF) << 4) | ((c3 & 0x3C) >> 2));

    /* c4 */
    do {
      c4 = str.charCodeAt(i++) & 0xff;
      if(c4 == 61)
	return out;
      c4 = this._decodeChars[c4];
    } while(i < len && c4 == -1);
    if(c4 == -1)
      break;
    out += String.fromCharCode(((c3 & 0x03) << 6) | c4);
  }
  return out;
}
// end base 64 stuff

// opengl constants
// did not put them into gl. because
// gl.Begin(GL_LINES) is nicer than gl.Begin(gl.LINES) or similar
// and
// gl.xxx(gl.2_BYTES) is not possible

GL_FALSE=0x0
GL_TRUE=0x1
GL_BYTE=0x1400
GL_UNSIGNED_BYTE=0x1401
GL_SHORT=0x1402
GL_UNSIGNED_SHORT=0x1403
GL_INT=0x1404
GL_UNSIGNED_INT=0x1405
GL_FLOAT=0x1406
GL_DOUBLE=0x140A
GL_2_BYTES=0x1407
GL_3_BYTES=0x1408
GL_4_BYTES=0x1409
GL_POINTS=0x0000
GL_LINES=0x0001
GL_LINE_LOOP=0x0002
GL_LINE_STRIP=0x0003
GL_TRIANGLES=0x0004
GL_TRIANGLE_STRIP=0x0005
GL_TRIANGLE_FAN=0x0006
GL_QUADS=0x0007
GL_QUAD_STRIP=0x0008
GL_POLYGON=0x0009
GL_VERTEX_ARRAY=0x8074
GL_NORMAL_ARRAY=0x8075
GL_COLOR_ARRAY=0x8076
GL_INDEX_ARRAY=0x8077
GL_EDGE_FLAG_ARRAY=0x8079
GL_VERTEX_ARRAY_SIZE=0x807A
GL_VERTEX_ARRAY_TYPE=0x807B
GL_VERTEX_ARRAY_STRIDE=0x807C
GL_NORMAL_ARRAY_TYPE=0x807E
GL_NORMAL_ARRAY_STRIDE=0x807F
GL_COLOR_ARRAY_SIZE=0x8081
GL_COLOR_ARRAY_TYPE=0x8082
GL_COLOR_ARRAY_STRIDE=0x8083
GL_INDEX_ARRAY_TYPE=0x8085
GL_INDEX_ARRAY_STRIDE=0x8086
GL_EDGE_FLAG_ARRAY_STRIDE=0x808C
GL_VERTEX_ARRAY_POINTER=0x808E
GL_NORMAL_ARRAY_POINTER=0x808F
GL_COLOR_ARRAY_POINTER=0x8090
GL_INDEX_ARRAY_POINTER=0x8091
GL_EDGE_FLAG_ARRAY_POINTER=0x8093
GL_V2F=0x2A20
GL_V3F=0x2A21
GL_C4UB_V2F=0x2A22
GL_C4UB_V3F=0x2A23
GL_C3F_V3F=0x2A24
GL_N3F_V3F=0x2A25
GL_C4F_N3F_V3F=0x2A26
GL_T2F_V3F=0x2A27
GL_T4F_V4F=0x2A28
GL_T2F_C4UB_V3F=0x2A29
GL_T2F_C3F_V3F=0x2A2A
GL_T2F_N3F_V3F=0x2A2B
GL_T2F_C4F_N3F_V3F=0x2A2C
GL_T4F_C4F_N3F_V4F=0x2A2D
GL_MATRIX_MODE=0x0BA0
GL_MODELVIEW=0x1700
GL_PROJECTION=0x1701
GL_POINT_SMOOTH=0x0B10
GL_POINT_SIZE=0x0B11
GL_POINT_SIZE_GRANULARITY=0x0B13
GL_POINT_SIZE_RANGE=0x0B12
GL_LINE_SMOOTH=0x0B20
GL_LINE_STIPPLE=0x0B24
GL_LINE_STIPPLE_PATTERN=0x0B25
GL_LINE_STIPPLE_REPEAT=0x0B26
GL_LINE_WIDTH=0x0B21
GL_LINE_WIDTH_GRANULARITY=0x0B23
GL_LINE_WIDTH_RANGE=0x0B22
GL_POINT=0x1B00
GL_LINE=0x1B01
GL_FILL=0x1B02
GL_CW=0x0900
GL_CCW=0x0901
GL_FRONT=0x0404
GL_BACK=0x0405
GL_POLYGON_MODE=0x0B40
GL_POLYGON_SMOOTH=0x0B41
GL_POLYGON_STIPPLE=0x0B42
GL_EDGE_FLAG=0x0B43
GL_CULL_FACE=0x0B44
GL_CULL_FACE_MODE=0x0B45
GL_FRONT_FACE=0x0B46
GL_POLYGON_OFFSET_FACTOR=0x8038
GL_POLYGON_OFFSET_UNITS=0x2A00
GL_POLYGON_OFFSET_POINT=0x2A01
GL_POLYGON_OFFSET_LINE=0x2A02
GL_POLYGON_OFFSET_FILL=0x8037
GL_COMPILE=0x1300
GL_COMPILE_AND_EXECUTE=0x1301
GL_LIST_BASE=0x0B32
GL_LIST_INDEX=0x0B33
GL_LIST_MODE=0x0B30
GL_NEVER=0x0200
GL_LESS=0x0201
GL_EQUAL=0x0202
GL_LEQUAL=0x0203
GL_GREATER=0x0204
GL_NOTEQUAL=0x0205
GL_GEQUAL=0x0206
GL_ALWAYS=0x0207
GL_DEPTH_TEST=0x0B71
GL_DEPTH_BITS=0x0D56
GL_DEPTH_CLEAR_VALUE=0x0B73
GL_DEPTH_FUNC=0x0B74
GL_DEPTH_RANGE=0x0B70
GL_DEPTH_WRITEMASK=0x0B72
GL_DEPTH_COMPONENT=0x1902
GL_LIGHTING=0x0B50
GL_LIGHT0=0x4000
GL_LIGHT1=0x4001
GL_LIGHT2=0x4002
GL_LIGHT3=0x4003
GL_LIGHT4=0x4004
GL_LIGHT5=0x4005
GL_LIGHT6=0x4006
GL_LIGHT7=0x4007
GL_SPOT_EXPONENT=0x1205
GL_SPOT_CUTOFF=0x1206
GL_CONSTANT_ATTENUATION=0x1207
GL_LINEAR_ATTENUATION=0x1208
GL_QUADRATIC_ATTENUATION=0x1209
GL_AMBIENT=0x1200
GL_DIFFUSE=0x1201
GL_SPECULAR=0x1202
GL_SHININESS=0x1601
GL_EMISSION=0x1600
GL_POSITION=0x1203
GL_SPOT_DIRECTION=0x1204
GL_AMBIENT_AND_DIFFUSE=0x1602
GL_COLOR_INDEXES=0x1603
GL_LIGHT_MODEL_TWO_SIDE=0x0B52
GL_LIGHT_MODEL_LOCAL_VIEWER=0x0B51
GL_LIGHT_MODEL_AMBIENT=0x0B53
GL_FRONT_AND_BACK=0x0408
GL_SHADE_MODEL=0x0B54
GL_FLAT=0x1D00
GL_SMOOTH=0x1D01
GL_COLOR_MATERIAL=0x0B57
GL_COLOR_MATERIAL_FACE=0x0B55
GL_COLOR_MATERIAL_PARAMETER=0x0B56
GL_NORMALIZE=0x0BA1
GL_CLIP_PLANE0=0x3000
GL_CLIP_PLANE1=0x3001
GL_CLIP_PLANE2=0x3002
GL_CLIP_PLANE3=0x3003
GL_CLIP_PLANE4=0x3004
GL_CLIP_PLANE5=0x3005
GL_ACCUM_RED_BITS=0x0D58
GL_ACCUM_GREEN_BITS=0x0D59
GL_ACCUM_BLUE_BITS=0x0D5A
GL_ACCUM_ALPHA_BITS=0x0D5B
GL_ACCUM_CLEAR_VALUE=0x0B80
GL_ACCUM=0x0100
GL_ADD=0x0104
GL_LOAD=0x0101
GL_MULT=0x0103
GL_RETURN=0x0102
GL_ALPHA_TEST=0x0BC0
GL_ALPHA_TEST_REF=0x0BC2
GL_ALPHA_TEST_FUNC=0x0BC1
GL_BLEND=0x0BE2
GL_BLEND_SRC=0x0BE1
GL_BLEND_DST=0x0BE0
GL_ZERO=0x0
GL_ONE=0x1
GL_SRC_COLOR=0x0300
GL_ONE_MINUS_SRC_COLOR=0x0301
GL_SRC_ALPHA=0x0302
GL_ONE_MINUS_SRC_ALPHA=0x0303
GL_DST_ALPHA=0x0304
GL_ONE_MINUS_DST_ALPHA=0x0305
GL_DST_COLOR=0x0306
GL_ONE_MINUS_DST_COLOR=0x0307
GL_SRC_ALPHA_SATURATE=0x0308
GL_CONSTANT_COLOR=0x8001
GL_ONE_MINUS_CONSTANT_COLOR=0x8002
GL_CONSTANT_ALPHA=0x8003
GL_ONE_MINUS_CONSTANT_ALPHA=0x8004
GL_FEEDBACK=0x1C01
GL_RENDER=0x1C00
GL_SELECT=0x1C02
GL_2D=0x0600
GL_3D=0x0601
GL_3D_COLOR=0x0602
GL_POINT_TOKEN=0x0701
GL_LINE_TOKEN=0x0702
GL_LINE_RESET_TOKEN=0x0707
GL_POLYGON_TOKEN=0x0703
GL_BITMAP_TOKEN=0x0704
GL_DRAW_PIXEL_TOKEN=0x0705
GL_COPY_PIXEL_TOKEN=0x0706
GL_PASS_THROUGH_TOKEN=0x0700
GL_FEEDBACK_BUFFER_POINTER=0x0DF0
GL_FEEDBACK_BUFFER_SIZE=0x0DF1
GL_FEEDBACK_BUFFER_TYPE=0x0DF2
GL_SELECTION_BUFFER_POINTER=0x0DF3
GL_SELECTION_BUFFER_SIZE=0x0DF4
GL_FOG=0x0B60
GL_FOG_MODE=0x0B65
GL_FOG_DENSITY=0x0B62
GL_FOG_COLOR=0x0B66
GL_FOG_INDEX=0x0B61
GL_FOG_START=0x0B63
GL_FOG_END=0x0B64
GL_LINEAR=0x2601
GL_EXP=0x0800
GL_EXP2=0x0801
GL_LOGIC_OP=0x0BF1
GL_INDEX_LOGIC_OP=0x0BF1
GL_COLOR_LOGIC_OP=0x0BF2
GL_LOGIC_OP_MODE=0x0BF0
GL_CLEAR=0x1500
GL_SET=0x150F
GL_COPY=0x1503
GL_COPY_INVERTED=0x150C
GL_NOOP=0x1505
GL_INVERT=0x150A
GL_AND=0x1501
GL_NAND=0x150E
GL_OR=0x1507
GL_NOR=0x1508
GL_XOR=0x1506
GL_EQUIV=0x1509
GL_AND_REVERSE=0x1502
GL_AND_INVERTED=0x1504
GL_OR_REVERSE=0x150B
GL_OR_INVERTED=0x150D
GL_STENCIL_TEST=0x0B90
GL_STENCIL_WRITEMASK=0x0B98
GL_STENCIL_BITS=0x0D57
GL_STENCIL_FUNC=0x0B92
GL_STENCIL_VALUE_MASK=0x0B93
GL_STENCIL_REF=0x0B97
GL_STENCIL_FAIL=0x0B94
GL_STENCIL_PASS_DEPTH_PASS=0x0B96
GL_STENCIL_PASS_DEPTH_FAIL=0x0B95
GL_STENCIL_CLEAR_VALUE=0x0B91
GL_STENCIL_INDEX=0x1901
GL_KEEP=0x1E00
GL_REPLACE=0x1E01
GL_INCR=0x1E02
GL_DECR=0x1E03
GL_NONE=0x0
GL_LEFT=0x0406
GL_RIGHT=0x0407
GL_FRONT_LEFT=0x0400
GL_FRONT_RIGHT=0x0401
GL_BACK_LEFT=0x0402
GL_BACK_RIGHT=0x0403
GL_AUX0=0x0409
GL_AUX1=0x040A
GL_AUX2=0x040B
GL_AUX3=0x040C
GL_COLOR_INDEX=0x1900
GL_RED=0x1903
GL_GREEN=0x1904
GL_BLUE=0x1905
GL_ALPHA=0x1906
GL_LUMINANCE=0x1909
GL_LUMINANCE_ALPHA=0x190A
GL_ALPHA_BITS=0x0D55
GL_RED_BITS=0x0D52
GL_GREEN_BITS=0x0D53
GL_BLUE_BITS=0x0D54
GL_INDEX_BITS=0x0D51
GL_SUBPIXEL_BITS=0x0D50
GL_AUX_BUFFERS=0x0C00
GL_READ_BUFFER=0x0C02
GL_DRAW_BUFFER=0x0C01
GL_DOUBLEBUFFER=0x0C32
GL_STEREO=0x0C33
GL_BITMAP=0x1A00
GL_COLOR=0x1800
GL_DEPTH=0x1801
GL_STENCIL=0x1802
GL_DITHER=0x0BD0
GL_RGB=0x1907
GL_RGBA=0x1908
GL_MAX_LIST_NESTING=0x0B31
GL_MAX_ATTRIB_STACK_DEPTH=0x0D35
GL_MAX_MODELVIEW_STACK_DEPTH=0x0D36
GL_MAX_NAME_STACK_DEPTH=0x0D37
GL_MAX_PROJECTION_STACK_DEPTH=0x0D38
GL_MAX_EVAL_ORDER=0x0D30
GL_MAX_LIGHTS=0x0D31
GL_MAX_CLIP_PLANES=0x0D32
GL_MAX_PIXEL_MAP_TABLE=0x0D34
GL_MAX_VIEWPORT_DIMS=0x0D3A
GL_MAX_CLIENT_ATTRIB_STACK_DEPTH=0x0D3B
GL_ATTRIB_STACK_DEPTH=0x0BB0
GL_CLIENT_ATTRIB_STACK_DEPTH=0x0BB1
GL_COLOR_CLEAR_VALUE=0x0C22
GL_COLOR_WRITEMASK=0x0C23
GL_CURRENT_INDEX=0x0B01
GL_CURRENT_COLOR=0x0B00
GL_CURRENT_NORMAL=0x0B02
GL_CURRENT_RASTER_COLOR=0x0B04
GL_CURRENT_RASTER_DISTANCE=0x0B09
GL_CURRENT_RASTER_INDEX=0x0B05
GL_CURRENT_RASTER_POSITION=0x0B07
GL_CURRENT_RASTER_POSITION_VALID=0x0B08
GL_INDEX_CLEAR_VALUE=0x0C20
GL_INDEX_MODE=0x0C30
GL_INDEX_WRITEMASK=0x0C21
GL_MODELVIEW_MATRIX=0x0BA6
GL_MODELVIEW_STACK_DEPTH=0x0BA3
GL_NAME_STACK_DEPTH=0x0D70
GL_PROJECTION_MATRIX=0x0BA7
GL_PROJECTION_STACK_DEPTH=0x0BA4
GL_RENDER_MODE=0x0C40
GL_RGBA_MODE=0x0C31
GL_VIEWPORT=0x0BA2
GL_AUTO_NORMAL=0x0D80
GL_MAP1_COLOR_4=0x0D90
GL_MAP1_GRID_DOMAIN=0x0DD0
GL_MAP1_GRID_SEGMENTS=0x0DD1
GL_MAP1_INDEX=0x0D91
GL_MAP1_NORMAL=0x0D92
GL_MAP1_VERTEX_3=0x0D97
GL_MAP1_VERTEX_4=0x0D98
GL_MAP2_COLOR_4=0x0DB0
GL_MAP2_GRID_DOMAIN=0x0DD2
GL_MAP2_GRID_SEGMENTS=0x0DD3
GL_MAP2_INDEX=0x0DB1
GL_MAP2_NORMAL=0x0DB2
GL_MAP2_VERTEX_3=0x0DB7
GL_MAP2_VERTEX_4=0x0DB8
GL_COEFF=0x0A00
GL_DOMAIN=0x0A02
GL_ORDER=0x0A01
GL_FOG_HINT=0x0C54
GL_LINE_SMOOTH_HINT=0x0C52
GL_PERSPECTIVE_CORRECTION_HINT=0x0C50
GL_POINT_SMOOTH_HINT=0x0C51
GL_POLYGON_SMOOTH_HINT=0x0C53
GL_DONT_CARE=0x1100
GL_FASTEST=0x1101
GL_NICEST=0x1102
GL_SCISSOR_TEST=0x0C11
GL_SCISSOR_BOX=0x0C10
GL_MAP_COLOR=0x0D10
GL_MAP_STENCIL=0x0D11
GL_INDEX_SHIFT=0x0D12
GL_INDEX_OFFSET=0x0D13
GL_RED_SCALE=0x0D14
GL_RED_BIAS=0x0D15
GL_GREEN_SCALE=0x0D18
GL_GREEN_BIAS=0x0D19
GL_BLUE_SCALE=0x0D1A
GL_BLUE_BIAS=0x0D1B
GL_ALPHA_SCALE=0x0D1C
GL_ALPHA_BIAS=0x0D1D
GL_DEPTH_SCALE=0x0D1E
GL_DEPTH_BIAS=0x0D1F
GL_PIXEL_MAP_S_TO_S_SIZE=0x0CB1
GL_PIXEL_MAP_I_TO_I_SIZE=0x0CB0
GL_PIXEL_MAP_I_TO_R_SIZE=0x0CB2
GL_PIXEL_MAP_I_TO_G_SIZE=0x0CB3
GL_PIXEL_MAP_I_TO_B_SIZE=0x0CB4
GL_PIXEL_MAP_I_TO_A_SIZE=0x0CB5
GL_PIXEL_MAP_R_TO_R_SIZE=0x0CB6
GL_PIXEL_MAP_G_TO_G_SIZE=0x0CB7
GL_PIXEL_MAP_B_TO_B_SIZE=0x0CB8
GL_PIXEL_MAP_A_TO_A_SIZE=0x0CB9
GL_PIXEL_MAP_S_TO_S=0x0C71
GL_PIXEL_MAP_I_TO_I=0x0C70
GL_PIXEL_MAP_I_TO_R=0x0C72
GL_PIXEL_MAP_I_TO_G=0x0C73
GL_PIXEL_MAP_I_TO_B=0x0C74
GL_PIXEL_MAP_I_TO_A=0x0C75
GL_PIXEL_MAP_R_TO_R=0x0C76
GL_PIXEL_MAP_G_TO_G=0x0C77
GL_PIXEL_MAP_B_TO_B=0x0C78
GL_PIXEL_MAP_A_TO_A=0x0C79
GL_PACK_ALIGNMENT=0x0D05
GL_PACK_LSB_FIRST=0x0D01
GL_PACK_ROW_LENGTH=0x0D02
GL_PACK_SKIP_PIXELS=0x0D04
GL_PACK_SKIP_ROWS=0x0D03
GL_PACK_SWAP_BYTES=0x0D00
GL_UNPACK_ALIGNMENT=0x0CF5
GL_UNPACK_LSB_FIRST=0x0CF1
GL_UNPACK_ROW_LENGTH=0x0CF2
GL_UNPACK_SKIP_PIXELS=0x0CF4
GL_UNPACK_SKIP_ROWS=0x0CF3
GL_UNPACK_SWAP_BYTES=0x0CF0
GL_ZOOM_X=0x0D16
GL_ZOOM_Y=0x0D17
GL_NEAREST_MIPMAP_NEAREST=0x2700
GL_NEAREST_MIPMAP_LINEAR=0x2702
GL_LINEAR_MIPMAP_NEAREST=0x2701
GL_LINEAR_MIPMAP_LINEAR=0x2703
GL_OBJECT_LINEAR=0x2401
GL_OBJECT_PLANE=0x2501
GL_EYE_LINEAR=0x2400
GL_EYE_PLANE=0x2502
GL_SPHERE_MAP=0x2402
GL_DECAL=0x2101
GL_MODULATE=0x2100
GL_NEAREST=0x2600
GL_REPEAT=0x2901
GL_CLAMP=0x2900
GL_S=0x2000
GL_T=0x2001
GL_R=0x2002
GL_Q=0x2003
GL_VENDOR=0x1F00
GL_RENDERER=0x1F01
GL_VERSION=0x1F02
GL_NO_ERROR=0x0
GL_INVALID_VALUE=0x0501
GL_INVALID_ENUM=0x0500
GL_INVALID_OPERATION=0x0502
GL_STACK_OVERFLOW=0x0503
GL_STACK_UNDERFLOW=0x0504
GL_OUT_OF_MEMORY=0x0505
GL_CURRENT_BIT=0x00000001
GL_POINT_BIT=0x00000002
GL_LINE_BIT=0x00000004
GL_POLYGON_BIT=0x00000008
GL_POLYGON_STIPPLE_BIT=0x00000010
GL_PIXEL_MODE_BIT=0x00000020
GL_LIGHTING_BIT=0x00000040
GL_FOG_BIT=0x00000080
GL_DEPTH_BUFFER_BIT=0x00000100
GL_ACCUM_BUFFER_BIT=0x00000200
GL_STENCIL_BUFFER_BIT=0x00000400
GL_VIEWPORT_BIT=0x00000800
GL_TRANSFORM_BIT=0x00001000
GL_ENABLE_BIT=0x00002000
GL_COLOR_BUFFER_BIT=0x00004000
GL_HINT_BIT=0x00008000
GL_EVAL_BIT=0x00010000
GL_LIST_BIT=0x00020000
GL_SCISSOR_BIT=0x00080000
GL_ALL_ATTRIB_BITS=0x000FFFFF
GL_ALPHA4=0x803B
GL_ALPHA8=0x803C
GL_ALPHA12=0x803D
GL_ALPHA16=0x803E
GL_LUMINANCE4=0x803F
GL_LUMINANCE8=0x8040
GL_LUMINANCE12=0x8041
GL_LUMINANCE16=0x8042
GL_LUMINANCE4_ALPHA4=0x8043
GL_LUMINANCE6_ALPHA2=0x8044
GL_LUMINANCE8_ALPHA8=0x8045
GL_LUMINANCE12_ALPHA4=0x8046
GL_LUMINANCE12_ALPHA12=0x8047
GL_LUMINANCE16_ALPHA16=0x8048
GL_INTENSITY=0x8049
GL_INTENSITY4=0x804A
GL_INTENSITY8=0x804B
GL_INTENSITY12=0x804C
GL_INTENSITY16=0x804D
GL_R3_G3_B2=0x2A10
GL_RGB4=0x804F
GL_RGB5=0x8050
GL_RGB8=0x8051
GL_RGB10=0x8052
GL_RGB12=0x8053
GL_RGB16=0x8054
GL_RGBA2=0x8055
GL_RGBA4=0x8056
GL_RGB5_A1=0x8057
GL_RGBA8=0x8058
GL_RGB10_A2=0x8059
GL_RGBA12=0x805A
GL_RGBA16=0x805B
GL_CLIENT_PIXEL_STORE_BIT=0x00000001
GL_CLIENT_VERTEX_ARRAY_BIT=0x00000002
GL_ALL_CLIENT_ATTRIB_BITS=0xFFFFFFFF
GL_CLIENT_ALL_ATTRIB_BITS=0xFFFFFFFF
GL_RESCALE_NORMAL=0x803A
GL_CLAMP_TO_EDGE=0x812F
GL_MAX_ELEMENTS_VERTICES=0x80E8
GL_MAX_ELEMENTS_INDICES=0x80E9
GL_BGR=0x80E0
GL_BGRA=0x80E1
GL_UNSIGNED_BYTE_3_3_2=0x8032
GL_UNSIGNED_BYTE_2_3_3_REV=0x8362
GL_UNSIGNED_SHORT_5_6_5=0x8363
GL_UNSIGNED_SHORT_5_6_5_REV=0x8364
GL_UNSIGNED_SHORT_4_4_4_4=0x8033
GL_UNSIGNED_SHORT_4_4_4_4_REV=0x8365
GL_UNSIGNED_SHORT_5_5_5_1=0x8034
GL_UNSIGNED_SHORT_1_5_5_5_REV=0x8366
GL_UNSIGNED_INT_8_8_8_8=0x8035
GL_UNSIGNED_INT_8_8_8_8_REV=0x8367
GL_UNSIGNED_INT_10_10_10_2=0x8036
GL_UNSIGNED_INT_2_10_10_10_REV=0x8368
GL_LIGHT_MODEL_COLOR_CONTROL=0x81F8
GL_SINGLE_COLOR=0x81F9
GL_SEPARATE_SPECULAR_COLOR=0x81FA
GL_SMOOTH_POINT_SIZE_RANGE=0x0B12
GL_SMOOTH_POINT_SIZE_GRANULARITY=0x0B13
GL_SMOOTH_LINE_WIDTH_RANGE=0x0B22
GL_SMOOTH_LINE_WIDTH_GRANULARITY=0x0B23
GL_ALIASED_POINT_SIZE_RANGE=0x846D
GL_ALIASED_LINE_WIDTH_RANGE=0x846E
GL_PACK_SKIP_IMAGES=0x806B
GL_PACK_IMAGE_HEIGHT=0x806C
GL_UNPACK_SKIP_IMAGES=0x806D
GL_UNPACK_IMAGE_HEIGHT=0x806E
GL_COLOR_TABLE=0x80D0
GL_POST_CONVOLUTION_COLOR_TABLE=0x80D1
GL_POST_COLOR_MATRIX_COLOR_TABLE=0x80D2
GL_PROXY_COLOR_TABLE=0x80D3
GL_PROXY_POST_CONVOLUTION_COLOR_TABLE=0x80D4
GL_PROXY_POST_COLOR_MATRIX_COLOR_TABLE=0x80D5
GL_COLOR_TABLE_SCALE=0x80D6
GL_COLOR_TABLE_BIAS=0x80D7
GL_COLOR_TABLE_FORMAT=0x80D8
GL_COLOR_TABLE_WIDTH=0x80D9
GL_COLOR_TABLE_RED_SIZE=0x80DA
GL_COLOR_TABLE_GREEN_SIZE=0x80DB
GL_COLOR_TABLE_BLUE_SIZE=0x80DC
GL_COLOR_TABLE_ALPHA_SIZE=0x80DD
GL_COLOR_TABLE_LUMINANCE_SIZE=0x80DE
GL_COLOR_TABLE_INTENSITY_SIZE=0x80DF
GL_CONVOLUTION_1D=0x8010
GL_CONVOLUTION_2D=0x8011
GL_SEPARABLE_2D=0x8012
GL_CONVOLUTION_BORDER_MODE=0x8013
GL_CONVOLUTION_FILTER_SCALE=0x8014
GL_CONVOLUTION_FILTER_BIAS=0x8015
GL_REDUCE=0x8016
GL_CONVOLUTION_FORMAT=0x8017
GL_CONVOLUTION_WIDTH=0x8018
GL_CONVOLUTION_HEIGHT=0x8019
GL_MAX_CONVOLUTION_WIDTH=0x801A
GL_MAX_CONVOLUTION_HEIGHT=0x801B
GL_POST_CONVOLUTION_RED_SCALE=0x801C
GL_POST_CONVOLUTION_GREEN_SCALE=0x801D
GL_POST_CONVOLUTION_BLUE_SCALE=0x801E
GL_POST_CONVOLUTION_ALPHA_SCALE=0x801F
GL_POST_CONVOLUTION_RED_BIAS=0x8020
GL_POST_CONVOLUTION_GREEN_BIAS=0x8021
GL_POST_CONVOLUTION_BLUE_BIAS=0x8022
GL_POST_CONVOLUTION_ALPHA_BIAS=0x8023
GL_CONSTANT_BORDER=0x8151
GL_REPLICATE_BORDER=0x8153
GL_CONVOLUTION_BORDER_COLOR=0x8154
GL_COLOR_MATRIX=0x80B1
GL_COLOR_MATRIX_STACK_DEPTH=0x80B2
GL_MAX_COLOR_MATRIX_STACK_DEPTH=0x80B3
GL_POST_COLOR_MATRIX_RED_SCALE=0x80B4
GL_POST_COLOR_MATRIX_GREEN_SCALE=0x80B5
GL_POST_COLOR_MATRIX_BLUE_SCALE=0x80B6
GL_POST_COLOR_MATRIX_ALPHA_SCALE=0x80B7
GL_POST_COLOR_MATRIX_RED_BIAS=0x80B8
GL_POST_COLOR_MATRIX_GREEN_BIAS=0x80B9
GL_POST_COLOR_MATRIX_BLUE_BIAS=0x80BA
GL_POST_COLOR_MATRIX_ALPHA_BIAS=0x80BB
GL_HISTOGRAM=0x8024
GL_PROXY_HISTOGRAM=0x8025
GL_HISTOGRAM_WIDTH=0x8026
GL_HISTOGRAM_FORMAT=0x8027
GL_HISTOGRAM_RED_SIZE=0x8028
GL_HISTOGRAM_GREEN_SIZE=0x8029
GL_HISTOGRAM_BLUE_SIZE=0x802A
GL_HISTOGRAM_ALPHA_SIZE=0x802B
GL_HISTOGRAM_LUMINANCE_SIZE=0x802C
GL_HISTOGRAM_SINK=0x802D
GL_MINMAX=0x802E
GL_MINMAX_FORMAT=0x802F
GL_MINMAX_SINK=0x8030
GL_TABLE_TOO_LARGE=0x8031
GL_BLEND_EQUATION=0x8009
GL_MIN=0x8007
GL_MAX=0x8008
GL_FUNC_ADD=0x8006
GL_FUNC_SUBTRACT=0x800A
GL_FUNC_REVERSE_SUBTRACT=0x800B
GL_BLEND_COLOR=0x8005
GL_NORMAL_MAP=0x8511
GL_REFLECTION_MAP=0x8512
GL_COMPRESSED_ALPHA=0x84E9
GL_COMPRESSED_LUMINANCE=0x84EA
GL_COMPRESSED_LUMINANCE_ALPHA=0x84EB
GL_COMPRESSED_INTENSITY=0x84EC
GL_COMPRESSED_RGB=0x84ED
GL_COMPRESSED_RGBA=0x84EE
GL_MULTISAMPLE=0x809D
GL_SAMPLE_ALPHA_TO_COVERAGE=0x809E
GL_SAMPLE_ALPHA_TO_ONE=0x809F
GL_SAMPLE_COVERAGE=0x80A0
GL_SAMPLE_BUFFERS=0x80A8
GL_SAMPLES=0x80A9
GL_SAMPLE_COVERAGE_VALUE=0x80AA
GL_SAMPLE_COVERAGE_INVERT=0x80AB
GL_MULTISAMPLE_BIT=0x20000000
GL_TRANSPOSE_MODELVIEW_MATRIX=0x84E3
GL_TRANSPOSE_PROJECTION_MATRIX=0x84E4
GL_TRANSPOSE_COLOR_MATRIX=0x84E6
GL_COMBINE=0x8570
GL_COMBINE_RGB=0x8571
GL_COMBINE_ALPHA=0x8572
GL_SOURCE0_RGB=0x8580
GL_SOURCE1_RGB=0x8581
GL_SOURCE2_RGB=0x8582
GL_SOURCE0_ALPHA=0x8588
GL_SOURCE1_ALPHA=0x8589
GL_SOURCE2_ALPHA=0x858A
GL_OPERAND0_RGB=0x8590
GL_OPERAND1_RGB=0x8591
GL_OPERAND2_RGB=0x8592
GL_OPERAND0_ALPHA=0x8598
GL_OPERAND1_ALPHA=0x8599
GL_OPERAND2_ALPHA=0x859A
GL_RGB_SCALE=0x8573
GL_ADD_SIGNED=0x8574
GL_INTERPOLATE=0x8575
GL_SUBTRACT=0x84E7
GL_CONSTANT=0x8576
GL_PRIMARY_COLOR=0x8577
GL_PREVIOUS=0x8578
GL_DOT3_RGB=0x86AE
GL_DOT3_RGBA=0x86AF
GL_CLAMP_TO_BORDER=0x812D
GL_CLAMP_TO_EDGE_SGIS=0x812F
GL_OCCLUSION_TEST_HP=0x8165
GL_OCCLUSION_TEST_RESULT_HP=0x8166
GL_NORMAL_MAP_NV=0x8511
GL_REFLECTION_MAP_NV=0x8512
GL_UNPACK_CLIENT_STORAGE_APPLE=0x85B2
GL_YCBCR_422_APPLE=0x85B9
GL_UNSIGNED_SHORT_8_8_APPLE=0x85BA
GL_UNSIGNED_SHORT_8_8_REV_APPLE=0x85BB
GL_TEXTURE_ENV=0x2300
GL_TEXTURE_ENV_MODE=0x2200
GL_TEXTURE_1D=0x0DE0
GL_TEXTURE_2D=0x0DE1
GL_TEXTURE_WRAP_S=0x2802
GL_TEXTURE_WRAP_T=0x2803
GL_TEXTURE_MAG_FILTER=0x2800
GL_TEXTURE_MIN_FILTER=0x2801
GL_TEXTURE_ENV_COLOR=0x2201
GL_TEXTURE_GEN_S=0x0C60
GL_TEXTURE_GEN_T=0x0C61
GL_TEXTURE_GEN_MODE=0x2500
GL_TEXTURE_BORDER_COLOR=0x1004
GL_TEXTURE_WIDTH=0x1000
GL_TEXTURE_HEIGHT=0x1001
GL_TEXTURE_BORDER=0x1005
GL_TEXTURE_COMPONENTS=0x1003
GL_TEXTURE_RED_SIZE=0x805C
GL_TEXTURE_GREEN_SIZE=0x805D
GL_TEXTURE_BLUE_SIZE=0x805E
GL_TEXTURE_ALPHA_SIZE=0x805F
GL_TEXTURE_LUMINANCE_SIZE=0x8060
GL_TEXTURE_INTENSITY_SIZE=0x8061
GL_NEAREST_MIPMAP_NEAREST=0x2700
GL_NEAREST_MIPMAP_LINEAR=0x2702
GL_LINEAR_MIPMAP_NEAREST=0x2701
GL_LINEAR_MIPMAP_LINEAR=0x2703
GL_OBJECT_LINEAR=0x2401
GL_OBJECT_PLANE=0x2501
GL_EYE_LINEAR=0x2400
GL_EYE_PLANE=0x2502
GL_SPHERE_MAP=0x2402
GL_DECAL=0x2101
GL_MODULATE=0x2100
GL_NEAREST=0x2600
GL_REPEAT=0x2901
GL_CLAMP=0x2900
GL_S=0x2000
GL_T=0x2001
GL_R=0x2002
GL_Q=0x2003
GL_TEXTURE_GEN_R=0x0C62
GL_TEXTURE_GEN_Q=0x0C63

0;
