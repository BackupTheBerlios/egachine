Video.swapBuffers();
gl.ClearColor(1,1,1,0);
Video.clear();
while (true)
{
  Input.poll();
  Video.swapBuffers();
}
