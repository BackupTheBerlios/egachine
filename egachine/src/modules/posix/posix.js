(function(posix){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsposix.la");
  if (!fname) throw new Error("Could not find module: 'ejsposix.la'");
  ejs.ModuleLoader.loadNative.call(posix,"ejsposix",fname.substring(0,fname.lastIndexOf(".")));
 })(this);
