// svg tiny supports all svg 1.1 paths except the elliptical arc curve command

// => we need: cubic bezier curves, quadratic bezier curves

function Bezier(controlpoints)
{
  this.controlpoints=controlpoints;
}

//! subdivide
Bezier.prototype.subdivide=function()
{
  if (this.b1) {
    // assert(this.b2)
    this.b1.subdivide();
    this.b2.subdivide();
    return;
  }

  function middle(p1,p2) {
    return new V2D((p1.x+p2.x)*0.5,(p1.y+p2.y)*0.5);
  }

  function middles(points) {
    var i;
    var l=points.length-1;
    var res=[];
    for (i=0;i<l;++i) {
      res[i]=middle(points[i],points[i+1]);
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
}

//! check if the control polygon is nearly a straight line
Bezier.prototype.isFlat=function(precision)
{
}

//! paint this curve
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

//! paint this curve
Bezier.prototype.paint=function(precision)
{
  gl.Begin(GL_LINE_STRIP);
  this.emitPoints((function(p){gl.Vertex2f(p.x,p.y);}));
  gl.End();
};


sx=4/3;
sy=1;
Video.setViewportCoords({left:0,right:sx,bottom:0,top:sy});
gl.LineWidth(5);
gl.Enable(GL_LINE_SMOOTH);
test=new Bezier([new V2D(0,0),new V2D(sx/2,0),new V2D(sx,sy),new V2D(sx/4,3/4*sy),new V2D(sx/4,1/4*sy), new V2D(sx,0)]);
while (true)
{
  Input.poll();
  gl.Clear( GL_COLOR_BUFFER_BIT );
  test.paint();
  test.subdivide();
  Video.swapBuffers();
  Timer.uSleep(1000000);
}



