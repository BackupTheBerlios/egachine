(function(svgl){
  var Video=ejs.ModuleLoader.get("Video");
  var Input=ejs.ModuleLoader.get("Input");

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejssvgl.la");
  if (!fname) throw new Error("Could not find module: 'ejssvgl.la'");
  ejs.ModuleLoader.loadNative.call(svgl,"ejssvgl",fname.substring(0,fname.lastIndexOf(".")));

  svgl.SVGDocument.prototype.addEventListener=function(type, listener, useCapture) {
    Input.addEventListener(type, listener, useCapture);
  };

  //! simple SVG viewer
  svgl.viewFile=function(fileName) {
    var gl=ejs.ModuleLoader.get("gl");
    var Timer=ejs.ModuleLoader.get("Timer");
    var svgl=ejs.ModuleLoader.get("svgl");
    var File=ejs.ModuleLoader.get("File");
    var stderr=ejs.ModuleLoader.get("Stream").stderr;

    var timeOut;

    Video.showMouseCursor(1);
    
    function mousemove(e) {
      // todo: panning / zooming
    };
    
    function mousedown(e) {
      var elem,i,j,picked;
      switch(e.button) {
      case 0:
	// picking
	picked=svgl.pick(e.screenX,e.screenY);
	stderr.write("pick: "+e.toSource()+":"+(picked ? "" : "nothing")+"\n");
	for (i=0;i<picked.length;++i) {
	  elem=picked[i];
	  for (j=0;j<elem.length;++j) {
	    stderr.write(elem[j].getNodeName()+" ");
	  }
	  stderr.write("\n");
	}
	break;
      };
    };
    
    Input.addEventListener("mousemove",mousemove);
    //  Input.addEventListener("mouseup",mouseup);
    Input.addEventListener("mousedown",mousedown);
    
    if (!gl.GetIntegerv(GL_STENCIL_BITS)[0])
      stderr.write("Warning no stencil buffer => output will be corrupt (perhaps you have to switch your color depth)\n");
    
    function getEvents(dt) {
      var tmp;
      if (timeOut) {
	// process timeout
	if ((timeOut.remain-=dt)<0) {
	  tmp=timeOut;
	  timeout=false;
	  tmp.func();
	}
      }
      Input.poll();
      return true;
    };
    
    function step(f,time) {
      var stepSize=1000000/85*2;
      if (time && (stepSize>time)) stepSize=time;
      
      var start,last,now;
      start=last=now=Timer.getTimeStamp();
      while ((!time)||(now-start<time)) {
	Timer.uSleep(stepSize);
	now=Timer.getTimeStamp();
	if (!f(now-last)) break;
	last=now;
      };
      return now-start;
    };
    
    //! called from native code (animation running)
    reschedule=function(sec){
      if (sec<0) throw sec;
      if (!sec) sec=0.001;
      return step(getEvents,sec*1000000)/1000000;
    };

    //! called from native code (animation running) todo: should not be global
    redisplay=function()
      {
	gl.Clear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	svgl.display();
	Video.swapBuffers();
      };

    var stream=File.read(fileName);
    if (stream.inAvailable()<=0)
      throw Error("Could not open file: "+argv[1]);
    
    // todo: hmm global
    document=new svgl.SVGDocument(stream.read(stream.inAvailable()));
    document.documentElement=document.getDocumentElement();
    svgl.selectDocument(document);

    // todo: hmm global
    setTimeout=function(toeval,ms) {
      // todo: should it be possible to set multiple timeouts?
      timeOut={func:function(){eval(toeval);},timeOut:ms*1000,remain:ms*1000};
    };
    
    document._handleScripts(this);
    svgl.startAnimation(document);
    // animation finished
    
    step(function(dt){redisplay();return getEvents(dt);});
  };

 })(this);
