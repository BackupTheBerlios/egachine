// this is to test sensitivity to refresh rate (and synchronization to screen refresh)
// this test uses the worst case a vertically moving bar

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.1");

var frameDiv=0;

function init() {
  var sx=4;
  var sy=3;
  var pos=new sg.V2D(sx/2,sy/2);
  var quadSize=new sg.V2D(sx/50,sy);
  var spriteSize=new sg.V2D(sx/5,sx/5);
  var test=[];
  var ctest=0;
  var speed=sx;
  var dir=new sg.Degrees(0);
  var marker=new sg.Color(1,0,0,1).add(new sg.Translate(new sg.V2D(sx/2,0)).add(new sg.Quad(new sg.V2D(quadSize.x,sy/20))));
  var unitSize=new sg.V2D(1,1);

  addResources();

  // setup some coordinate system
  Video.setViewportCoords({left:0,right:sx,top:sy,bottom:0});

  // register gamepad event listener (pressing right skips to next example)
  Input.addDevListener(function(i){
			 var j,g;
			 if (i.dev==0) {
			   if (i.x) {
			     frameDiv+=i.x;
			     frameDiv=Math.max(frameDiv,0);
			   }
			   if (i.y) {
			     ++ctest;
			     ctest=ctest%test.length;
			     EGachine.sceneGraph=test[ctest];
			   }
			 }else if (i.dev==1){
			   if (i.y) speed+=sx/20*i.y;
			   if (i.x) dir.value-=45*i.x;
			 }
		       });

  EGachine.sceneGraph=test[0]=new sg.Sprite("test", spriteSize, pos, dir);

  test[1]=new sg.Node()
    .add(new sg.Translate(pos)
	 .add(new sg.Rotate(dir)
	      .add(new sg.Scale(quadSize)
		   .add(new sg.Scale(new sg.V2D(1,1/4))
			.add(new sg.Translate(new sg.V2D(0,-2))
			     .add(new sg.Color(1,0,0,1)
				  .add(new sg.Quad(unitSize,new sg.V2D(0,3.5))))
			     .add(new sg.Color(0,1,0,1)
				  .add(new sg.Quad(unitSize,new sg.V2D(0,2.5))))
			     .add(new sg.Color(0,0,1,1)
				  .add(new sg.Quad(unitSize,new sg.V2D(0,1.5))))
			     .add(new sg.Color(1,1,1,1)
				  .add(new sg.Quad(unitSize,new sg.V2D(0,0.5)))))))))
    .add(marker);

  test[2]=new sg.Node().add(new sg.Translate(new sg.V2D(sx/2,sy/2))
			    .add(new sg.Quad(new sg.V2D(sx,sy))))
    .add(new sg.Color(0,0,0,1)
	 .add(new sg.Translate(pos)
	      .add(new sg.Rotate(dir)
		   .add(new sg.Quad(quadSize)))));
  
  // wrap value to [min,max[
  function wrap(min,max,value)
  {
    if (value<min) return wrap(min,max,max+(value-min));
    if (value>=max) return wrap(min,max,min+value-max);
    return value;
  };

  EGachine.step(function(dt){
		  //pos.x=wrap(0,sx,pos.x+sx/100);
		  pos.x=wrap(0,sx,pos.x+speed*Math.cos(dir.value/180*Math.PI)*dt);
		  pos.y=wrap(0,sy,pos.y+speed*Math.sin(dir.value/180*Math.PI)*dt);
		});
};

init();

(function(){
  var refreshRate=EGachine._refreshRate;
  var start,last,now,frames=0,tpf=Math.round(1/refreshRate*1000000),i;
  function print(x){Stream.stdout.write(x+"\n")};
  last=start=Timer.getTimeStamp();

  while (true) {
    now=Timer.getTimeStamp();
    dt=(now-last)/1000000.0;
    last=now;
    frames++;

    Input.poll();
    Video.clear();
    EGachine._step(dt);
    EGachine.sceneGraph.paint(now-start);
    gl.Finish();
    Video.swapBuffers();
    print(""+(now-start)/1000000+" "+frames);
    for (i=0;i<frameDiv;++i) {
      Input.poll();
      Video.clear();
      EGachine.sceneGraph.paint(now-start);
      gl.Finish();
      Video.swapBuffers();
    }
  };
 })();

function addResources(){
EGachine.addResource(
({name:"test", size:371, data:"\
iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAABmJLR0QA/wD/AP+gvaeTAAAACXBIWXM\
AAAsTAAALEwEAmpwYAAAAB3RJTUUH1QIXCzIhlGWwtAAAAB10RVh0Q29tbWVudABDcmVhdGVkIHdpdG\
ggVGhlIEdJTVDvZCVuAAAA10lEQVQ4y6WTMQ6CMBiFPwwbu4ETYDCByUu4ObB6IW/hBdy8BBMMJCQuT\
hB3R4PLI6m0AQMvaZr/7+vr/7evXt/3O1bA13xduP/sG8FB8xtogApolYuAFIiBQLnCrGDAC7gDN6AE\
OuVDIANOwBHYjlsYTr4DF6AeCT81HorzYWFjkBqdXE/0XIvTuAQqlT2HUlxLoDV6nkJnXO6PwCKYApF\
uew6huJZAqqeaQyauJRDrnZOJzYk4scsHgUzCH0YKXALIYblKnLOyJVCMxPYaAz4uk3lrv/MXvQIu39\
46kKYAAAAASUVORK5CYII=\
"}));
};
