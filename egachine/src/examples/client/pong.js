// simple pong clone

if (!EGachine.client) throw "This file must be run by egachine";
if (!EGachine.checkVersion(0,0,5)) throw "at least version 0.0.5 required";

for (x in argv) {
  print (x+": "+argv[x]);
}
print ("Version: "+EGachine.version.toSource());

// array - with current state of input devices
joypad=[];

viewport=Video.getViewport();
sx=viewport.sx;
sy=viewport.sy;
spriteSize=new V2D(sx/10,sx/10);

rackets=[];

EGachine.addResource("racket", '\
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\
');

EGachine.addResource("ball",'\
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\
a/Fa/B3G/wWKvKShjDb2ywAAAABJRU5ErkJggg==\
');

// handle input events
handleInput=function(i){
  joypad[i.dev]={x:i.x, y:i.y, buttons:i.buttons};
}


// clip value to [min,max]
function clip(min,max,value)
{
  if (value<min) return min;
  if (value>max) return max;
  return value;
}

function Ball() {
  this.speed=new V2D(-sx/3,-sx/3);
  this.degrees=new Degrees(0);
  this.rotspeed=sx/3;
  this.pos=new V2D(sx/2,sy/2);
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
  // collision with rackets
  for (var i=0;i<2;i++) {
    var dx=this.pos.x-rackets[i].pos.x;
    var dy=this.pos.y-rackets[i].pos.y;
    if (((Math.abs(dx)<spriteSize.x/2)&&(Math.abs(dy)<spriteSize.y))
	&&(((this.pos.x<sx/2)&&(this.speed.x<0))||((this.pos.x>sx/2)&&(this.speed.x>0)))) {
      this.speed.x*=-1;
      this.rotspeed*=-1;
    }
  }
  // points
  if (this.pos.x<0) {
    this.restart();
    print("point for player 1");
  }else if (this.pos.x>sx) {
    this.restart();
    print("point for player 0");
  }
}

// build our scenegraph
ball=new Ball();
root=new Node()
  .addNode(new Color(1,0,0,1).addNode(rackets[1]=new Sprite("racket",spriteSize,new V2D(spriteSize.x/2,sy/2))))
  .addNode(new Color(0,1,0,1).addNode(rackets[0]=new Sprite("racket",spriteSize,new V2D(sx-1-spriteSize.x/2,sy/2))))
  .addNode(new Sprite("ball",spriteSize,ball.pos,ball.degrees));

start=Timer.getTimeStamp();
last=start;
while(true){
  now=Timer.getTimeStamp();
  dt=(now-last)/1000000.0;
  last=now;

  // get input events
  Input.poll();
  // clear screen
  Video.clear();
  // paint scenegraph
  root.paint(dt);

  // move rackets
  for (var r=0;r<2;++r) {
    if (joypad[r]) {
      rackets[r].pos.y+=joypad[r].y*400.0*dt;
      rackets[r].pos.y=clip(spriteSize.y/2, sy-1-spriteSize.y/2, rackets[r].pos.y);
    }
  }
  // move ball
  ball.step(dt);

  // display new screen
  Video.swapBuffers();
}
