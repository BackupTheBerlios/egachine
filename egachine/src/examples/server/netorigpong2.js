// networked pong clone
// this version uses a different aproach regearding the scenegraph
// the scenegraph is a movie which is updated during the game
// and you can jump to any point in time => replays come for free

// the choice is always how much is done on the client side and how much
// on the server side
// the one extrem is everything is done on the server and send to the client
// (heavy traffic - network interference prone)
// the other extrem is only input events are distributed to the clients
// which requires more specific client code

// this example is quite on the second side but tries to find a good compromise

/*
TODO:
noch was zum automatischen updaten von objekten
ich beobachte ja die properties - wenn ein objekt einem property zugewiesen
wird geht das evtl. garnicht und v.a. wird wohl nicht berücksichtigt, daß dieses
objekt bereits im baum hängt?
muß ich mir merken - hash tabelle ...
*/

try{

if (!EGachine.server) 
  throw new Error("This file must be run by egaserver");
if (!EGachine.checkVersion(0,0,7)) 
  throw new Error("at least version 0.0.7 required");

println("Server is now running\nListening for connections on port "+listenPort);
println("ATTENTION: at the moment the server is insecure.");

// our game uses an aspect ratio of 1.333:1
sx=1+1/3;
sy=1;
racketSize=new V2D(0.05,0.1);
racket=new Quad(racketSize);
rackets=[];
points=[];
ballSize=new V2D(0.025,0.025);
bgColor=[0.58,0.29,0.06,1];
ballColor=[1,1,1,1];
topBorder=sy/5;
bottomBorder=sy/10;
fieldMiddle=((sy-topBorder)+bottomBorder)/2;
ballStartPos=new V2D(sx/2,fieldMiddle);
borderColor=[1,1,1,1];
playerColors=[[0.84,0.51,0.29,1],[0.35,0.73,0.35,1]];

// object LinMover - linear movement
function LinMover(pos, speed, rot, rotspeed) {
  if (!pos) throw new Error("pos undefined");
  if (!pos instanceof V2D) throw new Error("pos not a V2D");
  this.pos=pos;
  this.speed=speed;
  this.rot=rot;
  this.rotspeed=rotspeed;
}
LinMover.prototype.getPos=function(time){
  return new V2D(this.pos.x+this.speed.x*time,this.pos.y+this.speed.y*time);
}
LinMover.prototype.getSpeed=function(time){
  return this.speed;
}
LinMover.prototype.getRot=function(time){
  var rot;
  if (!this.rot) rot=new Degrees(0);
  if (this.rotspeed) rot.inc(this.rotspeed.value*time);
  return rot;
}

moveUp=new V2D(1/3,0);
moveDown=new V2D(-1/3,0);
dontMove=new V2D(0,0);

// handle input events (from network)
Input.handleInput=function(i){
  if ((i.dev!=undefined)&&(i.dev>=0)&&(i.dev<2)) {
    var now=Timer.getTimeStamp();
    var pos=rackets[i.dev].getPos(now);
    if (!pos) throw new Error("hmm");
    if (i.y>0)
      rackets[i.dev].appendMove(now,new LinMover(pos,moveUp));
    else if (i.y<0)
      rackets[i.dev].appendMove(now,new LinMover(pos,moveDown));
    else
      rackets[i.dev].appendMove(now,new LinMover(pos,dontMove));
  }
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
  stream.write(h); \
  stream.write(msg); \
  stream.sync(); \
}; \
Video.setViewportCoords({left:0,right:"+sx+",bottom:0,top:"+sy+"});\
while (true) { \
  Video.clear(); \
  now=Timer.getTimeStamp()/1000000; \
  root.paint(now); \
  Input.poll(); \
  if (stream.inAvailable()>0) readMsg(); \
  Video.swapBuffers(); \
}\
};\
run();\
");

}

// derived object TimeSwitch
function TimeSwitch() {
}
TimeSwitch.prototype=new Node();
TimeSwitch.prototype.getInterval=function(time){
  // find active interval - TODO: use better algo (this array is sorted)
  for (var i=1;i<this.timeline.length;++i)
    if (this.timeline[i]>time) return i-1;
  return i-1;
}
TimeSwitch.prototype.paint=function(time){
  if (!this.children) return;
  var i=this.getInterval(time);
  this.children[i].paint(time-this.timeline[i]);
}
TimeSwitch.prototype.add=function(time,node){
  if (!this.timeline) this.timeline=[];
  Node.prototype.add.call(this,node);
  this.timeline.push(time);
  return this;
}

// derived object MultiMover
function MultiMover() {
}
MultiMover.prototype=new Node();
MultiMover.prototype.getInterval=function(time){
  // find active interval - TODO: use better algo (this array is sorted)
  if (!this.moves) throw new Error("no moves");
  if (!this.moves.length) throw new Error("no moves");
  for (var i=1;i<this.moves.length;++i)
    if (this.moves[i].start>time) return i-1;
  return i-1;
}
MultiMover.prototype.paint=function(time){
  if (!this.children) return;
  Video.pushMatrix();
  var pos=this.getPos(time);
  if (pos) Video.translate(pos.x,pos.y);
  var rot=this.getRot(time);
  if (rot) Video.rotate(rot.value);
  Node.prototype.paint.call(this,time);
  Video.popMatrix();
}
MultiMover.prototype.getPos=function(time){
  var i=this.getInterval(time);
  if (i<0) throw new Error("Out of range");
  if (i>=this.moves.length) throw new Error("Out of range");
  return this.moves[i].move.getPos(time-this.moves[i].start);
}
MultiMover.prototype.getSpeed=function(time){
  var i=this.getInterval(time);
  return this.moves[i].move.getSpeed(time-this.moves[i].start);
}
MultiMover.prototype.getRot=function(time){
  var i=this.getInterval(time);
  return this.moves[i].move.getRot(time-this.moves[i].start);
}
MultiMover.prototype.appendMove=function(time,_move){
  if (!this.moves) {
	this.moves=[];
	this.moves.push({start:time,move:_move});
  }else{
	var prev=this.moves[this.moves.length-1];
	if (time<=prev.start) throw new Error("you can only append moves later in time: "+time+" <= "+prev.start);
	this.moves.push({start:time,move:_move});
  }
  return this;
}
function Points(points) {
  this.points=points;
}
Points.prototype=new Node();
Points.prototype.paint=function(time){
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
  Node.prototype.paint.call(this,time);
}


