(function(cc){
  cc._id=0;
  // compile and run c-code
  cc.run = function(body, includes) {
    var util=ejs.ModuleLoader.get("util");

    var ret=compile(body, includes)();
    util.GC();
    return ret;
  }
  cc.compile = function(body, includes) {
    var Stream=ejs.ModuleLoader.get("Stream");
    var File=ejs.ModuleLoader.get("File");
    var tmp=ejs.ModuleLoader.get("tmp");
    var posix=ejs.ModuleLoader.get("posix");
    var ltdl=ejs.ModuleLoader.get("ltdl");
    var stderr=Stream.stderr;

    var tmpdir=tmp.mkdir("cc");
    var cfile=File.write(tmpdir+"/c.c");
    cc._id++;
    var funcname="ejscc_"+cc._id;
    // todo: do not use internal spidermonkey knowledge here (setting rval)
    cfile.write((includes ? includes : "") + "\n"
		+"void "+funcname+"(void* ejs_cx, void* ejs_obj, unsigned ejs_argv, void* ejs_argc, int* ejs_rval) {\n"
		+ body
		+ "\n*ejs_rval=(1<<3)|0x6;return 1;\n"
		+ "}\n\n");
    cfile.close();
    var libname=tmpdir+"/c"+cc._id+".so";
    var cmd="gcc -fpic -o "+libname+" -shared "+tmpdir+"/c.c >"+tmpdir+"/stdout 2>"+tmpdir+"/stderr";
    //    stderr.write(cmd+"\n");
    if (posix.system(cmd))
      stderr.write("Compilation failed:\n"+File.read(tmpdir+"/stderr").readAll());
    var module=new ltdl.Ltmodule(libname);
    File.unlink(tmpdir+"/c.c");
    File.unlink(tmpdir+"/stdout");
    File.unlink(tmpdir+"/stderr");
    File.unlink(libname);
    File.rmdir(tmpdir);
    return module.getWrapper(funcname);
  }

 })(this);
