(function(){
  // module depends on the Stream module
  if (!this.Stream) ejs.ModuleLoader.load("Stream");
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsfile.la");
  if (!fname) throw new Error("Could not find module: 'ejsfile.la'");
  ejs.ModuleLoader.loadNative.call(ejs.getGlobal(),"ejsfile",fname.substring(0,fname.lastIndexOf(".")));

  //! File open modes
  File.mode={
    read:1,
    write:2,
    trunc:4,
    append:8,
    binary:16,
    ate:32
  };

  //! create Stream for reading from a file
  /*!
    \param name filename of file to open for reading
    \return Stream object
  */
  File.read=function(name){
    return File.open(name,File.mode.read);
  }
  //! create Stream for writing to a file
  /*!
    \param name filename of file to open for reading
    \return Stream object
  */
  File.write=function(name){
    return File.open(name,File.mode.write|File.mode.trunc);
  }
 })();
