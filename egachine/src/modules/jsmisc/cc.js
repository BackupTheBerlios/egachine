(function(cc){
  // bind modules we depend upon
  // todo: we should have a standard (declarative) way to define module dependencies
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

  // create a uniq function name
  cc.mkFuncName = function () {
    ++_id;
    return "ejscc_"+_id;
  }

  // compile, link and dynamically load c-code and return function named cfunc as JS function
  cc.compile2 = function(cfunc, code) {
    var stderr=Stream.stderr;
    var tmpdir=tmp.mkdir("cc");
    var cfile=File.write(tmpdir+"/c.c");
    // todo: do not use internal spidermonkey knowledge here (setting rval)
    cfile.write(code);
    cfile.close();
    var libname=tmpdir+"/c"+_id+".so";
    var cmd=ejs.config.cc.compileModule(tmpdir+"/c.c",libname)+" >"+tmpdir+"/stdout 2>"+tmpdir+"/stderr";
    var module;
    var error;
    try{
      stderr.write("Running: '"+cmd+"'\n");
      if (posix.system(cmd))
	throw Error("Compilation failed:\n"+File.read(tmpdir+"/c.c").readAll()+"\n"+File.read(tmpdir+"/stderr").readAll());
      else{
	stderr.write("Compiled successfully:\n"+File.read(tmpdir+"/stdout").readAll()+File.read(tmpdir+"/stderr").readAll());
      }
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
    if (error) throw error;
    return module.getWrapper(cfunc);
  }

  // compile c function body to JS function
  cc.compile = function(body, includes) {
    var funcname=mkFuncName();
    // todo: do not use internal spidermonkey knowledge here (setting rval)
    return cc.compile2(funcname, (includes ? includes : "") + "\n"
		       +"int "+funcname+"(void* ejs_cx, void* ejs_obj, unsigned ejs_argv, void* ejs_argc, int* ejs_rval) {\n"
		       + body
		       + "\n*ejs_rval=(1<<3)|0x6;return 1;\n"
		       + "}\n\n");
  }

 })(this);
