// networked pong clone
if (!EGachine.server) 
  throw new Error("This file must be run by egaserver");
if (!EGachine.checkVersion(0,0,7)) 
  throw new Error("at least version 0.0.7 required");

println("Server is now running\nListening for connections on port "+listenPort);
println("ATTENTION: at the moment the server is insecure.");

// our game uses an aspect ratio of 1.333:1
sx=1+1/3;
sy=1;
rackets=[];
points=[];
racketSize=new V2D(0.05,0.1);
ballSize=new V2D(0.025,0.025);
bgColor=[0.58,0.29,0.06,1];
ballColor=[1,1,1,1];
topBorder=sy/5;
bottomBorder=sy/10;
fieldMiddle=((sy-topBorder)+bottomBorder)/2;
borderColor=[1,1,1,1];
playerColors=[[0.84,0.51,0.29,1],[0.35,0.73,0.35,1]];


// handle input events (from network)
Input.handleInput=function(i){
  if ((i.dev!=undefined)&&(i.dev>=0)&&(i.dev<2))
    rackets[i.dev].speed.y=i.y*1/3;
}

// handle new client connection
function handleNewConnection(id,stream){
  // code we send to the client - which executes it
  // this is quite generic and should work for any similar networked game
  Net.sendTo(id,"\
function run(){if (!EGachine.checkVersion(0,0,7)) \
  throw new Error('at least version 0.0.7 required');\
Input.handleInput=function(i){\
  var msg=serialize(i); \
  var h=msg.length.convertTo(16,6); \
  stream.send(h); \
  stream.send(msg); \
  stream.sync(); \
}; \
Video.setViewportCoords({left:0,right:"+sx+",bottom:0,top:"+sy+"});\
last=Timer.getTimeStamp();\
while (true) { \
  Video.clear(); \
  now=Timer.getTimeStamp(); \
  dt=(now-last)/1000000; \
  last=now; \
  root.step(dt); \
  root.paint(dt); \
  Input.poll(); \
  if (stream.select(0)) readMsg(); \
  Video.swapBuffers(); \
}\
};\
run();\
");

}

// clip value to [min,max]
function clip(min,max,value)
{
  if (value<min) return min;
  if (value>max) return max;
  return value;
}

// our Ball object
function Ball() {
  this.speed=new V2D(-1/3,-1/5);
  this.degrees=new Degrees(0);
  this.pos=new V2D(sx/2,fieldMiddle);
}

Ball.prototype.restart=function(){
  this.pos.x=sx/2;
  this.pos.y=fieldMiddle;
}

Ball.prototype.step=function(dt){
  // collission with walls
  if (this.pos.y<bottomBorder) {
    this.pos.y=bottomBorder;
    this.speed.y*=-1;
  }else if (this.pos.y>sy-topBorder){
    this.pos.y=sy-topBorder;
    this.speed.y*=-1;
  }
  // collision with rackets
  for (var i=0;i<2;i++) {
    var dx=this.pos.x-rackets[i].children[0].pos.x;
    var dy=this.pos.y-rackets[i].children[0].pos.y;
    if (((Math.abs(dx)<racketSize.x/2+ballSize.x/2)&&(Math.abs(dy)<racketSize.y/2+ballSize.y/2))
	&&
	(((this.pos.x<sx/2)&&(this.speed.x<0))
	 ||((this.pos.x>sx/2)&&(this.speed.x>0)))) {
      this.speed.x*=-1;
    }
  }
  // points
  if (this.pos.x<0) {
    this.restart();
    ++(points[0].points);
  }else if (this.pos.x>sx) {
    this.restart();
    ++(points[1].points);
  }
}

ball=new Ball();

function Points(points) {
  this.points=points;
}
Points.prototype=new Node();
Points.prototype.paint=function(){
  var digits=["\
111\
101\
101\
101\
111","\
010\
010\
010\
010\
010","\
111\
001\
111\
100\
111","\
111\
001\
011\
001\
111","\
101\
101\
111\
001\
001","\
111\
100\
111\
001\
111","\
100\
100\
111\
101\
111","\
111\
001\
001\
001\
001","\
111\
101\
111\
101\
111","\
111\
101\
111\
001\
001"];
  Video.pushMatrix();
  Video.scale(1/5,1/5);
  Video.translate(0.5,-0.5+4);
  var i;
  var ps=this.points.toString();
  for (i=0;i<ps.length;++i) {
    var s=digits[ps.charAt(i)];
    var x;
    var y;
    for (y=0;y<5;++y) {
      for (x=0;x<3;++x) {
	if (s.charAt(x+y*3)=="1") Video.drawQuad(1,1);
	Video.translate(1,0);
      }
      Video.translate(-3,-1);
    }
    Video.translate(4,5);
  }
  Video.popMatrix();
  Node.prototype.paint.call(this);
}


// build our scenegraph
root=new Node()
  .add(new Color(bgColor)
       .add(new Quad(new V2D(sx,sy-topBorder-bottomBorder),new V2D(sx/2,fieldMiddle))))
  // racket of player one (right, green)
  .add(new Color(playerColors[0])
       .add(rackets[0]=new Mover(new V2D(0,0))
	    .add(new Quad(racketSize,
			  new V2D(sx-2*racketSize.x,fieldMiddle)))))
  // racket of player two (left, red)
  .add(new Color(playerColors[1])
       .add(rackets[1]=new Mover(new V2D(0,0))
	    .add(new Quad(racketSize,
			  new V2D(2*racketSize.x,fieldMiddle)))))
  .add(new Color(borderColor)
       .add(new Quad(new V2D(sx,3*0.05/5),new V2D(sx/2,sy-topBorder+3*0.05/5/2)))
       .add(new Quad(new V2D(sx,2*0.05/5),new V2D(sx/2,bottomBorder-2*0.05/5/2))))
  .add(new Color(bgColor)
       .add(new Quad(new V2D(sx,10*0.05/5),new V2D(sx/2,sy-topBorder+3*0.05/5+10*0.05/5/2)))
       .add(new Quad(new V2D(sx-0.1,0.01),new V2D(sx/2+0.1,sy-topBorder+3*0.05/5+10*0.05/5+0.01/2))))
  .add(new Color([0,0,0,1])
       .add(new Quad(new V2D(sx,bottomBorder-2*0.05/5),new V2D(sx/2,(bottomBorder-2*0.05/5)/2))))
  // the ball
  .add(new Color(ballColor)
       .add(new Mover(ball.speed)
	    .add(new Quad(ballSize,ball.pos,ball.degrees))))
  // display points for player one
  .add(new Translate(new V2D(0,0.85*sy))
       .add(new Color(playerColors[0])
	    .add(new Translate(new V2D(0.8*sx,0))
		 .add(new Scale(new V2D(sx*0.05,sx*0.05))
		      .add((points[0]=new Points("0",true))))))
       // display points for player two
       .add(new Color(playerColors[1])
	    .add(new Translate(new V2D(0.2*sx,0))
		 .add(new Scale(new V2D(sx*0.05,sx*0.05))
		      .add((points[1]=new Points("0",true)))))));

// distribute scenegraph to all clients
// NOTE: changes to the scenegraph are automatically distributed
// to the clients
Net.distribute(root,"root");

// main game loop
start=Timer.getTimeStamp();
last=start;

while(true) {
  now=Timer.getTimeStamp();
  dt=(now-last)/1000000;
  last=now;

  // get input events
  Net.poll();
  // update scenegraph
  root.step(dt);
  // move ball
  ball.step(dt);
  // send updates to clients
  Net.update(dt);
  // sleep a bit
  if (dt<15000) Timer.uSleep(15000-dt);
}
