// networked pong clone
// this version uses a different aproach regearding the scenegraph
// the scenegraph is a movie which is created during the game
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
  ich könnte die kollisionen auch im voraus berechnen
  so daß der server wirklich poll(Infinity) machen kann
*/

if (!EGachine.server) 
  throw new Error("This file must be run by egaserver");
if (!EGachine.checkVersion(0,1,1)) 
  throw new Error("at least version 0.1.1 required");

println("Server is now running\nListening for connections on port "+listenPort);
println("ATTENTION: at the moment the server is insecure.");

// our game uses an aspect ratio of 1.333:1
getProfile=false;
sx=1+1/3;
sy=1;
racketSize=new sg.V2D(0.05,0.1);
racket=new sg.Quad(racketSize);
rackets=[];
points=[];
ballSize=new sg.V2D(0.025,0.025);
bgColor=[0.58,0.29,0.06,1];
topBorder=sy/5;
bottomBorder=sy/10;
fieldMiddle=((sy-topBorder)+bottomBorder)/2;
ballStartPos=new sg.V2D(sx/2,fieldMiddle);
borderColor=[1,1,1,1];
moveUp=new sg.V2D(0,1/2/1000000);
moveDown=new sg.V2D(0,-1/2/1000000);
dontMove=new sg.V2D(0,0);
// wished step size
stepSize=40000;

function ballColor(alpha){return [1,1,1,alpha];};

function playerColors(pl,alpha){
  if (!alpha) alpha=1;
  if (pl==0)
    return [0.84,0.51,0.29,alpha];
  return [0.35,0.73,0.35,alpha];
};

function getNow() {
  return Timer.getTimeStamp()-start;
}

function profile(f,maxDt)
{
  if (!getProfile) return f();
  if (!maxDt) maxDt=stepSize;
  var s;
  var dt;
  var ret;
  s=Timer.getTimeStamp();
  ret=f();
  if ((dt=(Timer.getTimeStamp()-s))>maxDt) {
    println(uneval(f)+': '+dt);
    stdout.sync();
  }
  return ret;
};

// handle input events (from network)
Input.handleInput=function(i){
  if ((i.dev!=undefined)&&(i.dev>=0)&&(i.dev<2)) {
    var now=getNow();
    var pos=rackets[i.dev].getPos(now);
    if (i.y>0)
      rackets[i.dev].appendMove(now,new LinMover(pos,moveUp));
    else if (i.y<0)
      rackets[i.dev].appendMove(now,new LinMover(pos,moveDown));
    else
      rackets[i.dev].appendMove(now,new LinMover(pos,dontMove));
  }
};

// handle new client connection
function handleNewConnection(id,stream){
  // code we send to the client - which executes it
  // this is quite generic and should work for any similar networked game
  Net.server.remoteEval(id,"\
function run(){if (!EGachine.checkVersion(0,1,1))			\
  throw new Error('at least version 0.1.1 required');			\
objReader=new ObjectReader(stream);					\
Input.handleInput=function(i){						\
  if (i.x) {start+=i.x*1000000;stderr.write(start+'\\n');return;};	\
  var msg=jsolait.lang.objToJson(i);					\
  var h=msg.length.convertTo(16,6);					\
  stream.write(h);							\
  stream.write(msg);							\
  stream.sync();							\
};									\
function trace(f){var debug=0;if (debug) var s=Timer.getTimeStamp();	\
  f();									\
  if (debug) stdout.write(						\
                    uneval(f)+': '+(Timer.getTimeStamp()-s)+'\\n');	\
};									\
function getNow() {return Timer.getTimeStamp()-start;};			\
Video.setViewportCoords({left:0,right:"+sx+",bottom:0,top:"+sy+"});	\
start=Timer.getTimeStamp()-("+(now-200000)+");				\
now=last=getNow();							\
stepSize=1000000/20;							\
while (true) {								\
  while ((dt=((now=getNow())-last))<(stepSize-10000)) {			\
    Timer.uSleep(stepSize-dt);						\
  };									\
  stdout.write('dt:'+dt+'\\n');						\
  last=now;								\
  Input.poll();								\
  if (stream.inAvailable()>0) objReader.read();				\
  if (typeof root != 'undefined') trace(function(){root.paint(now);});	\
  Video.swapBuffers();							\
  Video.clear();							\
}									\
};									\
run();									\
");

};

