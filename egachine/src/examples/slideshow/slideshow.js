// this is quite a hack
ejs.ModuleLoader.load("gl");
ejs.ModuleLoader.load("svgl");
ejs.ModuleLoader.load("EGachine");
ejs.ModuleLoader.load("Input");
ejs.ModuleLoader.load("Audio");
ejs.ModuleLoader.load("Video");
ejs.ModuleLoader.load("File");
ejs.ModuleLoader.load("sg");
sg.Video=Video;
var stderr=ejs.ModuleLoader.get("Stream").stderr;
var oldinput;

// todo !!
var examplePATH=["/home/jens/develop/Diplomarbeit/egachine/src/examples"];

Audio._playMusic=Audio.playMusic;
Audio.playMusic=function(resname) {
  var res=EGachine.getResource(resname);
  var dec=res.decode();
  Audio._playMusic(dec);
};

Audio.samples={};

Audio._loadSample=Audio.loadSample;
Audio._playSample=Audio.playSample;

Audio.loadSample=function(resname) {
  var res=EGachine.getResource(resname);
  var dec=res.decode();
  var sid=Audio._loadSample(dec);
  Audio.samples[resname]=sid;
  return sid;
};

Audio.playSample=function(resname,repeat) {
  var sid=Audio.samples[resname];
  if (sid==undefined)
    sid=Audio.loadSample(resname);
  if (repeat==undefined)
    repeat=0;
  return Audio._playSample(sid,repeat);
};

// fake egachine (client)
EGachine.client=true;
EGachine.version=new EGachine.Version("0.1.1");
EGachine.step=function(f,stepSize) {
  // todo: for now we always ignore stepSize
  EGachine._step=f;
};

function findExample(fname) {
  return ejs.ModuleLoader.findFile(examplePATH,fname);
};

function ifstream(fname) {
  var stream,fullname;
  if ((fullname=findExample(fname)) 
      && (stream=File.read(fullname)) 
      && (stream.inAvailable()>0))
    return stream;
  throw Error("Could not open file: "+fname);
};

function loadFile(fname) {
  var stream=ifstream(fname);
  return stream.read(stream.inAvailable());
};

function readline(istream){
  var res="";
  var c;
  do {
    c=istream.read(1);
    res+=c;
  }while(c && (c!="\n"));
  return res;
};

function loadSVG(fname) {
  document=new svgl.SVGDocument(loadFile(fname));
  document.documentElement=document.getDocumentElement();
  svgl.selectDocument(document);
  document._handleScripts(this);
};

function handleNavigation(i) {
  function setArrowColor(id,color) {
    document.getElementById(id).setAttribute("style","fill:"+color+";stroke:black;stroke-width:6;stroke-opacity:0.06");
  };

  var ret=0;
  var id;
  if (i.dev==2) {
    setArrowColor("nextPage","#ffffb6");
    setArrowColor("prevPage","#ffffb6");
    if (i.x) {
      // key pressed
      if (i.x>0)
	setArrowColor("nextPage","#b1ed00");
      else
	setArrowColor("prevPage","#b1ed00");
    }else if (oldinput.x) {
      // key released
      ret=oldinput.x;
    }
    oldinput=i;
  }
  return ret;
};

