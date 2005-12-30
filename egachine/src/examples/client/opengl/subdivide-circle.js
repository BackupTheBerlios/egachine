if (!EGachine.client) throw "This file must be run by egachine";
EGachine.checkVersion("0.1.2");
if (!this.gl) throw "This application needs OpenGL";

function print(x) {
  ejs.ModuleLoader.get("Stream").stdout.write(""+x+"\n");
}

function init() {
  var viewport=Video.getViewport();
  var width=viewport[2];
  var height=viewport[3];
  var zrot=0,xrot=0;
  var colors=[[1,0.3,0,1],[0,1,0.3,1],[0,0.3,1,1]];

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

  function pointOnCircle(alpha){
    var scale=1;
    //    var scale=1+0.1*Math.abs(Math.sin(alpha*20));
    return [Math.cos(alpha)*scale,Math.sin(alpha)*scale];
  }
  
  //! subdivide angle
  function subdivide(a,b) {
    var beta=b;
    while(beta<a) beta+=Math.PI*2;
    return (a+beta)/2;
  }

  function Triangle(a,b,c) {
    var i,alpha,ret={
      // angles
      p:[a,b,c],
      // children
      c:[],
      screenArea:function(){
	function cpOnScreen(alpha) {
	  var pos=pointOnCircle(alpha);
	  return glu.Project(pos[0],pos[1],0,
			     gl.GetFloatv(GL_MODELVIEW_MATRIX),
			     gl.GetFloatv(GL_PROJECTION_MATRIX),
			     gl.GetIntegerv(GL_VIEWPORT));
	}
	var a=cpOnScreen(this.p[0]);
	var b=cpOnScreen(this.p[1]);
	var c=cpOnScreen(this.p[2]);
	print("a:"+a.toSource()+" b:"+b.toSource()+" c:"+c.toSource());
	a=new sg.V2D(a[0],a[1]);
	b=new sg.V2D(b[0],b[1]);
	c=new sg.V2D(c[0],c[1]);
	b.dec(a);
	c.dec(a);
	var ret=b.x*c.y-b.y*c.x;
	print(ret);
	return ret;
      },
      draw:function(){
	var i;
	if (0) {
	  for (i=0;i<this.p.length;++i) {
	    print(glu.Project(pointOnCircle(this.p[i])[0],pointOnCircle(this.p[i])[1],0,
			      gl.GetFloatv(GL_MODELVIEW_MATRIX),
			      gl.GetFloatv(GL_PROJECTION_MATRIX),
			      gl.GetIntegerv(GL_VIEWPORT)).toSource());
	  }
	}
	for (i=0;i<this.c.length;++i)
	  this.c[i].draw();
	//if (this.c.length) return;
	gl.Begin(GL_TRIANGLES);
	for (i=0;i<this.p.length;++i) {
	  Video.setColor4v(colors[i%3]);
	  gl.Vertex2fv(pointOnCircle(this.p[i]));
	}
	gl.End();
      },
      subdivide:function(){
	var i,ret=false;
	if (this.c.length) {
	  for (i=0;i<this.c.length;++i)
	    ret|=this.c[i].subdivide();
	  return ret;
	}
	if (Math.abs(this.screenArea())<1) return false;
	this._subdivide();
	return true;
      },
      _subdivide:function(){
	var i;
	for (i=0;i<2;++i)
	  this.c.push(Triangle(this.p[i],subdivide(this.p[i],this.p[i+1]),this.p[i+1]));
      }
    };
    return ret;
  }

  function baseTriangle() {
    var i,alpha,ret,p=[];
    for (i=0;i<3;++i) {
      p[i]=i*Math.PI*2/3;
    }
    ret=Triangle(p[0],p[1],p[2]);
    ret._subdivide=function(){
      var i;
      for (i=0;i<3;++i)
	this.c.push(Triangle(p[i],subdivide(p[i],p[(i+1)%3]),p[(i+1)%3]));
    }
    return ret;
  }

  Input.handleInput=function(i){
    if ((i.dev!=undefined)&&(i.dev>=0)&&(i.dev<2)) {
      base.subdivide();
      draw.free();
      draw=compile(function(){base.draw();});
    }
  }

  function setupMatrix(zrot) {
    gl.LoadIdentity();
    gl.Translatef(0,0,-2);
    gl.Rotatef(xrot-80,1.0,0.0,0.0);
    gl.Rotatef(zrot,0.0,0.0,1.0);
  }

  function drawScene(zrot) {
    Video.clear();
    setupMatrix(zrot);
    //    print(base.screenArea());
    draw();
    Video.setColor4v([1,1,1,0.3]);
    Video.drawRectangle(2,2);
  }

  gl.ShadeModel(GL_SMOOTH);
  gl.MatrixMode(GL_PROJECTION);
  gl.LoadIdentity();
  glu.Perspective(45.0,width/height,0.1,200.0);
  gl.MatrixMode(GL_MODELVIEW);

  var base=baseTriangle();
  /*
  setupMatrix(0);
  while(base.subdivide())
    ;
  */

  var draw=compile(function(){base.draw();});

  EGachine.step(function(dt){
		  zrot+=dt*45;
		  xrot+=dt*5;
		  drawScene(zrot);
		  Video.swapBuffers();
		});
}

init();
