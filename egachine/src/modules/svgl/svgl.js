(function(svgl){
  var Video=ejs.ModuleLoader.get("Video");
  var Input=ejs.ModuleLoader.get("Input");

  // load native library
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejssvgl.la");
  if (!fname) throw new Error("Could not find module: 'ejssvgl.la'");
  ejs.ModuleLoader.loadNative.call(svgl,"ejssvgl",fname.substring(0,fname.lastIndexOf(".")));

  function assert(f){
    if ((typeof f == "function")&&(f())) return;
    if (f) return;
    throw Error("Assertion: "+f.toSource()+" failed\n");
  };
  
  function debug(x){
    var stderr=ejs.ModuleLoader.get("Stream").stderr;
    stderr.write(x+"\n");
  }

  svgl.Node.prototype.addEventListener=function(type, listener, useCapture) {
    if (!this._target) this._target=new Input.EventTarget();
    this._target.addEventListener(type,listener,useCapture);
  };

  svgl.Node.prototype.dispatchEvent=function(evt) {
    if (!this._target) return;
    return this._target.dispatchEvent(evt);
  };

  svgl.Node.prototype.ELEMENT_NODE = 1;
  svgl.Node.prototype.ATTRIBUTE_NODE = 2;
  svgl.Node.prototype.TEXT_NODE = 3;
  svgl.Node.prototype.CDATA_SECTION_NODE = 4;
  svgl.Node.prototype.ENTITY_REFERENCE_NODE = 5;
  svgl.Node.prototype.ENTITY_NODE = 6;
  svgl.Node.prototype.PROCESSING_INSTRUCTION_NODE = 7;
  svgl.Node.prototype.COMMENT_NODE = 8;
  svgl.Node.prototype.DOCUMENT_NODE = 9;
  svgl.Node.prototype.DOCUMENT_TYPE_NODE = 10;
  svgl.Node.prototype.DOCUMENT_FRAGMENT_NODE = 11;
  svgl.Node.prototype.NOTATION_NODE = 12;

  //! load SVG file
  /*!
    \return SVG document object
   */
  svgl.load=function(fileName) {
    var document;
    var File=ejs.ModuleLoader.get("File");
    var stream=File.read(fileName);
    if (stream.inAvailable()<0)
      throw Error("Could not open file: "+fileName);
    
    document=new svgl.SVGDocument(stream.readAll());
    document.documentElement=document.getDocumentElement();
    return document;
  }

  //! simple SVG viewer
  svgl.viewFile=function(fileName) {
    var gl=ejs.ModuleLoader.get("gl");
    var Timer=ejs.ModuleLoader.get("Timer");
    var svgl=ejs.ModuleLoader.get("svgl");
    var stderr=ejs.ModuleLoader.get("Stream").stderr;
    var timeout,oldTarget,oldTargetPath;

    Video.showMouseCursor(1);

    function setNewTarget(e,newTarget) {
      //      assert(function(){return e && newTarget;});

      var subevt;
      if (newTarget!=oldTarget) {
	// emit mouseover and mouseout
	
	// mouseout
	if (oldTarget) {
	  subevt=e.clone();
	  subevt.type="mouseout";
	  subevt.target=oldTarget;
	  subevt._path=oldTargetPath;
	  subevt.relatedTarget=newTarget;
	  //debug("mouseout: target: "+subevt.target.nodeName+" relatedTarget: "+subevt.relatedTarget.nodeName);
	  Input.dispatchEvent(subevt);
	}

	// mouseover
	subevt=e.clone();
	subevt.type="mouseover";
	oldTarget=e.target=subevt.target=newTarget;
	oldTargetPath=e._path;
	//	stderr.write("setNewTarget (mouseover): "+subevt.toSource()+"\n");
	Input.dispatchEvent(subevt);
      }
      e.target=newTarget;
    };

    function setEventTarget(e) {
      //      assert(function(){return !e.target;});
      var picked,path=[document],p2,p3;
      picked=svgl.pick(e.screenX,e.screenY);
      //      stderr.write("setEventTarget: "+picked.toSource()+"\n");
      if ((!picked)||(!picked.length)) {
	// nothing picked
	e._path=path;
	setNewTarget(e,document);
	//	assert(function(){return e.target;});
	return e;
      }
      p2=picked[picked.length-1];
      if ((!p2)||(!p2.length)) throw Error("hmm: "+p2.toSource()+"\n");
      path=path.concat(p2);
      p3=p2[p2.length-1];
      e._path=path;
      //      assert(function(){return p3;});
      setNewTarget(e,p3);
      //      assert(function(){return e.target;});
      return e;
    };

    function dispatch(evt) {
      //      stderr.write("dispatch: "+evt.toSource());
      // capturing
      evt.eventPhase=Input.Event.CAPTURING_PHASE;
      var path=evt._path;
      delete evt._path;
      var i;
      for (i=0;i<path.length-1;++i) {
	evt.currentTarget=path[i];
	path[i].dispatchEvent(evt);
	if (evt._stopPropagation) break;
      };

      if (!evt._stopPropagation) {
	// at target
	i=path.length-1;
	evt.eventPhase=Input.Event.AT_TARGET;
	evt.currentTarget=path[i];
	path[i].dispatchEvent(evt);
      }

      if (!evt._stopPropagation) {
	// bubbling
	evt.eventPhase=Input.Event.BUBBLING_PHASE;
	var i;
	for (i=path.length-2;i>=0;--i) {
	  evt.currentTarget=path[i];
	  path[i].dispatchEvent(evt);
	  if (evt._stopPropagation) break;
	};
      };
      
      // restore path
      evt._path=path;
    };
    
    function mouse(evt) {
      //      stderr.write("mouse: "+evt.toSource()+"\n");

      if (!evt.target)
	setEventTarget(evt);
      //      assert(function(){return evt.target;});
      dispatch(evt);
      if (evt.type=="mouseup") {
	// todo: perhaps the evt was modified!! clone mouseup before dispatch
	// todo: click does not always happen on mouseup
	evt.type="click";
	dispatch(evt);
      }
    };
    
    function addMouseListener(type) {
      Input.addEventListener(type,function(evt){return mouse(evt);},false);
    };
    
    Input.clearEventListeners();

    addMouseListener("mousemove");
    addMouseListener("mousedown");
    addMouseListener("mouseup");
    addMouseListener("mouseover");
    addMouseListener("mouseout");
    
    if (!gl.GetIntegerv(GL_STENCIL_BITS)[0])
      stderr.write("Warning no stencil buffer => output will be corrupt (perhaps you have to switch your color depth)\n");
    
    function getEvents(dt) {
      var tmp;
      if (timeout) {
	// process timeout
	if ((timeout.remain-=dt)<0) {
	  tmp=timeout;
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
      
      var start,last,now,ret;
      start=last=now=Timer.getTimeStamp();
      while ((!time)||(now-start<time)) {
	Timer.uSleep(stepSize);
	now=Timer.getTimeStamp();
	if (!f(now-last)) break;
	last=now;
      };
      ret=now-start;
      //      debug("step: "+ret);
      return ret;
    };
    
    //! called from native code (animation running)
    reschedule=function(sec){
      //      debug("reschedule: "+sec);
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

    // todo: hmm global
    document=svgl.load(fileName);
    svgl.selectDocument(document);

    // todo: hmm global
    setTimeout=function(toeval,ms) {
      // todo: should it be possible to set multiple timeouts?
      if (typeof toeval == "function")
	timeout={func:function(){toeval();},timeout:ms*1000,remain:ms*1000};
      else
	timeout={func:function(){eval(toeval);},timeout:ms*1000,remain:ms*1000};
    };
    
    document._handleScripts(this);

    //! convert event attributes to addEventListener calls
    (function(){
      var types=["click","mouseover","mouseout","mousedown","mouseup","mousemove"];

      function convert(e) {
	var i,s,c;
	//	assert(e);
	if (e.getAttribute) {
	  for (i=0;i<types.length;++i) {
	    if ((s=e.getAttribute("on"+types[i]))) {
	      // todo: wrong scope?
	      // should those handlers really only be called if directly targeted?
	      // or normal bubbling?
	      // hmm
	      //	      debug("ADDED:"+types[i]+":"+s);
	      e.addEventListener(types[i],
				 (function(t,s){
				   return function(evt){
				     //				       debug("eval: "+s);
				       eval(s);
				   };})(e,s),
				 false);
	    }
	  }
	}
	c=e.childNodes;
	for (i=0;i<c.length;++i) {
	  convert(c.item(i));
	};
      };
      
      convert(document.documentElement);
    })();
    

    svgl.startAnimation(document);
    // animation finished
    
    step(function(dt){redisplay();return getEvents(dt);});
  };

 })(this);
