(function(){
  var gl=ejs.ModuleLoader.get("gl");
  var EGachine=ejs.ModuleLoader.get("EGachine");
  var Input=ejs.ModuleLoader.get("Input");
  var Timer=ejs.ModuleLoader.get("Timer");
  var svgl=ejs.ModuleLoader.get("svgl");
  var File=ejs.ModuleLoader.get("File");
  var stderr=ejs.ModuleLoader.get("Stream").stderr;
  var Video=ejs.ModuleLoader.get("Video");
  var timeOut;

  if (!gl.GetIntegerv(GL_STENCIL_BITS)[0])
    stderr.write("Warning no stencil buffer\n");

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
  }

  //! called from native code (animation running)
  reschedule=function(sec){
    if (sec<0) throw sec;
    if (!sec) sec=0.001;
    return step(getEvents,sec*1000000)/1000000;
  }

  //! called from native code (animation running)
  redisplay=function()
  {
    gl.Clear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    svgl.display();
    Video.swapBuffers();
  }

  if (argv.length<2) {
    stderr.write("Usage: "+argv[0]+" FILE\n");
    ejs.exit(false);
  };

  var stream=File.read(argv[1]);
  if (stream.inAvailable()<=0)
    throw Error("Could not open file: "+argv[1]);

  document=new svgl.SVGDocument(stream.read(stream.inAvailable()));
  document.documentElement=document.getDocumentElement();
  svgl.selectDocument(document);

  setTimeout=function(toeval,ms) {
    // todo: should it be possible to set multiple timeouts?
    timeOut={func:function(){eval(toeval);},timeOut:ms*1000,remain:ms*1000};
  };

  document._handleScripts(this);
  svgl.startAnimation(document);
  // animation finished
  
  step(function(dt){redisplay();return getEvents(dt);});
 })();
