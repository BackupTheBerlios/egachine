(function(Input) {
  // module configuration options
  var exit=(ejs.config.Input && ejs.config.Input.exit) || ejs.exit;
  var toggleFullscreen=(ejs.config.Input && ejs.config.Input.toggleFullscreen);

  // load native library
  if (this.Input) return;
  var fname=ejs.ModuleLoader.findFile(ejs.config.modules.libraryPath,"ejsinput.la");
  if (!fname) throw new Error("Could not find module: 'ejsinput.la'");
  ejs.ModuleLoader.loadNative.call(Input,"ejsinput",fname.substring(0,fname.lastIndexOf(".")));

  Input.NOEVENT=0;
  Input.ACTIVEEVENT=1;
  Input.KEYDOWN=2;
  Input.KEYUP=3;
  Input.MOUSEMOTION=4;
  Input.MOUSEBUTTONDOWN=5;
  Input.MOUSEBUTTONUP=6;
  Input.JOYAXISMOTION=7;
  Input.JOYBALLMOTION=8;
  Input.JOYHATMOTION=9;
  Input.JOYBUTTONDOWN=10;
  Input.JOYBUTTONUP=11;
  Input.QUIT=12;
  Input.SYSWMEVENT=13;
  Input.EVENT_RESERVEDA=14;
  Input.EVENT_RESERVEDB=15;
  Input.VIDEORESIZE=16;
  Input.VIDEOEXPOSE=17;
  Input.ACTIVEEVENTMASK=2;
  Input.KEYDOWNMASK=4;
  Input.KEYUPMASK=8;
  Input.MOUSEMOTIONMASK=16;
  Input.MOUSEBUTTONDOWNMASK=32;
  Input.MOUSEBUTTONUPMASK=64;
  Input.MOUSEEVENTMASK=112;
  Input.JOYAXISMOTIONMASK=128;
  Input.JOYBALLMOTIONMASK=256;
  Input.JOYHATMOTIONMASK=512;
  Input.JOYBUTTONDOWNMASK=1024;
  Input.JOYBUTTONUPMASK=2048;
  Input.JOYEVENTMASK=3968;
  Input.VIDEORESIZEMASK=65536;
  Input.VIDEOEXPOSEMASK=131072;
  Input.QUITMASK=4096;
  Input.SYSWMEVENTMASK=8192;
  Input.ALLEVENTS=4294967295;
  Input.ADDEVENT=0;
  Input.PEEKEVENT=1;
  Input.GETEVENT=2;
  Input.QUERY=-1;
  Input.IGNORE=0;
  Input.DISABLE=0;
  Input.ENABLE=1;
  Input.KEY_UNKNOWN=0;
  Input.KEY_FIRST=0;
  Input.KEY_BACKSPACE=8;
  Input.KEY_TAB=9;
  Input.KEY_CLEAR=12;
  Input.KEY_RETURN=13;
  Input.KEY_PAUSE=19;
  Input.KEY_ESCAPE=27;
  Input.KEY_SPACE=32;
  Input.KEY_EXCLAIM=33;
  Input.KEY_QUOTEDBL=34;
  Input.KEY_HASH=35;
  Input.KEY_DOLLAR=36;
  Input.KEY_AMPERSAND=38;
  Input.KEY_QUOTE=39;
  Input.KEY_LEFTPAREN=40;
  Input.KEY_RIGHTPAREN=41;
  Input.KEY_ASTERISK=42;
  Input.KEY_PLUS=43;
  Input.KEY_COMMA=44;
  Input.KEY_MINUS=45;
  Input.KEY_PERIOD=46;
  Input.KEY_SLASH=47;
  Input.KEY_0=48;
  Input.KEY_1=49;
  Input.KEY_2=50;
  Input.KEY_3=51;
  Input.KEY_4=52;
  Input.KEY_5=53;
  Input.KEY_6=54;
  Input.KEY_7=55;
  Input.KEY_8=56;
  Input.KEY_9=57;
  Input.KEY_COLON=58;
  Input.KEY_SEMICOLON=59;
  Input.KEY_LESS=60;
  Input.KEY_EQUALS=61;
  Input.KEY_GREATER=62;
  Input.KEY_QUESTION=63;
  Input.KEY_AT=64;
  Input.KEY_LEFTBRACKET=91;
  Input.KEY_BACKSLASH=92;
  Input.KEY_RIGHTBRACKET=93;
  Input.KEY_CARET=94;
  Input.KEY_UNDERSCORE=95;
  Input.KEY_BACKQUOTE=96;
  Input.KEY_a=97;
  Input.KEY_b=98;
  Input.KEY_c=99;
  Input.KEY_d=100;
  Input.KEY_e=101;
  Input.KEY_f=102;
  Input.KEY_g=103;
  Input.KEY_h=104;
  Input.KEY_i=105;
  Input.KEY_j=106;
  Input.KEY_k=107;
  Input.KEY_l=108;
  Input.KEY_m=109;
  Input.KEY_n=110;
  Input.KEY_o=111;
  Input.KEY_p=112;
  Input.KEY_q=113;
  Input.KEY_r=114;
  Input.KEY_s=115;
  Input.KEY_t=116;
  Input.KEY_u=117;
  Input.KEY_v=118;
  Input.KEY_w=119;
  Input.KEY_x=120;
  Input.KEY_y=121;
  Input.KEY_z=122;
  Input.KEY_DELETE=127;
  Input.KEY_WORLD_0=160;
  Input.KEY_WORLD_1=161;
  Input.KEY_WORLD_2=162;
  Input.KEY_WORLD_3=163;
  Input.KEY_WORLD_4=164;
  Input.KEY_WORLD_5=165;
  Input.KEY_WORLD_6=166;
  Input.KEY_WORLD_7=167;
  Input.KEY_WORLD_8=168;
  Input.KEY_WORLD_9=169;
  Input.KEY_WORLD_10=170;
  Input.KEY_WORLD_11=171;
  Input.KEY_WORLD_12=172;
  Input.KEY_WORLD_13=173;
  Input.KEY_WORLD_14=174;
  Input.KEY_WORLD_15=175;
  Input.KEY_WORLD_16=176;
  Input.KEY_WORLD_17=177;
  Input.KEY_WORLD_18=178;
  Input.KEY_WORLD_19=179;
  Input.KEY_WORLD_20=180;
  Input.KEY_WORLD_21=181;
  Input.KEY_WORLD_22=182;
  Input.KEY_WORLD_23=183;
  Input.KEY_WORLD_24=184;
  Input.KEY_WORLD_25=185;
  Input.KEY_WORLD_26=186;
  Input.KEY_WORLD_27=187;
  Input.KEY_WORLD_28=188;
  Input.KEY_WORLD_29=189;
  Input.KEY_WORLD_30=190;
  Input.KEY_WORLD_31=191;
  Input.KEY_WORLD_32=192;
  Input.KEY_WORLD_33=193;
  Input.KEY_WORLD_34=194;
  Input.KEY_WORLD_35=195;
  Input.KEY_WORLD_36=196;
  Input.KEY_WORLD_37=197;
  Input.KEY_WORLD_38=198;
  Input.KEY_WORLD_39=199;
  Input.KEY_WORLD_40=200;
  Input.KEY_WORLD_41=201;
  Input.KEY_WORLD_42=202;
  Input.KEY_WORLD_43=203;
  Input.KEY_WORLD_44=204;
  Input.KEY_WORLD_45=205;
  Input.KEY_WORLD_46=206;
  Input.KEY_WORLD_47=207;
  Input.KEY_WORLD_48=208;
  Input.KEY_WORLD_49=209;
  Input.KEY_WORLD_50=210;
  Input.KEY_WORLD_51=211;
  Input.KEY_WORLD_52=212;
  Input.KEY_WORLD_53=213;
  Input.KEY_WORLD_54=214;
  Input.KEY_WORLD_55=215;
  Input.KEY_WORLD_56=216;
  Input.KEY_WORLD_57=217;
  Input.KEY_WORLD_58=218;
  Input.KEY_WORLD_59=219;
  Input.KEY_WORLD_60=220;
  Input.KEY_WORLD_61=221;
  Input.KEY_WORLD_62=222;
  Input.KEY_WORLD_63=223;
  Input.KEY_WORLD_64=224;
  Input.KEY_WORLD_65=225;
  Input.KEY_WORLD_66=226;
  Input.KEY_WORLD_67=227;
  Input.KEY_WORLD_68=228;
  Input.KEY_WORLD_69=229;
  Input.KEY_WORLD_70=230;
  Input.KEY_WORLD_71=231;
  Input.KEY_WORLD_72=232;
  Input.KEY_WORLD_73=233;
  Input.KEY_WORLD_74=234;
  Input.KEY_WORLD_75=235;
  Input.KEY_WORLD_76=236;
  Input.KEY_WORLD_77=237;
  Input.KEY_WORLD_78=238;
  Input.KEY_WORLD_79=239;
  Input.KEY_WORLD_80=240;
  Input.KEY_WORLD_81=241;
  Input.KEY_WORLD_82=242;
  Input.KEY_WORLD_83=243;
  Input.KEY_WORLD_84=244;
  Input.KEY_WORLD_85=245;
  Input.KEY_WORLD_86=246;
  Input.KEY_WORLD_87=247;
  Input.KEY_WORLD_88=248;
  Input.KEY_WORLD_89=249;
  Input.KEY_WORLD_90=250;
  Input.KEY_WORLD_91=251;
  Input.KEY_WORLD_92=252;
  Input.KEY_WORLD_93=253;
  Input.KEY_WORLD_94=254;
  Input.KEY_WORLD_95=255;
  Input.KEY_KP0=256;
  Input.KEY_KP1=257;
  Input.KEY_KP2=258;
  Input.KEY_KP3=259;
  Input.KEY_KP4=260;
  Input.KEY_KP5=261;
  Input.KEY_KP6=262;
  Input.KEY_KP7=263;
  Input.KEY_KP8=264;
  Input.KEY_KP9=265;
  Input.KEY_KP_PERIOD=266;
  Input.KEY_KP_DIVIDE=267;
  Input.KEY_KP_MULTIPLY=268;
  Input.KEY_KP_MINUS=269;
  Input.KEY_KP_PLUS=270;
  Input.KEY_KP_ENTER=271;
  Input.KEY_KP_EQUALS=272;
  Input.KEY_UP=273;
  Input.KEY_DOWN=274;
  Input.KEY_RIGHT=275;
  Input.KEY_LEFT=276;
  Input.KEY_INSERT=277;
  Input.KEY_HOME=278;
  Input.KEY_END=279;
  Input.KEY_PAGEUP=280;
  Input.KEY_PAGEDOWN=281;
  Input.KEY_F1=282;
  Input.KEY_F2=283;
  Input.KEY_F3=284;
  Input.KEY_F4=285;
  Input.KEY_F5=286;
  Input.KEY_F6=287;
  Input.KEY_F7=288;
  Input.KEY_F8=289;
  Input.KEY_F9=290;
  Input.KEY_F10=291;
  Input.KEY_F11=292;
  Input.KEY_F12=293;
  Input.KEY_F13=294;
  Input.KEY_F14=295;
  Input.KEY_F15=296;
  Input.KEY_NUMLOCK=300;
  Input.KEY_CAPSLOCK=301;
  Input.KEY_SCROLLOCK=302;
  Input.KEY_RSHIFT=303;
  Input.KEY_LSHIFT=304;
  Input.KEY_RCTRL=305;
  Input.KEY_LCTRL=306;
  Input.KEY_RALT=307;
  Input.KEY_LALT=308;
  Input.KEY_RMETA=309;
  Input.KEY_LMETA=310;
  Input.KEY_LSUPER=311;
  Input.KEY_RSUPER=312;
  Input.KEY_MODE=313;
  Input.KEY_COMPOSE=314;
  Input.KEY_HELP=315;
  Input.KEY_PRINT=316;
  Input.KEY_SYSREQ=317;
  Input.KEY_BREAK=318;
  Input.KEY_MENU=319;
  Input.KEY_POWER=320;
  Input.KEY_EURO=321;
  Input.KEY_UNDO=322;
  Input.KMOD_NONE=0;
  Input.KMOD_LSHIFT=1;
  Input.KMOD_RSHIFT=2;
  Input.KMOD_LCTRL=64;
  Input.KMOD_RCTRL=128;
  Input.KMOD_LALT=256;
  Input.KMOD_RALT=512;
  Input.KMOD_LMETA=1024;
  Input.KMOD_RMETA=2048;
  Input.KMOD_NUM=4096;
  Input.KMOD_CAPS=8192;
  Input.KMOD_MODE=16384;
  Input.KMOD_CTRL=192;
  Input.KMOD_SHIFT=3;
  Input.KMOD_ALT=768;
  Input.KMOD_META=3072;

  //! mouse buttons
  Input.BUTTON_LEFT=1;
  Input.BUTTON_MIDDLE=2;
  Input.BUTTON_RIGHT=3;
  Input.BUTTON_WHEELUP=4;
  Input.BUTTON_WHEELDOWN=5;

  //! state
  Input.PRESSED = 1;
  Input.RELEASED = 0;

  //! register function to be called if gamepad state changes
  Input.addDevListener=function(f) {
    if (typeof Input.devListeners == "undefined") Input.devListeners=[];
    if (!Input.handleInput) Input.handleInput=function(devState) {
      var i;
      for (i=0;i<Input.devListeners.length;++i)
	Input.devListeners[i](devState);
    };
    devListeners.push(f);
    return devListeners.length-1;
  };


  //! dom level  2 events
  Input.Event=function() {
    /*
      this.type=
      this.timeStamp=
      ...
    */
  }
  Input.Event.CAPTURING_PHASE = 1;
  Input.Event.AT_TARGET = 2;
  Input.Event.BUBBLING_PHASE = 3;

  Input.Event.prototype.preventDefault=function(){
    this._preventDefault=true;
  };

  Input.Event.prototype.stopPropagation=function(){
    this._stopPropagation=true;
  };

  Input.Event.prototype.clone=function(){
    // only works for now!
    var ret=new Input.Event();
    var i;
    for (i in this)
      if (this.hasOwnProperty(i))
	ret[i]=this[i];
    return ret;
  }

  Input.MouseEvent=function() {
  };

  Input.MouseEvent.prototype=new Input.Event();
  /* todo
  Input.MouseEvent.prototype.initMouseEvent=function(typeArg,
						     canBubbleArg,
						     cancelableArg,
						     viewArg,
						     detailArg,
						     screenXArg,
						     screenYArg,
						     clientXArg,
						     clientYArg,
						     ctrlKeyArg,
						     altKeyArg,
						     shiftKeyArg,
						     metaKeyArg,
						     buttonArg,
						     relatedTargetArg) {
    throw Error("not yet implemented");
    };*/

  //! convert event to dom level 2 MouseEvent
  Input.toMouseEvent=function(e) {
    var ret=new Input.MouseEvent();
    switch(e.type) {
    case Input.MOUSEMOTION:
      ret.type="mousemove";
      break;
    case Input.MOUSEBUTTONDOWN:
      ret.type="mousedown";
      break;
    case Input.MOUSEBUTTONUP:
      ret.type="mouseup";
      break;
    default:
      throw Error("wrong type: "+e.type);
    };

    switch(e.type) {
    case Input.MOUSEBUTTONDOWN:
    case Input.MOUSEBUTTONUP:
      ret.button=e.button-1;
      // no break here
    case Input.MOUSEMOTION:
      ret.screenX=e.x;
      ret.screenY=e.y;
      // todo: ?
      ret.clientX=e.x;
      ret.clientY=e.y;
      /* todo:
	 ret.ctrlKey=;
	 ret.shiftKey=;
	 ret.altKey=;
	 ret.metaKey=;
	 ret.metaKey=;
	 ret.type=;
      */
      break;
    default:
      throw Error("wrong event type");
    };
    ret.bubbles=true;
    ret.cancelable=true;
    ret.timeStamp=Date.now();
    return ret;
  };

  Input.EventTarget=function() {
    this.listeners={};
  };

  Input.EventTarget.prototype.addEventListener=function(type, listener, useCapture) {
    var a=this.listeners[type];
    if (!a) this.listeners[type]=a=[];
    a.push({listener:listener,useCapture:useCapture});
  };

  Input.EventTarget.prototype.dispatchEvent=function(evt) {
    var stderr=ejs.ModuleLoader.get("Stream").stderr;
    //    stderr.write("i.e.p.dispatch: "+this.toSource()+":"+evt.toSource()+"\n");

    var i;
    var l=this.listeners[evt.type];
    if (l) {
      for (i=0;i<l.length;++i) {
	var bubble=(evt.eventPhase==Input.Event.BUBBLING_PHASE)&&(!l[i].useCapture);
	var capture=(evt.eventPhase==Input.Event.CAPTURING_PHASE)&&(l[i].useCapture);
	if (bubble||capture||evt.eventPhase==Input.Event.AT_TARGET) {
	  try{
	    //	    stderr.write("call listener: "+l[i].listener+"\n");
	    l[i].listener(evt);
	  }catch(e){
	    // Any exceptions thrown inside an EventListener will not stop 
	    // propagation of the event.
	    stderr.write("Warning: "+e+"\n"+e.stack+"\n");
	  }
	}
      }
    }
    /*
      the return value of dispatchEvent indicates whether any of the listeners 
      which handled the event called preventDefault. 
      If preventDefault was called the value is false, else the value is true.
    */
    return evt._preventDefault;
  };

  Input.EventTarget.prototype.clear=function(){
    this.listeners={};
  };

  /* todo:
  Input.EventTarget.prototype.removeEventListener=function(type, listener, useCapture) {
  };*/

  var target=new Input.EventTarget();
  Input.addEventListener=function(type, listener, useCapture) {
    target.addEventListener(type, listener, useCapture);
  };
  Input.dispatchEvent=function(evt) {
    evt.eventPhase=Input.Event.AT_TARGET;
    target.dispatchEvent(evt);
  };
  Input.clearEventListeners=function(){
    target.clear();
  };

  Input.handleMouse=function(e) {
    Input.dispatchEvent(Input.toMouseEvent(e));
  };

  //! todo: this must always work => untrusted code should not be allowed to mess with this

  // backward compatibility layer
  Input.charMode=function(enable){
    Input.enableUnicode(enable? 1 : 0);
  }

  Input.keys=[
	      [Input.KEY_KP8, Input.KEY_KP2,  Input.KEY_KP4,  Input.KEY_KP6,   Input.KEY_KP0,  Input.KEY_KP_PERIOD],
	      [Input.KEY_w,   Input.KEY_s,    Input.KEY_a,    Input.KEY_d,     Input.KEY_1,    Input.KEY_2],
	      [Input.KEY_UP,  Input.KEY_DOWN, Input.KEY_LEFT, Input.KEY_RIGHT, Input.KEY_RALT, Input.KEY_RMETA],
	      ];
  // all input devices
  Input.devState=[];
  // map joystick number to input device number
  Input.joyDevMap=[];
  // opened joysticks
  Input.joySticks=[];
  // map keyboard number to input device number
  Input.keyDevMap=[];

  // devstate class
  // this holds the state of joypad like input device
  Input.DevState=function DevState(dev,x,y,buttons){
    if (typeof dev == "undefined") throw Error("dev required");
    this.dev=dev;
    this.x=x || 0;
    this.y=y || 0;
    this.buttons=buttons || 0;
  };

  Input.DevState.prototype.clone=function() {
    return new Input.DevState(this.dev,this.x,this.y,this.buttons);
  };

  function assert(f) {
    if (!f()) throw Error("assertion "+f.toSource()+" failed\n");
  };

  function debug(msg) {
    ejs.ModuleLoader.get("Stream").stderr.write(msg+"\n");
  };

  function info(msg) {
    ejs.ModuleLoader.get("Stream").stderr.write("INFO: "+msg+"\n");
  };

  (function(){
    // check for usable joysticks
    var numj,i,joy,dnum;
    numj=Input.numJoysticks();
    for (i=0;i<numj;++i) {
      //      try{
	joy=new Input.Joystick(i);
	if (joy.numAxes()>=2) {
	  dnum=Input.joyDevMap[i]=Input.devState.length;
	  Input.devState.push(new Input.DevState(dnum));
	  // otherwise it is close again
	  Input.joySticks.push(joy);
	}
	//      }catch(e){
	//      }
    }
    info("found "+numj+" joystick(s). "+Input.devState.length+" useable as game pad (>=2 axes)");

    // add keyboard devices
    for (i=0;i<Input.keys.length;++i) {
      dnum=Input.keyDevMap[i]=Input.devState.length;
      Input.devState.push(new Input.DevState(dnum));
    };
  })();

  Input.poll=function(){
    var events,i,event,maxe=0,mouseMotion=false;

    function handleDevKey(e, keyDev)
    {
      var dev,m_state,old,pressed,k;
      // map keyboard dev no to real dev no
      assert(function(){return ((keyDev>=0)&&(keyDev<Input.keys.length));});
      dev=Input.keyDevMap[keyDev];
      assert(function(){return typeof Input.devState != "undefined";});
      assert(function(){return ((dev>=0)&&(dev<Input.devState.length));});
  
      m_state=Input.devState[dev];
      old=m_state.clone();
      pressed=e.state;

      k=e.sym;
      if (k==Input.keys[keyDev][0]) {
	// up
	if (pressed) m_state.y=1;
	else if (m_state.y==1) m_state.y=0;
      }else if (k==Input.keys[keyDev][1]) {
	// down
	if (pressed) m_state.y=-1;
	else if (m_state.y==-1) m_state.y=0;
      }else if (k==Input.keys[keyDev][2]) {
	// left 
	if (pressed) m_state.x=-1;
	else if (m_state.x==-1) m_state.x=0;
      }else if (k==Input.keys[keyDev][3]) {
	// right
	if (pressed) m_state.x=1;
	else if (m_state.x==1) m_state.x=0;
      }else if (k==Input.keys[keyDev][4]) {
	// button 1
	if (pressed) m_state.buttons|=1;
	else m_state.buttons&=~1;
      }else if (k==Input.keys[keyDev][5]) {
	// button 1
	if (pressed) m_state.buttons|=2;
	else m_state.buttons&=~2;
      }
      if ((m_state.x!=old.x)||(m_state.y!=old.y)||(m_state.buttons!=old.buttons)) {
	Input.handleInput(m_state.clone());
	return true;
      }
      return false;
    }

    function handleSpecialKey(e)
    {
      if (!e.state) return false;
      switch (e.sym){
      case Input.KEY_ESCAPE:
	exit(true);
      case Input.KEY_RETURN:
	if (e.mod & Input.KMOD_ALT){
	  toggleFullscreen();
	  return true;
	}
	break;
      default:
	return false;
      }
      return false;
    }

    //! handle key in character mode
    function handleKey(e)
    {
      if (!e.state) return false;
      Input.handleChar(String.fromCharCode(e.unicode));
      return true;
    }

    function joyScale(v) 
    {
      var clearance=20000;
      if (v>clearance) return 1;
      if (v<-clearance) return -1;
      return 0;
    }

    function handleJoyMotion(event)
    {
      var joy=event.which;
      if ((joy<0)||(joy>=joyDevMap.length)) return;
  
      var dev=joyDevMap[joy];
      if ((dev<0)||(dev>=devState.length)) return;
  
      var m_state=devState[dev];
      var old=m_state.clone();

      switch (event.axis) {
      case 0: 
	m_state.x=joyScale(event.value);
	break;
      case 1:
	m_state.y=joyScale(-event.value);
	break;
      }

      if ((m_state.x!=old.x)||(m_state.y!=old.y))
	Input.handleInput(m_state.clone());
    };

    function handleJoyButton(e)
    {
      var joy=event.which;
      if ((joy<0)||(joy>=joyDevMap.length)) return;
  
      var dev=joyDevMap[joy];
      if ((dev<0)||(dev>=devState.length)) return;

      var m_state=devState[dev];
      var old=m_state.clone();

      if (e.state==Input.PRESSED)
	m_state.buttons|=1<<e.button;
      else
	m_state.buttons&=~(1<<e.button);

      if (m_state.buttons!=old.buttons)
	Input.handleInput(m_state.clone());
    };

    events=Input.getEvents();
    if (events.length>maxe) {
      maxe=events.length;
      //      debug("maxe: "+maxe);
    }
    for (i=0;i<events.length;++i) {
      event=events[i];
      switch (event.type) {
      case Input.QUIT:
	exit(true);
      case Input.KEYDOWN:
      case Input.KEYUP:
	if (!Input.enableUnicode(-1)) {
	  if (!handleDevKey(event,0))
	    if (!handleDevKey(event,1))
	      if (!handleDevKey(event,2))
		handleSpecialKey(event);
	}else{
	  if (!handleSpecialKey(event)) {
	    handleKey(event);
	  }
	}
	break;
      case Input.JOYAXISMOTION:
	handleJoyMotion(event);
	break;
      case Input.JOYBUTTONDOWN:
      case Input.JOYBUTTONUP:
	handleJoyButton(event);
	break;
      case Input.VIDEORESIZE:
	Input.handleResize(event);
	break;
      case Input.MOUSEMOTION:
	// accumulate mouse motion events
	mouseMotion=event;
	break;
      case Input.MOUSEBUTTONDOWN:
      case Input.MOUSEBUTTONUP:
	if (mouseMotion) {
	  // keep mouse events in order
	  Input.handleMouse(mouseMotion);
	  mouseMotion=false;
	}
	Input.handleMouse(event);
	break;
      };
    };
    
    // report pending mouse motion events
    if (mouseMotion) {
      Input.handleMouse(mouseMotion);
      mouseMotion=false;
    };
  };

 })(this);
