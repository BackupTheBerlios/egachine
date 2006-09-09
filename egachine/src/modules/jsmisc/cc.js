(function(cc){
  var util=ejs.ModuleLoader.get("util");
  var Stream=ejs.ModuleLoader.get("Stream");
  var File=ejs.ModuleLoader.get("File");
  var tmp=ejs.ModuleLoader.get("tmp");
  var posix=ejs.ModuleLoader.get("posix");
  var ltdl=ejs.ModuleLoader.get("ltdl");

  // compilation id (todo: use string to avoid "overflow")
  var _id=0;

  // compile and run c-code
  cc.run = function(body, includes) {
    var ret=compile(body, includes)();
    util.GC();
    return ret;
  }
  // compile c function
  cc.compile = function(body, includes) {
    var stderr=Stream.stderr;

    var tmpdir=tmp.mkdir("cc");
    var cfile=File.write(tmpdir+"/c.c");
    _id++;
    var funcname="ejscc_"+_id;
    // todo: do not use internal spidermonkey knowledge here (setting rval)
    cfile.write((includes ? includes : "") + "\n"
		+"int "+funcname+"(void* ejs_cx, void* ejs_obj, unsigned ejs_argv, void* ejs_argc, int* ejs_rval) {\n"
		+ body
		+ "\n*ejs_rval=(1<<3)|0x6;return 1;\n"
		+ "}\n\n");
    cfile.close();
    var libname=tmpdir+"/c"+_id+".so";
    var cmd="gcc -Wall -Werror -fpic -o "+libname+" -shared "+tmpdir+"/c.c >"+tmpdir+"/stdout 2>"+tmpdir+"/stderr";
    //    stderr.write(cmd+"\n");
    var module;
    var error;
    try{
      if (posix.system(cmd))
	throw Error("Compilation failed:\n"+File.read(tmpdir+"/stderr").readAll());
      module=new ltdl.Ltmodule(libname);
    }catch(e){
      error=e;
    }finally {
      File.unlink(tmpdir+"/c.c");
      File.unlink(tmpdir+"/stdout");
      File.unlink(tmpdir+"/stderr");
      // todo: probably will not work on win32
      File.unlink(libname);
      File.rmdir(tmpdir);
    }
    if (error)
      throw error;
    return module.getWrapper(funcname);
  }

 })(this);
