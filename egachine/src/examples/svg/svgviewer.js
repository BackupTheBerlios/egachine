(function(){
  var gl=ejs.ModuleLoader.get("gl");
  var EGachine=ejs.ModuleLoader.get("EGachine");
  var Input=ejs.ModuleLoader.get("Input");
  var Timer=ejs.ModuleLoader.get("Timer");
  var svgl=ejs.ModuleLoader.get("svgl");
  var File=ejs.ModuleLoader.get("File");
  var stderr=ejs.ModuleLoader.get("Stream").stderr;
  var Video=ejs.ModuleLoader.get("Video");

  if (!gl.GetIntegerv(GL_STENCIL_BITS)[0])
    stderr.write("Warning no stencil buffer\n");

  reschedule=function(sec){
    if (sec<0) throw sec;
    var start=Timer.getTimeStamp();
    var dt;
    Input.poll();
    if (!sec) sec=0.01;
    while ((dt=((Timer.getTimeStamp()-start)/1000000))<sec)
      Timer.uSleep((sec-dt)*1000000);
    return dt;
  }

  redisplay=function()
  {
    gl.Clear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    svgl.display(document);
    Video.swapBuffers();
  }

  SVGViewer={
    debug:function(x){stderr.write(x+"\n");},
    run:function(){
      var frame=0,start,last,now,dt,i,timeOut;
      last=now=start=Timer.getTimeStamp();
      while(true) {
	now=Timer.getTimeStamp();
	dt=now-last;
	last=now;
	gl.Clear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

	if (this.timeOut) {
	  // process timeout
	  timeOut=this.timeOut;
	  if ((timeOut.remain-=dt)<0) {
	    this.timeOut=false;
	    timeOut.func();
	  }
	}

	Input.poll();
	svgl.display(document);
	Video.swapBuffers();
	++frame;
      };
    }
  };

  if (argv.length<2) {
    stderr.write("Usage: "+argv[0]+" FILE\n");
    ejs.exit(false);
  };

  stream=File.read(argv[1]);
  if (stream.inAvailable()<=0)
    throw Error("Could not open file: "+argv[1]);
  document=new svgl.SVGDocument(stream.read(stream.inAvailable()));
  document.documentElement=document.getDocumentElement();

  /*
    hmm
    dom level 2 does not have keyboard events?
    dom level 3 events is only a note?
  
    Input.handleChar=function(c){
    // todo: this is not yet complete ;-)
    if (SVGViewer.onKeyDown) SVGViewer.onKeyDown({
    preventDefault:function(){},
    keyCode:c.charCodeAt(0)
    });
    };
    document.documentElement.addEventListener=function(event, handler, bx){
    // bx = ?
    if (event == "keydown") {
    SVGViewer.onKeyDown=handler;
    }else{
    stderr.write("Warning: event: "+event+" not supported");
    }
    };
  */

  setTimeout=function(toeval,ms) {
    // todo: should it be possible to set multiple timeouts?
    SVGViewer.timeOut={func:function(){eval(toeval);},timeOut:ms*1000,remain:ms*1000};
  };
  document._handleScripts(this);
  svgl.startAnimation(document);
  SVGViewer.run();
 })();
