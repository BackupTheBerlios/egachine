// svg tiny supports all svg 1.1 paths except the elliptical arc curve command

// => we need: cubic bezier curves, quadratic bezier curves

function Bezier(controlpoints)
{
  this.controlpoints=controlpoints;
}

//! subdivide
Bezier.prototype.subdivide=function(precision)
{
  if (this.b1) {
    // assert(this.b2)
    this.b1.subdivide(precision);
    this.b2.subdivide(precision);
    return;
  }
  
  if (this.isFlat(precision))
    return;

  function middles(points) {
    var i;
    var l=points.length-1;
    var res=[];
    for (i=0;i<l;++i) {
      res[i]=V2D.middle(points[i],points[i+1]);
    }
    return res;
  }

  var i;
  var l=this.controlpoints.length;
  var pyramid=[];
  pyramid[0]=this.controlpoints;
  for (i=1;i<l;++i) {
    pyramid[i]=middles(pyramid[i-1]);
  }

  var left=[];
  for (i=0;i<l;++i)
    left[i]=pyramid[i][0];
  var right=[];
  for (i=0;i<l;++i)
    right[i]=pyramid[l-1-i][i];
  this.b1=new Bezier(left);
  this.b2=new Bezier(right);
  this.b1.subdivide(precision);
  this.b2.subdivide(precision);
}

//! check if the control polygon is nearly a straight line
Bezier.prototype.isFlat=function(precision)
{
  for (i=0;i<this.controlpoints.length-2;++i) {
    var p1=this.controlpoints[i];
    var p2=this.controlpoints[i+1];
    var p3=this.controlpoints[i+1];
    var l1=p2.sub(p1);
    var l2=p3.sub(p2);
    if (Math.abs(l1.x*l2.y-l1.y+l2.x)>precision)
      return false;
    /*
    // nearly colinear
    // if p1,p2,p3 are colinear then there is a t with:
    // p3 = p2+l1*t => t = l2.x/l1.x
    var tx = l2.x/l1.x;
    var ty = l2.y/l1.y;
    if ((tx<0)&&(ty<0)) return false; // wrong direction
    */
  }
  return true;
}

//! emit all controlpoints
Bezier.prototype.emitPoints=function(callback)
{
  if (this.b1) {
    // assert(this.b2);
    this.b1.emitPoints(callback);
    this.b2.emitPoints(callback);
  }else{
    var i;
    for (i in this.controlpoints) {
      callback(this.controlpoints[i]);
    }
  }
}

//! emit all line segments
Bezier.prototype.emitLines=function(callback)
{
  if (this.b1) {
    // assert(this.b2);
    this.b1.emitLines(callback);
    this.b2.emitLines(callback);
  }else{
    var i,l=this.controlpoints.length-1;
    for (i=0;i<l;++i) {
      callback(this.controlpoints[i],this.controlpoints[i+1]);
    }
  }
}

//! paint this curve
Bezier.prototype.paint=function(strokeWidth)
{
  gl.Begin(GL_LINE_STRIP);
  this.emitPoints((function(p){gl.Vertex2f(p.x,p.y);}));
  gl.End();
  return;

  gl.Begin(GL_LINES);
  this.emitLines(function(p1,p2){
		   function vertex(v) {
		     gl.Vertex2f(v.x,v.y);
		   }
		   vertex(p1);
		   vertex(p2);
		   m=V2D.middle(p1,p2);
		   ortho=p1.sub(p2).rot90().normalized().scale(strokeWidth/2);
		   vertex(m.add(ortho));
		   vertex(m.sub(ortho));
		 });
  /*
  gl.Begin(GL_QUADS);
  this.emitLines(function(p1,p2){
		   function vertex(v) {
		     gl.Vertex2f(v.x,v.y);
		   }
		   var ortho=p1.sub(p2).rot90().normalized().scale(strokeWidth/2);
		   vertex(p1.add(ortho));
		   vertex(p1.sub(ortho));
		   vertex(p2.sub(ortho));
		   vertex(p2.add(ortho));
		 });
  */
  gl.End();
};


function onCircle(f,radius)
{
  var i;
  for (i=0;i<360;i+=1){
    gl.PushMatrix();
    gl.Translated(Math.cos(i)*radius,Math.sin(i)*radius,0);
    f();
    gl.PopMatrix();
  }
}

sx=4/3;
sy=1;
Video.setViewportCoords({left:0,right:sx,bottom:0,top:sy});
//gl.Enable(GL_LINE_SMOOTH);
gl.LineWidth(1280/1000);
if (gl.GetError()) throw gl.GetError();
test=new Bezier([new V2D(sx/8,-sy),new V2D(sx,sy),new V2D(0,sy),new V2D(7/8*sx,-sy)]);
test.subdivide(4/3/50);
var dl=gl.GenLists(1);
gl.NewList(dl,GL_COMPILE);
test.paint(sx/20);
gl.EndList();
delete test;

var dlcircle=gl.GenLists(1);
gl.NewList(dlcircle,GL_COMPILE);
onCircle((function(){gl.CallList(dl);}),sx/10);
gl.CallList(dl);
gl.CallList(dl);
gl.EndList();


var d=20;
gl.Translated(4/3/2,1/2,0);
gl.Scaled(0.5,0.5,1);
gl.Color4f(1,1,1,0.3);
gl.Enable(GL_BLEND);
while (true)
{
  Input.poll();
  gl.Clear( GL_COLOR_BUFFER_BIT );
  gl.CallList(dlcircle);
  gl.Rotated(1,0,0,1);
  Video.swapBuffers();
  Timer.uSleep(10000);
}



