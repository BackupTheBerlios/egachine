/* 
   this is taken from monkeyglobs
   modified for egachine by Jens Thiele, 2004

   (c) 2002 Mike Phillips
   glxgears implementation in js
   GPL v2, blah, blah, blah
   Shameslessly hacked from glxgears.c and test.js
*/

if ((typeof EGachine == 'undefined')||(!EGachine.client))
  throw "This file must be run by egachine";
EGachine.checkVersion("0.1.2");

function init() {
  var gl=ejs.ModuleLoader.get("gl");
  var stdout=ejs.ModuleLoader.get("Stream").stdout;

  var view_rotx = 20.0;
  var view_roty = 30.0;
  var view_rotz = 0.0;
  var gear1 = 0;
  var gear2 = 0;
  var gear3 = 0;
  var angle = 0.0;
  var t0 = -1;
  var frames = 0;
  var seconds;
  var time;
  var lasttime;
  var fps;
  // array - with current state of input devices
  var joypad=[];

  Input.addDevListener(function(i)
		       {
			 joypad[i.dev]=i;
		       });

  /*
   *
   *  Draw a gear wheel.  You'll probably want to call this function when
   *  building a display list since we do a lot of trig here.
   * 
   *  Input:  inner_radius - radius of hole at center
   *          outer_radius - radius at center of teeth
   *          width - width of gear
   *          teeth - number of teeth
   *          tooth_depth - depth of tooth
   */
  function gear(inner_radius, outer_radius, width, teeth, tooth_depth)
  {
    var i;
    var r0, r1, r2;
    var angle, da;
    var u, v, len;

    r0 = inner_radius;
    r1 = outer_radius - tooth_depth / 2.0;
    r2 = outer_radius + tooth_depth / 2.0;

    da = 2.0 * Math.PI / teeth / 4.0;

    gl.ShadeModel(GL_FLAT);

    gl.Normal3f(0.0, 0.0, 1.0);

    /* draw front face */
    gl.Begin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;
      gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		  width * 0.5);
      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  width * 0.5);
      if (i < teeth) {
	gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		    width * 0.5);
	gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		    r1 * Math.sin(angle + 3 * da), width * 0.5);
      }
    }
    gl.End();

    /* draw front sides of teeth */
    gl.Begin(GL_QUADS);
    da = 2.0 * Math.PI / teeth / 4.0;
    for (i = 0; i < teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;

      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + da), r2 * Math.sin(angle + da),
		  width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + 2 * da),
		  r2 * Math.sin(angle + 2 * da), width * 0.5);
      gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		  r1 * Math.sin(angle + 3 * da), width * 0.5);
    }
    gl.End();

    gl.Normal3f(0.0, 0.0, -1.0);

    /* draw back face */
    gl.Begin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;
      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  -width * 0.5);
      gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		  -width * 0.5);
      if (i < teeth) {
	gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		    r1 * Math.sin(angle + 3 * da), -width * 0.5);
	gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		    -width * 0.5);
      }
    }
    gl.End();

    /* draw back sides of teeth */
    gl.Begin(GL_QUADS);
    da = 2.0 * Math.PI / teeth / 4.0;
    for (i = 0; i < teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;

      gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		  r1 * Math.sin(angle + 3 * da), -width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + 2 * da),
		  r2 * Math.sin(angle + 2 * da), -width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + da), r2 * Math.sin(angle + da),
		  -width * 0.5);
      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  -width * 0.5);
    }
    gl.End();

    /* draw outward faces of teeth */
    gl.Begin(GL_QUAD_STRIP);
    for (i = 0; i < teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;

      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  width * 0.5);
      gl.Vertex3f(r1 * Math.cos(angle), r1 * Math.sin(angle),
		  -width * 0.5);
      u = r2 * Math.cos(angle + da) - r1 * Math.cos(angle);
      v = r2 * Math.sin(angle + da) - r1 * Math.sin(angle);
      len = Math.sqrt(u * u + v * v);
      u /= len;
      v /= len;
      gl.Normal3f(v, -u, 0.0);
      gl.Vertex3f(r2 * Math.cos(angle + da), r2 * Math.sin(angle + da),
		  width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + da), r2 * Math.sin(angle + da),
		  -width * 0.5);
      gl.Normal3f(Math.cos(angle), Math.sin(angle), 0.0);
      gl.Vertex3f(r2 * Math.cos(angle + 2 * da),
		  r2 * Math.sin(angle + 2 * da), width * 0.5);
      gl.Vertex3f(r2 * Math.cos(angle + 2 * da),
		  r2 * Math.sin(angle + 2 * da), -width * 0.5);
      u = r1 * Math.cos(angle + 3 * da) - r2 * Math.cos(angle + 2 * da);
      v = r1 * Math.sin(angle + 3 * da) - r2 * Math.sin(angle + 2 * da);
      gl.Normal3f(v, -u, 0.0);
      gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		  r1 * Math.sin(angle + 3 * da), width * 0.5);
      gl.Vertex3f(r1 * Math.cos(angle + 3 * da),
		  r1 * Math.sin(angle + 3 * da), -width * 0.5);
      gl.Normal3f(Math.cos(angle), Math.sin(angle), 0.0);
    }

    gl.Vertex3f(r1 * Math.cos(0), r1 * Math.sin(0), width * 0.5);
    gl.Vertex3f(r1 * Math.cos(0), r1 * Math.sin(0), -width * 0.5);

    gl.End();

    gl.ShadeModel(GL_SMOOTH);

    /* draw inside radius cylinder */
    gl.Begin(GL_QUAD_STRIP);
    for (i = 0; i <= teeth; i++) {
      angle = i * 2.0 * Math.PI / teeth;
      gl.Normal3f(-Math.cos(angle), -Math.sin(angle), 0.0);
      gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		  -width * 0.5);
      gl.Vertex3f(r0 * Math.cos(angle), r0 * Math.sin(angle),
		  width * 0.5);
    }
    gl.End();
  }


  function draw()
  {
    gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gl.PushMatrix();
    gl.Rotatef(view_rotx, 1.0, 0.0, 0.0);
    gl.Rotatef(view_roty, 0.0, 1.0, 0.0);
    gl.Rotatef(view_rotz, 0.0, 0.0, 1.0);

    gl.PushMatrix();
    gl.Translatef(-3.0, -2.0, 0.0);
    gl.Rotatef(angle, 0.0, 0.0, 1.0);
    gl.CallList(gear1);
    gl.PopMatrix();

    gl.PushMatrix();
    gl.Translatef(3.1, -2.0, 0.0);
    gl.Rotatef(-2.0 * angle - 9.0, 0.0, 0.0, 1.0);
    gl.CallList(gear2);
    gl.PopMatrix();

    gl.PushMatrix();
    gl.Translatef(-3.1, 4.2, 0.0);
    gl.Rotatef(-2.0 * angle - 25.0, 0.0, 0.0, 1.0);
    gl.CallList(gear3);
    gl.PopMatrix();

    //    gl.PushMatrix();
    //    gl.Color3f(1.0, 0.0, 0.0);
    //    glutprint(0, 0, 0, 0, fps + "fps");
    //    gl.PopMatrix();

    gl.PopMatrix();
  }


  /* new window size or exposure */
  function reshape(width, height)
  {
    var h = height / width;

    gl.Viewport(0, 0, width, height);
    gl.MatrixMode(GL_PROJECTION);
    gl.LoadIdentity();
    gl.Frustum(-1.0, 1.0, -h, h, 5.0, 60.0);
    gl.MatrixMode(GL_MODELVIEW);
    gl.LoadIdentity();
    gl.Translatef(0.0, 0.0, -40.0);
  }

  var viewport=Video.getViewport();
  reshape(viewport[2],viewport[3]);

  var pos =[5.0, 5.0, 10.0, 0.0];
  var red =[0.8, 0.1, 0.0, 1.0];
  var green =[0.0, 0.8, 0.2, 1.0];
  var blue =[0.2, 0.2, 1.0, 1.0];

  gl.ClearColor(0.0, 0.0, 0.2, 1.0);
  gl.ShadeModel(GL_SMOOTH);

  gl.Lightfv(GL_LIGHT0, GL_POSITION, pos);
  gl.Enable(GL_CULL_FACE);
  gl.Enable(GL_LIGHTING);
  gl.Enable(GL_LIGHT0);
  gl.Enable(GL_DEPTH_TEST);
    
  /* make the gears */
  var gear1 = gl.GenLists(1);
  gl.NewList(gear1, GL_COMPILE);
  gl.Materialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
  gear(1.0, 4.0, 1.0, 20, 0.7);
  gl.EndList();

  var gear2 = gl.GenLists(1);
  gl.NewList(gear2, GL_COMPILE);
  gl.Materialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green);
  gear(0.5, 2.0, 2.0, 10, 0.7);
  gl.EndList();

  var gear3 = gl.GenLists(1);
  gl.NewList(gear3, GL_COMPILE);
  gl.Materialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, blue);
  gear(1.3, 2.0, 0.5, 10, 0.7);
  gl.EndList();
  
  gl.Enable(GL_NORMALIZE);

  EGachine.step(function(){
		  var d = new Date();
		  time = d.getTime();
		  
		  angle += 2;
		  draw();
		  
		  if (joypad[0]) {
		    view_rotx+=joypad[0].y;
		    view_roty+=joypad[0].x;
		  }
		  
		  Video.swapBuffers();
		  
		  /* calc framerate */
		  if (t0 < 0)
		    t0 = time;
		  frames++;
		  if (time - t0 >= 5000.0) {
		    seconds = time - t0;
		    fps = frames / seconds * 1000;
		    stdout.write(frames + " frames in " + seconds / 1000 + " FPS " + fps +
				 "\n");
		    t0 = time;
		    frames = 0;
		  }
		});
};
init();
