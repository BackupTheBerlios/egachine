(function(){
  // module depends on the Stream module
  if (!this.Stream) ejs.ModuleLoader.load("Stream");
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsfile.la");
  if (!fname) throw new Error("Could not find module: 'ejsfile.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejsfile",fname.substring(0,fname.lastIndexOf(".")));
  File.mode={
    read:1,
    write:2,
    trunc:4,
    append:8,
    binary:16,
    ate:32
  };
  File.read=function(name){
    return File.open(name,File.mode.read);
  }
  File.write=function(name){
    return File.open(name,File.mode.write|File.mode.trunc);
  }
 })();
