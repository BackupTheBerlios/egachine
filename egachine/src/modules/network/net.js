(function(){
  // Net module depends on the Stream module
  if (!this.Stream) ejs.ModuleLoader.load("Stream");
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsnet.la");
  if (!fname) throw new Error("Could not find module: 'ejsnet.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejsnet",fname.substring(0,fname.lastIndexOf(".")));
  Net._loadedScript=true;
 })();
