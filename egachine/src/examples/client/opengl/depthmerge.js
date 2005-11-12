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

function drawScene() {
  Video.pushMatrix();
  gl.Translatef(0,0,0.9);
  gl.Color3f(0,1,0);
  gl.Rectf(0.2,0.2,3.8,1);
  Video.popMatrix();

  Video.pushMatrix();
  gl.Translatef(0,0,0.5);
  gl.Rotatef(-10,0,0,1);
  gl.Color3f(1,0,0);
  gl.Rectf(1,1,2,2);
  Video.popMatrix();

  Video.pushMatrix();
  gl.Rotatef(45,0,0,1);
  gl.Color3f(0,0,1);
  gl.Rectf(-2,-2,4,4);
  Video.popMatrix();
}

function init() {
  var gl=ejs.ModuleLoader.get("gl");
  var stdout=ejs.ModuleLoader.get("Stream").stdout;
  var viewport=Video.getViewport();
  var textures;
  var textureSize=1;
  var error;

  while ((textureSize<viewport[2])||(textureSize<viewport[3]))
    textureSize<<=1;

  gl.MatrixMode(GL_PROJECTION);		
  gl.LoadIdentity();
  gl.Ortho(0,4,0,3,-1,1);
  gl.MatrixMode(GL_MODELVIEW);

  gl.Enable(GL_DEPTH_TEST);
  gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // create textures to store pixel and depth data
  textures=gl.GenTextures(2);
  colorTexture=textures[0];
  depthTexture=textures[1];

  gl.Enable(GL_TEXTURE_2D);
  gl.Color3f(1.0,1.0,1.0);
  gl.BindTexture(GL_TEXTURE_2D, colorTexture);
  gl.TexImage2D(GL_TEXTURE_2D, 0, GL_RGB, textureSize, textureSize, 0, GL_RGB, GL_UNSIGNED_SHORT, null);
  gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  gl.BindTexture(GL_TEXTURE_2D, depthTexture);
  gl.TexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, textureSize, textureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, null);
  gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  gl.TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  // draw some scene with depth
  gl.Disable(GL_TEXTURE_2D);
  gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  drawScene();

  gl.Enable(GL_TEXTURE_2D);
  gl.BindTexture(GL_TEXTURE_2D, colorTexture);
  gl.CopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, textureSize, textureSize);
  gl.BindTexture(GL_TEXTURE_2D, depthTexture);
  gl.CopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, textureSize, textureSize);
  //  gl.BindTexture(GL_TEXTURE_2D, colorTexture);
  if ( (error=gl.GetError()) != GL_NO_ERROR)
    throw new Error("OpenGL error: "+error);

  // register function to be called for each step in time
  EGachine.step(function(dt){
		  var w,h;
		  gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		  if (1) {
		    w=viewport[2]/textureSize;
		    h=viewport[3]/textureSize;
		    gl.Color3f(1.0,1.0,1.0);
		    gl.Enable(GL_TEXTURE_2D);
		    //	    gl.BindTexture(GL_TEXTURE_2D, colorTexture);
		    gl.Begin(GL_QUADS);
		    gl.TexCoord2f(0,0);
		    gl.Vertex2f(0,0);
		    gl.TexCoord2f(w,0);
		    gl.Vertex2f(4,0);
		    gl.TexCoord2f(w,h);
		    gl.Vertex2f(4,3);
		    gl.TexCoord2f(0,h);
		    gl.Vertex2f(0,3);
		    gl.End();
		    if ( (error=gl.GetError()) != GL_NO_ERROR)
		      throw new Error("OpenGL error: "+error);
		  }else
		    drawScene();
		  Video.swapBuffers();
		});
};
init();
