TODO: this does not work in the moment

/*
  Until now we assumed we have got a fixed sized screen of 1024x768
  this is bad because:

  - perhaps the JGachine runs on a small display (there is no 1024x768 resolution available)
  - JGachine runs on a desktop system with a window manager and the window could be resized
  - it is to slow with 1024x768
  - ...
  => Now we try to change the game to not assume any resolution

  Screens differ in resolution and in aspect ratio. (and in real size)

  We could try to react to aspect ratio changes in a intelligent manner (change layout, ..)
  for now we are happy if it all works and the image isn't distorted.

  Since 1.333...:1 is a very common aspect ratio we use a coordinate system like that:
  x: left<=-1.333<=+1.333<=right
  y: bottom<=-1<=+1<=top

  if the aspect ratio is 1.333..:1 everything is fine
  if the ratio is 2.666..:1 we depend only on half of the screen (the rest could be filled with
  unimportant stuff or simply be black)

  we replaced 1024/2 with Pong.width/2 everyhere
  and 768/2 with Pong.height/2 everywhere
  
  then we adjusted speeds and similar stuff approximately to our new coordinate system
  this means: 1024 gets 2*1.33... => newx=oldx/1024*(2+1/3)

  we replaced the sprite size (64) with Pong.spriteSize
*/

//! a racket
/*!
  we start with the rackets

  a pong game has 2 rackets
  they can move up and down
*/
function RacketMover(pos){
  this.pos=pos;
  this.vel=0;
}
RacketMover.prototype.step=function(dt){
  // little physics: s=V*t (assuming V is constant)
  this.pos.y+=this.vel*dt;

  // do not move out of the screen
  if (this.pos.y>Pong.height/2)
    this.pos.y=Pong.height/2;
  else if (this.pos.y<-Pong.height/2)
    this.pos.y=-Pong.height/2;
}

//! a ball
/*!
  a pong game needs of course a ball
*/
function Ball(){
  this.pos=new V2D(0,0);
  this.dir=0;
  this.ddir=1;
  this.vel=new V2D(-0.45,-0.45);
}

//! is called after a player gets a point
Ball.prototype.restart=function(){
  this.pos.x=this.pos.y=0;
  this.vel.normalize();
  this.vel.scale(0.7f);
}

Ball.prototype.step=function(dt){
  // the ball can move in x and y directions
  this.pos.x+=this.vel.x*dt;
  this.pos.y+=this.vel.y*dt;

  // check collision with upper border
  if (this.pos.y>Pong.height/2) {
    this.pos.y=Pong.height/2;
    this.vel.y=-this.vel.y;
  }else if (this.pos.y<-Pong.height/2){
    this.pos.y=-Pong.height/2;
    this.vel.y=-this.vel.y;
  }
  // check left/right out
  if (this.pos.x<-Pong.width/2) {
    restart();
    Pong.win(1);
  }else if (this.pos.x>Pong.width/2) {
    restart();
    Pong.win(0);
  }
  // check if racket hits
  var srx=Pong.width/2-Pong.spriteSize-Pong.spriteSize/2;
  var erx=srx+Pong.spriteSize;
  var ydiff=this.pos.y-Pong.racket[0].pos.y;
  if (((this.pos.x>-erx)&&(this.pos.x<-srx)) && (Math.abs(ydiff)<Pong.spriteSize/2) && (this.vel.x<0)) {
    this.vel.x=-this.vel.x*(1.0f+(float)Pong.devState[0].x/5);
    this.vel.y+=ydiff*10;
    this.ddir=-this.ddir;
  }
  ydiff=this.pos.y-Pong.racket[1].pos.y;
  if (((pos.x> srx)&&(pos.x< erx)) && (Math.abs(ydiff)<Pong.spriteSize/2) && (this.vel.x>0)) {
    this.vel.x=-this.vel.x*(1.0f-(float)Pong.devState[1].x/5);
    this.vel.y+=ydiff*10;
    this.ddir=-this.ddir;
  }
  
  this.dir+=this.ddir*dt*this.vel.length()*438;
}


//! ball sprite
/*!
  we now have the physics of the ball right
  but of course we must paint it

  we paint a ball consisting of:
  a ball sprite + light map + shadow
*/
function Ballsprite() {
  this.pos = new V2D();
  this.size=new V2D(Pong.spriteSize,Pong.spriteSize);

  // node which draws rotated and scaled sprite without shadow (the ball)
  ball=new Translate(this.pos);
  this.rotate = new Rotate();
  ball.addNode(this.rotate.addNode(new Scale(this.size).addNode(new Texture("ball.png"))));

  // node which draws the lightmap
  lightMap=new Translate(this.pos).addNode(new Scale(this.size).addNode(new Texture("ball-light.png")));

  // node which draws the shadow
  shadow=new Translate(new V2D(12,-12)).addNode(new AdjustColor(new Color(0,0,0,0.3)).addNode(ball));

  // put it all together
  this.addNode(shadow)
    .addNode(ball)
    .addNode(lightMap);
}
BallSprite.prototype=new Node();
BallSprite.prototype.constructor=BallSprite;


