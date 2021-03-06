(function(util){
  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsutil.la");
  if (!fname) throw new Error("Could not find module: 'ejsutil.la'");
  ejs.ModuleLoader.loadNative.call(util,"ejsutil",fname.substring(0,fname.lastIndexOf(".")));

  //! interactive eval 
  /*!
    1. reads JavaScript code from istream until completion of a compilable unit
    2. evaluates JavaScript code
    3. writes result to ostream and errors to estream
    4. repeat with 1.
  */
  util.ieval=function(istream,ostream,estream){
    function readline(){
      var res="";
      var c;
      do {
	c=istream.read(1);
	res+=c;
      }while(c && (c!="\n"));
      return res;
    }

    while (true) {
      ostream.write("ejs> ");
      ostream.sync();
      var buffer="";
      var got;
      do{
	buffer+=readline();
      }while (!util.isCompilableUnit(buffer))
	if (!buffer.length) break;
      try{
	ejs.lastResult=eval(buffer);
	ostream.write(ejs.lastResult+"\n");
      }catch(error){
	estream.write(error+"\n");
	ejs.lastResult=undefined;
	ejs.lastError=error;
      }
    }
  };

  //! get current stack as string
  util.getStack=function(){
    function chopLine(x){
      return x.substring(x.indexOf("\n")+1,x.length);
    };
    return chopLine(chopLine(new Error().stack));
  };
 })(this);