function displayTextPage(page) {
  var svg=document.getElementById("svg");
  var group;
  var hstyle="font-family:Verdana;font-size:48;fill:black;";
  var style="font-family:Verdana;font-size:32;fill:black;";
  var i;
  var np=0;

  //  stderr.write("display text page:"+page.toSource()+"\n");

  restoreGL();
  gl.ClearColor(1, 1, 1, 1);

  function addText(x,y,style,text) {
    var ret;
    ret=document.createElement("text");
    ret.setAttribute("style",style);
    ret.appendChild(document.createTextNode(text));
    ret.setAttribute("transform","translate("+x+" "+y+")");
    group.appendChild(ret);
    return ret;
  };

  group=document.createElement("g");
  svg.appendChild(group);
  for (i=0;i<page.lines.length;++i)
    addText(100 ,i*50+90+(i?40:0), i ? style : hstyle,page.lines[i].replace(/\n$/,''));

  Input.devListeners=[];
  Input.addDevListener(function(i){np=handleNavigation(i);});
  do{
    gl.Clear(GL_COLOR_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
    svgl.display(document);
    Video.swapBuffers();
    if (!Input.waitEvent())
      stderr.write("Warning: error while waiting for event");
    Input.poll();
  }while(np==0);
  svg.removeChild(group);
  return np;
};

function restoreGL()
{
  var viewport=Video.getViewport();
  // try to restore gl
  gl.MatrixMode(GL_PROJECTION);
  gl.LoadIdentity();
  gl.Ortho(0,viewport[2],0,viewport[3],-100,100);
  gl.MatrixMode(GL_MODELVIEW);
  gl.LoadIdentity();
  gl.Disable(GL_CULL_FACE);
  gl.Disable(GL_LIGHTING);
  gl.Disable(GL_LIGHT0);
  gl.Disable(GL_DEPTH_TEST);
  //  gl.Enable(GL_LINE_SMOOTH);
  //  gl.Enable(GL_POLYGON_SMOOTH);
}

function displayScript(fname) {
  var np=0;
  var start,last,now;
  var Timer=ejs.ModuleLoader.get("Timer");

  Input.devListeners=[];
  Input.addDevListener(function(i){np=handleNavigation(i);});
  delete EGachine._step;
  delete EGachine.sceneGraph;
  restoreGL();
  gl.ClearColor(0, 0, 0, 1);
  gl.Color4f(1,1,1,1);
  gl.Enable(GL_BLEND);
  ejs.ModuleLoader.loadScript.call(ejs.getGlobal(), findExample(fname));
  last=start=Timer.getTimeStamp();
  do{
    Input.poll();
    if (typeof display != "undefined") {
      display();
    }else if (EGachine._step) {
      // application without its own main loop
      // => we provide the main loop

      // this is taken from egachine.in (keep it in sync)
      // or better: todo: provide stuff in egachine.in as module
      now=Timer.getTimeStamp();
      dt=(now-last)/1000000.0;
      last=now;
      
      // get input events
      Input.poll();
      
      // clear screen
      if (EGachine.sceneGraph)
	Video.clear();
      
      EGachine._step(dt);
      
      // paint scenegraph
      if (EGachine.sceneGraph) {
	EGachine.sceneGraph.paint(dt);
	Video.swapBuffers();
      };
    }else throw new Error("no display function and no function registered via EGachine.step");
  }while(np==0);
  Audio.stopMusic();
  return np;
}

function parsePages(fname) {
  var ret=[];
  var stream=ifstream(fname);
  var cpage;

  function addLine(line) {
    var fname;
    if (!cpage) {
      // start new page
      if (line[0]=="@") {
	fname=line.slice(1).replace(/\n$/,'');
	cpage={script:line.slice(1),display:function(){return displayScript(fname);}};
      }else{
	cpage={lines:[],display:function(){return displayTextPage(this);}};
      }
    }
    if (cpage.lines instanceof Array) cpage.lines.push(line);
  };
  
  function endOfPage() {
    if (!cpage) return;
    ret.push(cpage);
    cpage=undefined;
  }
  
  while ((line=readline(stream))) {
    if (line=="--\n")
      endOfPage();
    else
      addLine(line);
  };
  endOfPage();
  return ret;
};

function main() {
  var pages;
  var cpage=0;
  var jump;
  loadSVG("slideshow/back.svg");
  pages=parsePages("slideshow/pages.txt");
  stderr.write(pages.toSource());
  while (true) {
    jump=pages[cpage].display();
    if (!jump) break;
    cpage+=jump;
    while (cpage<0) cpage+=pages.length;
    while (cpage>=pages.length) cpage-=pages.length;
  };
};

main();