// build our scenegraph
root=new Node()
  .add(new Color(bgColor)
       .add(new Quad(new V2D(sx,sy-topBorder-bottomBorder),new V2D(sx/2,fieldMiddle))))
  // racket of player one (right, green)
  .add(new Color(playerColors[0])
       .add(rackets[0]=new MultiMover()
		.appendMove(0,new LinMover(new V2D(sx-2*racketSize.x,fieldMiddle),dontMove))
		.add(racket)))
  // racket of player two (left, red)
  .add(new Color(playerColors[1])
       .add(rackets[1]=new MultiMover()
		.appendMove(0,new LinMover(new V2D(   2*racketSize.x,fieldMiddle),dontMove))
		.add(racket)))
  .add(new Color(borderColor)
       .add(new Quad(new V2D(sx,3*0.05/5),new V2D(sx/2,sy-topBorder+3*0.05/5/2)))
       .add(new Quad(new V2D(sx,2*0.05/5),new V2D(sx/2,bottomBorder-2*0.05/5/2))))
  .add(new Color(bgColor)
       .add(new Quad(new V2D(sx,    10*0.05/5),new V2D(sx/2,         sy-topBorder+3*0.05/5+10*0.05/5/2)))
       .add(new Quad(new V2D(sx-0.1,     0.01),new V2D(sx/2+0.1,sy-topBorder+3*0.05/5+10*0.05/5+0.01/2))))
  .add(new Color([0,0,0,1])
       .add(new Quad(new V2D(sx,bottomBorder-2*0.05/5),new V2D(sx/2,(bottomBorder-2*0.05/5)/2))))
  // the ball
  .add(new Color(ballColor)
       .add((ball=new MultiMover()
	    .appendMove(0,new LinMover(ballStartPos, new V2D(-1/3,-1/5)))
	    .add(new Quad(ballSize)))))
  // display points for player one
  .add(new Translate(new V2D(0,0.85*sy))
       .add(new Color(playerColors[0])
	    .add(new Translate(new V2D(0.8*sx,0))
		 .add(new Scale(new V2D(sx*0.05,sx*0.05))
		      .add((points[0]=new TimeSwitch().add(0,new Points("0",true)))))))
       // display points for player two
       .add(new Color(playerColors[1])
	    .add(new Translate(new V2D(0.2*sx,0))
		 .add(new Scale(new V2D(sx*0.05,sx*0.05))
		      .add((points[1]=new TimeSwitch().add(0,new Points("0",true))))))));

// distribute scenegraph to all clients
// NOTE: changes to the scenegraph are automatically distributed
// to the clients
Net.distribute(root,"root");

// main game loop
start=Timer.getTimeStamp();
last=0;

var ballPos=ball.getPos(0);
println("t:0 x:"+ballPos.x+" y:"+ballPos.y);

while(true) {
  now=(Timer.getTimeStamp()-start)/1000000;
  dt=(now-last)/1000000;
  last=now;

  // get input events
  Net.poll();

  // collision detection

  // get current ball position
  var ballPos=ball.getPos(now);
//  println("t:"+now+" x:"+ballPos.x+" y:"+ballPos.y);
  var ballSpeed=ball.getSpeed(now);
  var ycol=1;
  var xcol=1;

  // collission with walls
  if (ballPos.y<bottomBorder) {
    ballPos.y=bottomBorder;
    ycol=-1;
  }else if (ballPos.y>sy-topBorder){
    ballPos.y=sy-topBorder;
    ycol=-1;
  }

  // collision with rackets

  for (var i=0;i<2;i++) {
    // get current racket position
    var racketPos=rackets[i].getPos(now);

    var dx=ballPos.x-racketPos.x;
    var dy=ballPos.y-racketPos.y;
    if (((Math.abs(dx)<racketSize.x/2+ballSize.x/2)&&(Math.abs(dy)<racketSize.y/2+ballSize.y/2))
	&&
	(((ballPos.x<sx/2)&&(ballSpeed.x<0))
	 ||((ballPos.x>sx/2)&&(ballSpeed.x>0)))) {
      xcol=-1;
    }
  }
  // points
  if (ballPos.x<0) {
    ball.appendMove(now, new LinMover(ballStartPos, ballSpeed));
    points[0].add(now, new Points(points[0].children[points[0].children.length-1].points+1,true));
//    println("point");
  }else if (ballPos.x>sx) {
    ball.appendMove(now, new LinMover(ballStartPos, ballSpeed));
    points[1].add(now, new Points(points[1].children[points[1].children.length-1].points+1,true));
//    println("point");
  }else if ((xcol<0)||(ycol<0)){
    // collission response
//    println("collision");
    ball.appendMove(now, new LinMover(ballPos, new V2D(ballSpeed.x*xcol,ballSpeed.y*ycol)));
  }
  
  // send updates to clients
  Net.update(dt);
  // sleep a bit
  if (dt<15000) Timer.uSleep(15000-dt);
}
}catch(error){
  println(error);
  println(error.stack);
}