//! paint blured ballsprite
/*!
  now lets get a bit fancy - we have a lighted ball with shadow
  but to get a visualization for the speed and to make the animation more smooth
  let us add a bluring effect

  the idea is simply to blend old positions of the ball
*/
function BluredBallSprite() {
  // we simply create some BallSprite objects
  this.blured = 6;
  this.balls=new Array(this.blured);
  for (var i=0;i<this.blured;++i) {
    this.balls[i]=new BallSprite();
    node = this.balls[i];
    if (i<blured-1) {
      // we blend all ball sprites except the last one and start with an alpha of startAlpha and fade
      // out linear
      startAlpha=0.1f;
      node = new AdjustColor(new Color(1.0,1.0,1.0,startAlpha*(i+1)/(this.blured-1) ) ).addNode(node);
    }
    addNode(node);
  }
}
BluredBallSprite.prototype=new Node();
BluredBallSprite.prototype.constructor=BluredBallSprite;
BluredBallSprite.prototype.rotateTo(dir){
  this.balls[this.blured-1].rotate.r=dir;
}
BluredBallSprite.prototype.moveTo(pos){
  for (var i=0;i<this.blured-1;++i)
    this.balls[i].pos.set(this.balls[i+1].pos);
  this.balls[blured-1].pos.set(pos);
}


HIER
TODO
xxx

//! moveable and rotateable sprite with shadow (which is blured)
/*!
  \todo fix this - nearly the same as above .... 
*/
class ShadowedSprite extends Node
{
    ShadowedSprite(Node sprite,
		   Vector2f _pos,
		   float dir) 
    {
	pos=new Vector2f[blured];
	for (int i=0;i<blured;++i) {
	    pos[i]=new Vector2f(_pos);
	    
	    // node which draws rotated and scaled sprite without shadow
	    Translate t=new Translate(pos[i]);
	    rotate = new Rotate(dir);
	    t.addNode(rotate.addNode(sprite));

	    // node which draws a shadow (black, alpha blended sprite)
	    AdjustColor shadow = (AdjustColor)new AdjustColor(new Color(0.0f,0.0f,0.0f,0.3f)).addNode(t);

	    Node base = this;
	    if (i<blured-1) {
		base = new AdjustColor(new Color(1.0f,1.0f,1.0f,0.1f*((float)i+1)/(blured-1) ) );
		addNode(base);
	    }
	    base
		.addNode(new Translate(new Vector2f(6.0f,-6.0f)).addNode(shadow))
		.addNode(new Translate(new Vector2f(12.0f,-12.0f)).addNode(shadow))
		.addNode(t);
	}
    }
    void rotateTo(float dir){
	rotate.r=dir;
    }
    void moveTo(Vector2f _pos){
	//	if ((++moved%2)==0)
	for (int i=0;i<blured-1;++i)
	    pos[i].set(pos[i+1]);
	pos[blured-1].set(_pos);
    }

    // how many sprites do we paint
    static final int blured = 8;

    Rotate rotate;
    Vector2f pos[];
    int moved=0;
}

//! simple pong clone
/*!
  now let us put it all together
  
  we must implement Runnable
*/
public class Pong implements Runnable
{
    Pong()
    {
	// how many input devices are available?
	int devs=JGachine.numDevices();
	// create array holding the current state of each input device
	devState=new DevState[devs];
	// create 2 rackets and corresponding sprites
	racket=new RacketMover[2];
	racketSprite=new ShadowedSprite[2];

	// don't forget to initialize our input state
	for (int i=0;i<devs;++i) {
	    devState[i]=new DevState((byte)i);
	}
	// register our input callbacks (slots)
	JGachine.input.connect(new Slot(this,"handleDevState"));
	JGachine.input.connect(new Slot(this,"handleResize"));
    }

    //! handle input - is called when the state of a device changed
    public void handleDevState(DevState s)
    {
	// we are only interested in the first 2 devices
	if (s.devno>1) return;
	// debug("New Input: "+s.devno+": "+s.x+s.y+s.buttons);
	devState[s.devno]=s;
	// update racket velocity
	racket[s.devno].vel=s.y*0.92f;
    }

    //! handle resize events
    public void handleResize(Resize r)
    {
	int nw=r.width;
	int nh=r.height;
	if (nw==0) nw=1;
	if (nh==0) nh=1;

	JGachine.setViewport(0,0,nw,nh);

	// we want 1.333:1 (or 2.666:2)
	// nw equals 2.666
	// => nh equals nh/nw*2.666
	float w=2.0f*(1.0f+(1.0f/3.0f));
	float h=(float)nh/(float)nw*w;
	// now h is possibly < 2 - if so we must re-adjust
	if (h<2.0f) {
	    h=2.0f;
	    w=(float)nw/nh*h;
	}
	JGachine.setViewportCoordinates(-w/2,w/2,
					-h/2,h/2,
					-100,+100);
    }
    