//! simulate
function advanceTo(time) {
  // collision detection

  // get current ball position
  var ballPos=ball.getPos(time);
  var ballSpeed=ball.getSpeed(time);
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
    var racketPos=rackets[i].getPos(time);

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
    println("point");
    profile(function(){
	      ball.appendMove(time, new LinMover(ballStartPos, ballSpeed));
	    },stepSize/2);
    profile(function(){
	      points[0].add(time, new Points(1+points[0][points[0].children-1].points,true));
	    },stepSize/2);
  }else if (ballPos.x>sx) {
    ball.appendMove(time, new LinMover(ballStartPos, ballSpeed));
    points[1].add(time, new Points(1+points[1][points[1].children-1].points,true));
  }else if ((xcol<0)||(ycol<0)){
    // collission response
    ball.appendMove(time, new LinMover(ballPos, new sg.V2D(ballSpeed.x*xcol,ballSpeed.y*ycol)));
  }
};

// object TimeShift
TimeShift=constructor(function(shift) {
			this.shift=shift;
		      });
TimeShift.prototype=new sg.Node();
TimeShift.prototype.paint=function(time){
  Node.prototype.paint.call(this,time-this.shift);
};

// object LinMover - linear movement
LinMover=constructor(function(pos, speed, rot, rotspeed) {
		       if (!pos) throw new Error("pos undefined");
		       if (!pos instanceof V2D) throw new Error("pos not a V2D");
		       this.pos=pos;
		       this.speed=speed;
		       this.rot=rot;
		       this.rotspeed=rotspeed;
		     });
LinMover.prototype.getPos=function(time){
  return new sg.V2D(this.pos.x+this.speed.x*time,this.pos.y+this.speed.y*time);
};
LinMover.prototype.getSpeed=function(time){
  return this.speed;
};
LinMover.prototype.getRot=function(time){
  var rot;
  if (!this.rot) rot=new sg.Degrees(0);
  if (this.rotspeed) rot.inc(this.rotspeed.value*time);
  return rot;
};


// monitorable array object
Marray=constructor(function(){this.length=0;});
Marray.prototype.push=function(v){
  var t=this;
  profile(function(){t[t.length]=v;},stepSize/4);
  this.length+=1;
};


// derived object TimeSwitch
TimeSwitch=constructor(function () {});
TimeSwitch.prototype=new sg.Node();
TimeSwitch.prototype.getInterval=function(time){
  // find active interval - TODO: use better algo (this array is sorted)
  var m=this.timeline;
  return findLastNotGreater(function(i){return m[i]},m.length,time);
};
TimeSwitch.prototype.paint=function(time){
  var i;
  if (this.children && ((i=this.getInterval(time))>=0))
    this[i].paint(time-this.timeline[i]);
};
TimeSwitch.prototype.add=function(time,node){
  if (!this.timeline) this.timeline=new Marray();
  Node.prototype.add.call(this,node);
  this.timeline.push(time);
  return this;
};

// derived object MultiMover
MultiMover=constructor(function(){});
MultiMover.prototype=new sg.Node();
MultiMover.prototype.getInterval=function(time){
  // find active interval
  var m=this.moves;
  var i=findLastNotGreater(function(i){return m[i].start},m.length,time);
  if (i<0) throw Error("no match");
  return i;
};
MultiMover.prototype.paint=function(time){
  var e;
  var pos;
  var rot;
  if (!this.children) return;
  Video.pushMatrix();
  try{
    if ((pos=this.getPos(time))) Video.translate(pos.x,pos.y);
    if ((rot=this.getRot(time))) Video.rotate(rot.value);
    Node.prototype.paint.call(this,time);
  }catch(e){if (e.message!="no match") throw e;}
  Video.popMatrix();
};
MultiMover.prototype.getPos=function(time){
  var i=this.getInterval(time);
  return this.moves[i].move.getPos(time-this.moves[i].start);
};
MultiMover.prototype.getSpeed=function(time){
  var i=this.getInterval(time);
  return this.moves[i].move.getSpeed(time-this.moves[i].start);
};
MultiMover.prototype.getRot=function(time){
  var i=this.getInterval(time);
  return this.moves[i].move.getRot(time-this.moves[i].start);
};
MultiMover.prototype.appendMove=function(time,_move){
  if (!this.moves) {
    this.moves=new Marray();
    this.moves.push({start:time,move:_move});
  }else{
    var prev=this.moves[this.moves.length-1];
    if (time<=prev.start) throw new Error("you can only append moves later in time: "+time+" <= "+prev.start);
    this.moves.push({start:time,move:_move});
  }
  return this;
};

//! point display object
Points=constructor(function(points) {
		     this.points=points;
		   });
