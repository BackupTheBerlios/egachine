// simple example (move quad with gamepad)

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.1");

function init() {
  var viewport=Video.getViewport();
  var sx=viewport[2],sy=viewport[3];
  var pos=new sg.V2D(sx/2,sy/2);
  var size=new sg.V2D(sx/10,sy/10);
  // array - with current state of input devices
  var joypad=[];

  EGachine.sceneGraph=new sg.Quad(size,pos);
  Input.addDevListener(function(i){
			 joypad[i.dev]={x:i.x, y:i.y, buttons:i.buttons};
		       });
  // register function to be called for each step in time
  EGachine.step(function(dt){
		  // move quad
		  var joy=joypad[0];
		  if (joy) {
		    pos.x+=joy.x*sx/4*dt;
		    pos.y+=joy.y*sy/4*dt;
		  }
		});
};

init();
