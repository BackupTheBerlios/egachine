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

(function(EGachine){
  // load other required modules
  var Base64 = ejs.ModuleLoader.get("Base64");
  var Zlib   = ejs.ModuleLoader.get("Zlib");
  var Timer  = ejs.ModuleLoader.get("Timer");
  var Stream = ejs.ModuleLoader.get("Stream");
  var Util   = ejs.ModuleLoader.get("Util");
  var lang   = ejs.ModuleLoader.get("jsolait").importModule("lang");

  EGachine.objToJson=lang.objToJson;
  EGachine.jsonToObj=lang.jsonToObj;

  // todo: remove this 
  Number.prototype.convertTo=function(base,padTo){
    var s=this.toString(base);
    if (!padTo || s.length>=padTo) throw new Error("Does not fit");
    return Math.pow(10,padTo-s.length).toString().slice(1)+s;
  };

  // todo: remove
  findLastNotGreater=function(at,length,v){
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

  EGachine.Version=function(version) {
    var a;
    if (arguments.length==3) {
      this.maj=arguments[0];
      this.min=arguments[1];
      this.mic=arguments[2];
    }else if (version instanceof EGachine.Version) {
      this.maj=version.maj;
      this.min=version.min;
      this.mic=version.mic;
    }else if (typeof version == "string") {
      a=version.split(".");
      this.maj=a[0];
      this.min=a[1];
      this.mic=a[2];
    }else{
      throw Error("String, Version object or maj,min,mic required as argument(s)");
    }
  };

  EGachine.Version.prototype.compatible=function(version) {
    var required;
    if (arguments.length==3)
      required=new EGachine.Version(arguments[0],arguments[1],arguments[2]);
    else
      required=new EGachine.Version(version);
    if (required.maj != this.maj) return false;
    if (required.min<this.min) return true;
    if (required.min>this.min) return false;
    if (required.mic<this.mic) return true;
    if (required.mic>this.mic) return false;
    return true;
  };

  //! Resource object
  EGachine.Resource=function(resname,resource) {
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
  };

  EGachine.Resource.prototype.decode=function(){
    if (this.z) return Zlib.uncompress(Base64.decode(this.data),this.size);
    return Base64.decode(this.data);
  };

  EGachine.Resource.prototype.toString=function(){
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
  EGachine.checkVersion=function() {
    if (!EGachine.version.compatible.apply(EGachine.version,arguments))
      throw "Required version: "+requiredVersion+" is incompatible with available version: "+EGachine.version;
    return true;
  };

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsegachine.la");
  if (!fname) throw new Error("Could not find module: 'ejsegachine.la'");
  ejs.ModuleLoader.loadNative.call(EGachine,"ejsegachine",fname.substring(0,fname.lastIndexOf(".")));
})(this);
