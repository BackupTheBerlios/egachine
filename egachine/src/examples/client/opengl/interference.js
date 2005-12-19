if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.2");

(function(){
  // array - with current state of input devices
  var viewport=Video.getViewport();
  var sx=viewport[2];
  var sy=viewport[3];

  addResources();

  var Pattern=function(resname,tilesX,tilesY){
    this.resname=resname;
    this.tc=[[0,0],
	     [tilesX,0],
	     [tilesX,tilesY],
	     [0,tilesY],
	     ];
    this.vc=[[-0.5,-0.5],
	     [+0.5,-0.5],
	     [+0.5,+0.5],
	     [-0.5,+0.5],
	     ];
  };
  Pattern.prototype.paint=function(time){
    var i;
    gl.Enable(GL_TEXTURE_2D);
    gl.BindTexture(GL_TEXTURE_2D,Video.getTextureID(this.resname));
    gl.Begin(GL_QUADS);
    for (i=0;i<4;++i) {
      gl.TexCoord2f(this.tc[i][0],this.tc[i][1]);
      gl.Vertex2f(this.vc[i][0],this.vc[i][1]);
    }
    gl.End();
    sg.Node.prototype.paint.call(this,time);
  };

  // grid
  var scroll=new sg.V2D(sx/2,sy/2);
  var grid=new sg.Scale(new sg.V2D(sx*1,sy*1)).add(new Pattern("pattern",40,30));
  var rot;
  EGachine.sceneGraph=new sg.Node()
    .add(new sg.Color(0,1,0.5).add(new sg.Translate(new sg.V2D(sx/2,sy/2)).add(grid)))
    .add(new sg.Color(1,0.3,0,1).add(new sg.Translate(new sg.V2D(sx/2+3,sy/2+3)).add(new sg.Rotate(new sg.Degrees(1)).add(grid))))
    .add(new sg.Color(0,0.3,1).add(new sg.Translate(scroll).add(new sg.Rotate((rot=new sg.Degrees(0))).add(grid))));

  var oldX;
  var oldY;
  Input.addEventListener("mousemove",function(evt){
			   if (oldX!=undefined) {
			     scroll.x+=(evt.screenX-oldX)/10;
			     scroll.y-=(evt.screenY-oldY)/10;
			   }
			   oldX=evt.screenX;
			   oldY=evt.screenY;
			 },false);

  var rotate=false;
  Input.addEventListener("mousedown",function(evt){
			   rotate=true;
			 },false);

  Input.addEventListener("mouseup",function(evt){
			   rotate=false;
			 },false);

  // clip value to [min,max]
  function clip(min,max,value)
  {
    if (value<min) return min;
    if (value>max) return max;
    return value;
  }

  // register function to be called for each step in time
  EGachine.step(function(dt){
		  // move grid
		  //		  scroll.x+=dt*200;
		  //		  while (scroll.x>sx) scroll.x-=sx;
		  //		  scroll.y+=dt*100;
		  //		  while (scroll.y>sy) scroll.y-=sy;
		  if (rotate) rot.value+=10*dt;
		});

  function addResources(){
EGachine.addResource(
({name:"pattern", size:234, data:"\
iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAQAAADZc7J/AAAAAmJLR0QA/4ePzL8AAAAJcEhZcwAACxM\
AAAsTAQCanBgAAAAHdElNRQfVCwwRLh8oymQLAAAAe0lEQVRIx+3SoQ3CUBRG4dOGBEfQlKBYoHPg2K\
Eb4NttcGzSCao7AqmoOJjSPHv9/d131UvewXIviPoALPSMNHyrj9ew1YFtnpyiroH1f+DIPeoaaPfDA\
8J28rI96OyoUeMN7Hw7OKtGDfgsfybs7CA7yA6yg+wgOyj2A9b8sl4+EP1GAAAAAElFTkSuQmCC\
"}));
  };
 })();
