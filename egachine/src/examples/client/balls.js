// move some balls

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.1");

(function(){
  // array - with current state of input devices
  var joypad=[];
  var viewport=Video.getViewport();
  var sx=viewport[2];
  var sy=viewport[3];
  var spriteSize=new sg.V2D(sx/10,sx/10);

  addResources();

  // register gamepad event listener
  Input.addDevListener(function(i){
		   addBall();
		 });

  // clip value to [min,max]
  function clip(min,max,value)
  {
    if (value<min) return min;
    if (value>max) return max;
    return value;
  }

  // return random number out of [min,max]
  function rnd(min,max) {
    return Math.random()/(max-min)+min;
  }

  // ball object
  function Ball() {
    this.speed=new sg.V2D(-sx/5*rnd(0.5,1),-sx/5*rnd(0.5,1));
    this.degrees=new sg.Degrees(rnd(0,90));
    this.rotspeed=rnd(270,720);
    this.pos=new sg.V2D(sx*rnd(0,1),sy*rnd(0,1));
  }
  Ball.prototype.restart=function(){
    this.pos.x=sx/2;
    this.pos.y=sy/2;
  }
  Ball.prototype.step=function(dt){
    this.pos.x+=this.speed.x*dt;
    this.pos.y+=this.speed.y*dt;
    this.degrees.value+=this.rotspeed*dt;
    // collission detection with walls
    if (this.pos.y<0) {
      this.pos.y=0;
      this.speed.y*=-1;
    }else if (this.pos.y>sy){
      this.pos.y=sy;
      this.speed.y*=-1;
    }
    if (this.pos.x<0) {
      this.pos.x=0;
      this.speed.x*=-1;
    }else if (this.pos.x>sx) {
      this.pos.x=sx;
      this.speed.x*=-1;
    }
  }

  // build our scenegraph
  EGachine.sceneGraph=new sg.Node();

  var i;
  var ball=[];

  function addBall() {
    var i=ball.length;
    ball.push(new Ball());
    EGachine.sceneGraph.add(new sg.Color(rnd(0.4,1),rnd(0.4,1),rnd(0.4,1),rnd(0.4,1))
			    .add(new sg.Sprite("ball",spriteSize,ball[i].pos,ball[i].degrees)
				 .add(new sg.Sprite("ball",spriteSize.scale(0.5),spriteSize))
				 .add(new sg.Sprite("ball",spriteSize.scale(0.5),spriteSize.scale(-1)))
				 ));
  }

  addBall();

  // register function to be called for each step in time
  EGachine.step(function(dt){
		  // move balls
		  for (i=0;i<ball.length;++i)
		    ball[i].step(dt);
		});

  function addResources(){
EGachine.addResource(({name:"ball", size:6778,
data:"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"}));
  };
 })();