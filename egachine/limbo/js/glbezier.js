function println(x){stderr.write(x+"\n");};
sx=4/3;
sy=1;
Video.setViewportCoords({left:0,right:sx,bottom:0,top:sy});
gl.LineWidth(2);
gl.Enable(GL_LINE_SMOOTH);
gl.Enable(GL_BLEND);
if (gl.GetError()) throw gl.GetError();
var points=[0, 0,       0,
	    sx,sy*9/10, 0,
	    0, sy*9/10, 0,
	    sx/2,0,       0];
gl.Map1f(GL_MAP1_VERTEX_3,0,1,points);
if ((error=gl.GetError()) != GL_NO_ERROR) println("Error: "+glu.ErrorString(error));
gl.Enable(GL_MAP1_VERTEX_3);
precision=50;
gl.MapGrid1f(precision,0,1);

while (true)
{
  Input.poll();
  gl.Clear( GL_COLOR_BUFFER_BIT );
  gl.Color3f(0.5,0.5,0.5);
  gl.Begin(GL_LINE_STRIP);
  for (i=0;i<points.length;i+=3) {
    gl.Vertex3f(points[i],points[i+1],points[i+2]);
  }
  gl.End();
  gl.Color3f(1,1,1);
  gl.EvalMesh1(GL_LINE,0,precision);
  Video.swapBuffers();
  Timer.uSleep(10000);
}


