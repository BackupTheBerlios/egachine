(function(mod){
  var Video=ejs.ModuleLoader.get("Video");

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejssvgl.la");
  if (!fname) throw new Error("Could not find module: 'ejssvgl.la'");
  ejs.ModuleLoader.loadNative.call(mod,"ejssvgl",fname.substring(0,fname.lastIndexOf(".")));
 })(this);
