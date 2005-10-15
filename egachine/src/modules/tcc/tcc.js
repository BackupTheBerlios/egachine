(function(tcc){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejstcc.la");
  if (!fname) throw new Error("Could not find module: 'ejstcc.la'");
  ejs.ModuleLoader.loadNative.call(tcc,"ejstcc",fname.substring(0,fname.lastIndexOf(".")));

  tcc.run = function(x, includes) {
    var tccState=new tcc.TCCState();
    tccState.compile( (includes ? includes : "#include <tcclib.h>\n")
	      + "void ejstcc_compiled_function(){\n"
	      + x
	      + "\n\
}");
    tccState.relocate();
    tccState.callVV("ejstcc_compiled_function");
  }
  /*
    tcc.wrap = function(fname, fbody, includes) {
    // TODO: platform dependant and requires spidermonkey headers to be installed
    tcc.compile((includes ? includes : "#define XP_UNIX\n#include <smjs/jsapi.h>\n")
    + "JSBool wrap_"+fname+"(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) {\n"
    + fbody
    + "\nreturn JS_TRUE;\n"
    + "}\n"
    );
    }
  */
 })(this);

