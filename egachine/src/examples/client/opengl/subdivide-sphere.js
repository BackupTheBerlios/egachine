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
	      [0.8, 0.7, 0.0, 1.0],
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

  function drawSphereGrid() {
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

  function isVec(a) {
    return a instanceof Array;
  }

  function assert(x){if (!x()) throw new Error(x.toSource());};

  function assertIsVec(a) {
    assert(function(){return isVec(a);});
  }

  //! check that a and b are vectors of the same dimension
  function assertVecDim(a,b) {
    assertIsVec(a);
    assertIsVec(b);
    assert(function(){return a.length==b.length;});
  }

  function bind2nd(f,a) {
    return function(x){return f(x,a);};
  }

  //! call a function for each array element
  function foreach(f,a) {
    assertIsVec(a);
    var i;
    for (i=0;i<a.length;++i) f(a[i]);
  }

  //! call a function for each array element (and return array with results)
  function map1(f,a) {
    assertIsVec(a);
    var i,r=[];
    for (i=0;i<a.length;++i)
      r[i]=f(a[i]);
    return r;
  }

  //! combine two vectors componentwise
  function map2(f,a,b) {
    assertVecDim(a,b);
    var i,r=[];
    for (i=0;i<a.length;++i)
      r[i]=f(a[i],b[i]);
    return r;
  }

  //! n-ary function from binary function
  function apply(f,a) {
    var i,r;
    assert(function(){return a.length>=2;});
    r=f(a[0],a[1]);
    for (i=2;i<a.length;++i)
      r=f(r,a[i]);
    return r;
  }

  function sum(a,b) {
    return a+b;
  }

  function mul(a,b) {
    return a*b;
  }

  function vecSum(a,b) {
    return map2(sum,a,b);
  }

  function dot(a,b) {
    return apply(sum,map2(mul,a,b));
  }

  function norm2(a) {
    return Math.sqrt(dot(a,a));
  }

  function normalize(a) {
    return map1(bind2nd(mul,1/norm2(a)),a);
  }

  //! return triangle mesh representing tetrahedron
  function tetrahedron() {
    var p=[[0,180],[0,60],[120,60],[240,60]];
    var i,j;
    var r=[];
    for (i=0;i<p.length;++i) {
      for (j=0;j<p[i].length;++j)
	p[i][j]=rad(p[i][j]);
    }

    r[0]=[];
    for (i=1;i<4;++i)
      r[0].push(pointOnSphere(p[i][0],p[i][1]));
    for (i=0;i<3;++i) {
      r[i+1]=[];
      r[i+1].push(pointOnSphere(p[0][0],p[0][1]));
      for (j=0;j<2;++j) {
	k=j+i+1;
	if (k>=p.length) k=1; // not 0!
	r[i+1].push(pointOnSphere(p[k][0],p[k][1]));
      }
    }
    return r;
  }

  function drawMesh(m) {
    var i,j;
    gl.Begin(GL_TRIANGLES);
    for (i=0;i<m.length;++i) {
      gl.Color4fv(colors[(i%5)+2]);
      for (j=0;j<m[i].length;++j)
	gl.Vertex3fv(m[i][j]);
    }
    gl.End();
  }

  function subdivide(mesh) {
    var newMesh=[];

    function subdivideTriangle(tp) {
      assertIsVec(tp);
      assert(function(){return tp.length==3;});
      var i,j,np=[];
      for (i=0;i<3;++i) {
	j=(i+1)%3;
	np[i]=normalize(vecSum(tp[i],tp[j]));
      }
      return [[tp[0],np[0],np[2]],
	      [tp[1],np[1],np[0]],
	      [tp[2],np[2],np[1]],
	      [np[0],np[1],np[2]]];
    };
    
    myforeach(function(t){
		var sub=subdivideTriangle(t);
		assertIsVec(sub);
		assert(function(){return sub.length==4;});
		myforeach(function(st){
			    newMesh.push(st);
			  },sub);
	      },mesh);
    return newMesh;
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

  var sphere={
    mesh:tetrahedron(),
    subdivide:function(){this.mesh=subdivide(this.mesh);},
    draw:function(){drawMesh(this.mesh);}
  };

  function drawScene() {
    sphere.draw();
    drawSphereGrid();
    gl.DepthMask(false);
    Video.setColor4v(colors[colors.length-1]);
    Video.drawRectangle(2,2);
    gl.DepthMask(true);
  }

  var draw=compile(drawScene);

  Input.addEventListener("mousemove",function(evt){
			   zrot=-360*evt.screenX/(width-1);
			   xrot=180*evt.screenY/(height-1);
			 }
			 ,false);
  Input.addEventListener("mousedown",function(evt){
			   sphere.subdivide();
			   draw.free();
			   draw=compile(drawScene);
			 }
			 ,false);
  Video.showMouseCursor(1);

  EGachine.step(function(dt){
		  gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		
		  gl.LoadIdentity();
		  gl.Translatef(0,0,-3);
		  gl.Rotatef(xrot,1.0,0.0,0.0);
		  gl.Rotatef(zrot,0.0,0.0,1.0);
		  draw();
		  Video.swapBuffers();
		});
}

init();
