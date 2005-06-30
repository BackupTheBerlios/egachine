(function(tcc){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejstcc.la");
  if (!fname) throw new Error("Could not find module: 'ejstcc.la'");
  ejs.ModuleLoader.loadNative.call(tcc,"ejstcc",fname.substring(0,fname.lastIndexOf(".")));

  tcc._run = tcc.run;
  tcc.run = function(x) {
    tcc._run("\
#include <tcclib.h>\n				\
void ejstcc_compiled_function(){\n"
	     +x
	     +"\n\
}");
  }
 })(this);
