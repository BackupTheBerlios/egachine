(function(tmp){
  var File=ejs.ModuleLoader.get("File");
  // utilities for (hopefully) secure temporary file/directory creation
  tmp.getDefaultTempDir=function() {
    var tmpdir=ejs.ModuleLoader.get("util").getenv("TMPDIR");
    if (!tmpdir) tmpdir=File.getP_tmpdir();
    if (!tmpdir) tmpdir="/tmp";
    return tmpdir;
  }    
  tmp.mkdir=function(name){
    return File.mkdtemp(tmp.getDefaultTempDir()+"/"+name+"XXXXXXXXXX");
  }
 })(this);
