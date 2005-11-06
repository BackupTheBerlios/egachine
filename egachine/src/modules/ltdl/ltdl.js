(function(ltdl){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsltdl.la");
  if (!fname) throw new Error("Could not find module: 'ejsltdl.la'");
  ejs.ModuleLoader.loadNative.call(ltdl,"ejsltdl",fname.substring(0,fname.lastIndexOf(".")));
  /*
  ltdl.Ltmodule.prototype._getWrapper=ltdl.Ltmodule.prototype.getWrapper;
  ltdl.Ltmodule.prototype.getWrapper=function(funcName, args){
    return this._getWrapper(funcName,(args==undefined) ? 0 : args);
  }
  */
 })(this);

