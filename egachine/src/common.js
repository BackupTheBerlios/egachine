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
   \file common.js
   \brief script library
   \author Jens Thiele
*/

// this script is evaluated from all programs in the egachine distribution

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

// misc extensions
Number.prototype.convertTo=function(base,padTo){
    var s=this.toString(base);
    if (!padTo || s.length>=padTo) throw new Error("Does not fit");
    return Math.pow(10,padTo-s.length).toString().slice(1)+s;
}

//! Resource object
function Resource(resname,resource) {
  this.name=resname;
  this.size=resource.length;

  var comp=Base64.encode(Zlib.compress(resource));
  var uncomp=Base64.encode(resource);
  if (comp.length<uncomp.length) {
    this.zip=true;
    this.data=comp;
  }else{
    this.zip=false;
    this.data=uncomp;
  }
}

Resource.prototype.decode=function(){
  if (this.zip)
    return Zlib.decompress(Base64.decode(this.data),this.size);
  return Base64.decode(this.data);
}

//! EGachine object
EGachine={};
EGachine.r={};
EGachine.addResource=function(name,res){
  if (res!=undefined){
    // legacy case (TODO: remove it)
    print("legacy resource:"+name+" this is deprecated");
    EGachine.r[name]=new Resource(name,Base64.decode(res));
  }else if (typeof(name) == 'object') {
    var resource=name;
    resource.__proto__=Resource.prototype;
    EGachine.r[resource.name]=resource;
  }
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
