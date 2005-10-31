(function(ltdl){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsltdl.la");
  if (!fname) throw new Error("Could not find module: 'ejsltdl.la'");
  ejs.ModuleLoader.loadNative.call(ltdl,"ejsltdl",fname.substring(0,fname.lastIndexOf(".")));

  // ATTENTION/TODO: keep this in sync with js API!
  ltdl.defaultFlags=0;
 })(this);

