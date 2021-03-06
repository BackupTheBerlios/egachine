// simple example (move quad with gamepad)

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.2");

function init() {
  var viewport=Video.getViewport();
  var sx=viewport[2],sy=viewport[3];
  var pos=new sg.V2D(sx/2,sy/2);
  var size=new sg.V2D(sx/5,sy/5);
  // array - with current state of input devices
  var joypad=[];

  EGachine.sceneGraph=new sg.Rectangle(size,pos);
  Input.addDevListener(function(i){
			 joypad[i.dev]={x:i.x, y:i.y, buttons:i.buttons};
		       });
  // register function to be called for each step in time
  EGachine.step(function(dt){
		  // move quad
		  var joy=joypad[0];
		  if (joy) {
		    pos.x+=joy.x*sx/2*dt;
		    pos.y+=joy.y*sy/2*dt;
		  }
		});
};

init();
