(function() {
  // load native library
  if (this.Input) return;
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsinput.la");
  if (!fname) throw new Error("Could not find module: 'ejsinput.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejsinput",fname.substring(0,fname.lastIndexOf(".")));
  
  //! todo: this must always work => untrusted code should not be allowed to mess with this
  Input.handleQuit=function(){
    ejs.exit(true);
  };
  Input.toggleFullscreen=function(){
    Video.toggleFullscreen();
  };
 })();
