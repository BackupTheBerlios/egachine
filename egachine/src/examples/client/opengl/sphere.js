if (!EGachine.client) throw "This file must be run by egachine";
EGachine.checkVersion("0.1.2");
if (!this.gl) throw "This application needs OpenGL";

function init() {
  var viewport=Video.getViewport();
  var width=viewport[2];
  var height=viewport[3];
  var zrot=0,xrot=0;
  var colors=[[0.4, 0.6, 0.5, 1.0],
	      [1.0, 0.0, 0.0, 1.0],
	      [0.0, 1.0, 0.0, 1.0],
	      [0.8, 0.5, 0.0, 1.0],
	      [0.6, 0.4, 0.0, 1.0],
	      [0.0, 0.5, 0.0, 1.0],
	      [0.0, 0.5, 1.0, 0.5]];

  function compile(f) {
    var l,r;
    l = gl.GenLists(1);
    gl.NewList(l, GL_COMPILE);
    f();
    gl.EndList();
    r=function(){gl.CallList(l);};
    r.free=function(){gl.DeleteLists(l,1);};
    return r;
  }

  function rad(deg){
    return Math.PI*deg/180;
  }

  function pointOnCircle(alpha){
    var scale=1;
    return [Math.cos(alpha)*scale,Math.sin(alpha)*scale];
  }

  function pointOnSphere(alpha,beta){
    var a=pointOnCircle(alpha);
    var b=pointOnCircle(beta);
    return [a[0]*b[1],a[1]*b[1],b[0]];
  }

  function drawSphere() {
    var alphaGrid=24;
    var betaGrid=18;
    var alpha,beta;
    for (alpha=0;alpha<360;alpha+=360/alphaGrid) {
      gl.Begin(GL_LINE_STRIP);
      gl.Color4fv(colors[alpha ? 0 : 1]);
      for (beta=0;beta<=180;beta+=180/betaGrid) {
	gl.Vertex3fv(pointOnSphere(rad(alpha),rad(beta)));
      }
      gl.End();
    }
    for (beta=180/betaGrid;beta<180;beta+=180/betaGrid) {
      gl.Begin(GL_LINE_STRIP);
      for (alpha=0;alpha<=360;alpha+=360/alphaGrid) {
	gl.Vertex3fv(pointOnSphere(rad(alpha),rad(beta)));
      }
      gl.End();
    }
  }

  function drawTetrahedron() {
    var p=[[0,180],[0,60],[120,60],[240,60]];
    var i,j;
    for (i=0;i<p.length;++i) {
      for (j=0;j<p[i].length;++j)
	p[i][j]=rad(p[i][j]);
    }

    gl.Begin(GL_TRIANGLES);
    gl.Color4fv(colors[2]);
    for (i=1;i<4;++i) {
      gl.Vertex3fv(pointOnSphere(p[i][0],p[i][1]));
    }
    for (i=0;i<3;++i) {
      gl.Color4fv(colors[i+3]);
      gl.Vertex3fv(pointOnSphere(p[0][0],p[0][1]));
      for (j=0;j<2;++j) {
	k=j+i+1;
	if (k>=p.length) k=1; // not 0!
	gl.Vertex3fv(pointOnSphere(p[k][0],p[k][1]));
      }
    }
    gl.End();
  }
  
  function setupMatrix() {
    gl.LoadIdentity();
    gl.Translatef(0,0,-3);
    gl.Rotatef(xrot,1.0,0.0,0.0);
    gl.Rotatef(zrot,0.0,0.0,1.0);
  }

  function drawScene() {
    gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
    setupMatrix();
    draw();
  }

  gl.ShadeModel(GL_FLAT);
  gl.DepthFunc(GL_LESS);
  gl.Enable(GL_DEPTH_TEST);
  gl.Disable(GL_LINE_SMOOTH);
  gl.LineWidth(1);
  gl.MatrixMode(GL_PROJECTION);
  gl.LoadIdentity();
  glu.Perspective(45.0,width/height,0.1,200.0);
  gl.MatrixMode(GL_MODELVIEW);

  Input.addEventListener("mousemove",function(evt){
			   zrot=-360*evt.screenX/(width-1);
			   xrot=180*evt.screenY/(height-1);
			 }
			 ,false);
  Video.showMouseCursor(1);

  var draw=compile(function(){
		     drawTetrahedron();
		     drawSphere();
		     gl.DepthMask(false);
		     Video.setColor4v(colors[colors.length-1]);
		     Video.drawRectangle(2,2);
		     gl.DepthMask(true);
		   });

  EGachine.step(function(dt){
		  drawScene();
		  Video.swapBuffers();
		});
}

init();
