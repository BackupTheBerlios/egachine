(function(){
  if (this.svgl) return;

  // we need video module
  if (!this.Video) ejs.ModuleLoader.load("Video");

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejssvgl.la");
  if (!fname) throw new Error("Could not find module: 'ejssvgl.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejssvgl",fname.substring(0,fname.lastIndexOf(".")));
 })();