Points.prototype.digits=["\
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
Points.prototype.paint=function(){
  // todo: this is slow as hell?
  // a display list for each digit probably would be a good idea
  Video.pushMatrix();
  Video.scale(1/5,1/5);
  Video.translate(0.5,-0.5+4);
  var i;
  var ps=this.points.toString();
  for (i=0;i<ps.length;++i) {
    var s=this.digits[ps.charAt(i)];
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
};

// build our scenegraph
root=new sg.Node()
  .add(new sg.Color(bgColor)
       .add(new sg.Quad(new sg.V2D(sx,sy-topBorder-bottomBorder),new sg.V2D(sx/2,fieldMiddle))))
// racket of player one (right, green)
  .add(new sg.Color(playerColors(0))
       .add(rackets[0]=new MultiMover()
	    .appendMove(0,new LinMover(new sg.V2D(sx-2*racketSize.x,fieldMiddle),dontMove))
	    .add(racket)))
// racket of player two (left, red)
  .add(new sg.Color(playerColors(1))
       .add(rackets[1]=new MultiMover()
	    .appendMove(0,new LinMover(new sg.V2D(   2*racketSize.x,fieldMiddle),dontMove))
	    .add(racket)))
  .add(new sg.Color(borderColor)
       .add(new sg.Quad(new sg.V2D(sx,3*0.05/5),new sg.V2D(sx/2,sy-topBorder+3*0.05/5/2)))
       .add(new sg.Quad(new sg.V2D(sx,2*0.05/5),new sg.V2D(sx/2,bottomBorder-2*0.05/5/2))))
  .add(new sg.Color(bgColor)
       .add(new sg.Quad(new sg.V2D(sx,    10*0.05/5),new sg.V2D(sx/2,         sy-topBorder+3*0.05/5+10*0.05/5/2)))
       .add(new sg.Quad(new sg.V2D(sx-0.1,     0.01),new sg.V2D(sx/2+0.1,sy-topBorder+3*0.05/5+10*0.05/5+0.01/2))))
  .add(new sg.Color([0,0,0,1])
       .add(new sg.Quad(new sg.V2D(sx,bottomBorder-2*0.05/5),new sg.V2D(sx/2,(bottomBorder-2*0.05/5)/2))))
// the ball
  .add(new sg.Color(ballColor(1))
       .add((ball=new MultiMover()
	     .appendMove(0,new LinMover(ballStartPos, new sg.V2D(-1/3/1000000,-1/5/1000000)))
	     .add(new sg.Quad(ballSize)))))
  .add(new sg.Translate(new sg.V2D(0,0.85*sy))
       // display points for player one
       .add(new sg.Color(playerColors(0))
	    .add(new sg.Translate(new sg.V2D(0.8*sx,0))
		 .add(new sg.Scale(new sg.V2D(sx*0.05,sx*0.05))
		      .add((points[0]=new TimeSwitch().add(0,new Points(0,true)))))))
       // display points for player two
       .add(new sg.Color(playerColors(1))
	    .add(new sg.Translate(new sg.V2D(0.2*sx,0))
		 .add(new sg.Scale(new sg.V2D(sx*0.05,sx*0.05))
		      .add((points[1]=new TimeSwitch().add(0,new Points(0,true))))))));

// add some motion blur effects
if (true)
  (function(){
    var i;
    var shadows=3;
    root
      .add(new sg.Color(playerColors(0,0.5))
	   .add(new TimeShift(50000)
		.add(rackets[0])))
      .add(new sg.Color(playerColors(1,0.5))
	   .add(new TimeShift(50000)
		.add(rackets[1])));
    for (i=1;i<=shadows;++i)
      root.add(new sg.Color(ballColor(1-i/(shadows+2)))
	       .add(new TimeShift(1000000*i/20).add(ball)));
  })();

// distribute scenegraph to all clients
// NOTE: changes to the scenegraph are automatically distributed
// to the clients
Net.server.distribute(root,"root");
start=Timer.getTimeStamp();
now=last=getNow();
simTime=0;

// main game loop
while(true) {
  // get input events and sleep a bit
  while ((dt=((now=getNow())-last))<(stepSize-10000)) {
    profile(function(){Net.server.poll(stepSize-dt);});
  };
  //  stdout.write("dt:"+dt+"\n");
  
  // forward simulation
  while ( (now-simTime) > stepSize) {
    profile(function(){advanceTo(simTime+=stepSize);});
  };
  last=now;
  
  // send updates to clients
  profile(function(){Net.server.update();});
};
