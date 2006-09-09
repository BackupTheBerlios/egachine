(function(tmp){
  // utilities for (hopefully) secure temporary file/directory creation
  tmp.getDefaultTempDir=function() {
    var tmpdir=ejs.ModuleLoader.get("util").getenv("TMPDIR");
    if (!tmpdir) tmpdir=ejs.ModuleLoader.get("File").getP_tmpdir();
    if (!tmpdir) tmpdir="/tmp";
    return tmpdir;
  }    
  tmp.mkdir=function(name){
    return mkdtemp(File.getDefaultTempDir()+"/"+name+"XXXXXXXXXX");
  }
 })(this);