    //! is called if a player get's a point
    static void win(int who) {
	String s=new Integer(++points[who]).toString();
	if (s.length()<2) s="0"+s;
	pointsText[who].set(s);
    }

    public void run()  throws java.io.IOException
    {
	// set coordinate system
	JGachine.setViewportCoordinates(-width/2,width/2,
					-height/2,height/2,
					-100,+100);

	// create sceneGraph

	// our scene graph root Node
	Node sceneGraph = new Node();

	// set up camera (screen middle is now 0/0)
	Camera camera = new Camera(new Vector2f(0,0));

	// add background
	int bgTextureID=JGachine.createTexture("data:back.jpg");
	sceneGraph
	    .addNode(camera.addNode(new Scale(new Vector2f(Pong.width,Pong.width)).addNode(new Sprite(bgTextureID))));

	// add rackets
	for (int i=0;i<racket.length;++i) {
	    float xpos=Pong.width/2-Pong.spriteSize;
	    racket[i]=new RacketMover(new Vector2f((i==0)?-xpos:xpos,0));
	    float r=( i   %3)!=0 ? 0.6f:1.0f;
	    float g=((i+1)%3)!=0 ? 0.4f:1.0f;
	    float b=((i+2)%3)!=0 ? 0.6f:1.0f;
	    racketSprite[i]=new ShadowedSprite(new Scale(new Vector2f(Pong.spriteSize,Pong.spriteSize)).addNode(new Sprite(JGachine.createTexture("data:car.png"))),racket[i].pos,0);
	    camera.addNode(new AdjustColor(new Color(r,g,b,1.0f)).addNode(racketSprite[i]));
	}

	// add ball
	Ball ball = new Ball();
	BluredBallSprite ballSprite= new BluredBallSprite();
	camera.addNode(ballSprite);

	// add point displays
	pointsText = new Text[2];
	pointsText[0]=new Text("00");
	pointsText[1]=new Text("00");

	// text should be half the size of the ball and rackets
	final float textSize=Pong.spriteSize/2;
	final float border=textSize/10;
	camera
	    .addNode(new AdjustColor(new Color(1.0f,0.4f,0.6f,0.9f)).addNode(new Translate(new Vector2f(-Pong.width/2   ,Pong.height/2-textSize-border)).addNode(new Scale(new Vector2f(textSize,textSize)).addNode(pointsText[0]))))
	    .addNode(new AdjustColor(new Color(0.6f,0.4f,1.0f,0.9f)).addNode(new Translate(new Vector2f( Pong.width/2-Pong.spriteSize,Pong.height/2-textSize-border)).addNode(new Scale(new Vector2f(textSize,textSize)).addNode(pointsText[1]))));
	

	long start = JGachine.time();
	long last = start;
	long mark = last;
	long 
	    cur=0,
	    dt=0,
	    frame=0;
	float fdt=0.02f;
	float averagedt=fdt;
	float dterror=0;
	float fakedt;

	// main game loop
	while (!JGachine.quit) {
	    // clear screen
	    JGachine.clear();
	    // get input
	    JGachine.poll();
	    // move rackets
	    for (int i=0;i < racket.length;++i) {
		racket[i].step(fdt);
		racketSprite[i].moveTo(racket[i].pos);
	    }
	    // move ball
	    ball.step(fdt);
	    ballSprite.rotateTo(ball.dir);
	    ballSprite.moveTo(ball.pos);

	    // paint
	    sceneGraph.apply();
	    JGachine.swapBuffers();
	    ++frame;
	    cur = JGachine.time();
	    dt = cur - last;

	    // give other processes some time
	    while (dt<20000) {
		debug("sleep");
		// JGachine.uSleep(20000-dt);
		JGachine.uSleep(19000);
		cur = JGachine.time();
		dt = cur - last;
	    }
	    while (dt<30000) {
		cur = JGachine.time();
		dt = cur - last;
		}
	    
	    last = cur;
	    fdt=(float)dt/1000000.0f;

	    // smooth the stepsize
	    fakedt=averagedt+dterror/40;
	    averagedt=(averagedt*19+fakedt)/20;
	    dterror+=fdt-fakedt;
	    fdt=fakedt;

	    debug("fdt: "+fdt+" error: "+dterror);
	    // debug("mem: "+Runtime.getRuntime().freeMemory());

	    // after 200 frames update the fps counter
	    if (frame%200 == 0) {
		//		debug("run gc");
		//		Runtime.getRuntime().gc();

		long diff = last - mark;
		mark=last;
		float sec=(float)diff/1000000.0f;
		float fps=200.0f/sec;
	    }
	}
    }

    static protected void debug(String s){
	//	System.out.println("Pong.java: "+s);
    }

    //! our rackets
    static RacketMover[] racket;
    static ShadowedSprite[] racketSprite;

    //! state of our input devices
    static DevState[] devState;
    //! points
    static Text pointsText[];
    static int points[]={0,0};

    //! size of ball and rackets
    static final float spriteSize=0.15f;

    //! width of screen
    static final float width=2.0f*(1.0f+(1.0f/3.0f));
    //! height of screen
    static final float height=2.0f;
}
