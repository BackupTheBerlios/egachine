(function() {
  // load native library
  if (this.Audio) return;
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsaudio.la");
  if (!fname) throw new Error("Could not find module: 'ejsaudio.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejsaudio",fname.substring(0,fname.lastIndexOf(".")));
 })();
