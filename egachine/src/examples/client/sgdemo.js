// demo which should help to understand the scenegraph basics

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.1");

function init() {
  var sx=4;
  var sy=3;
  var middle=new sg.V2D(sx/2,sy/2);
  var example=[];
  var cExample=0;
  var quadSize=new sg.V2D(sx/10,sx/10);

  // setup some coordinate system
  Video.setViewportCoords({left:0,right:sx,top:sy,bottom:0});

  // register gamepad event listener (pressing right skips to next example)
  Input.addDevListener(function(i){
			 if (i.x<=0) return;
			 if (++cExample>=example.length) ejs.exit(true);
			 EGachine.sceneGraph=example[cExample];
		       });

  // quad in bottom left corner
  example[0]=new sg.Quad(quadSize);

  // quad moved to middle of the screen
  example[1]=new sg.Translate(middle).add(new sg.Quad(quadSize));

  // quad moved to middle of the screen and then rotated by 45 degrees
  example[2]=new sg.Translate(middle)
    .add(new sg.Rotate(new sg.Degrees(45))
	 .add(new sg.Quad(quadSize)));

  // give the quad some color
  // (Note: we simply reuse the scene from the example above)
  example[3]=new sg.Color(1,0,0).add(example[2]);

  // so far this has been like turtle graphics
  // but hey this is a scene graph and not a scene list

  // move to middle
  example[4]=new sg.Translate(middle)
    // draw a quad
    .add(new sg.Quad(quadSize))
    // move a bit right, set color to red and draw quad
    .add(new sg.Translate(new sg.V2D(middle.x/2,0))
	 .add(new sg.Color(1,0,0)
	      .add(new sg.Quad(quadSize))))
    // (automatically return back to middle and color white)
    // move a bit left and draw quad
    .add(new sg.Translate(new sg.V2D(-middle.x/2,0))
	 .add(new sg.Quad(quadSize)));

  // to see how to use the scene-graph for animations
  // see the simple.js example

  EGachine.step(function(){});

  EGachine.sceneGraph=example[0];
};

init();
