/*
  This code was created by Jeff Molofee '99 (ported to Solaris/GLUT by Lakmal Gunasekara '99)

  If you've found this code useful, please let me know.

  Visit me at www.demonews.com/hosted/nehe
  (email Richard Campbell at ulmont@bellsouth.net)
  (email Lakmal Gunasekara at lakmal@gunasekara.de)

  Ported to Python by Ryan Showalter '04
  tankcoder@warpmail.net
  
  This is pretty much an exact port from the C code,
  please use this as refence and not as an example
  of how to program well (or at all) in OO.


  "Ported" to EGachine/Javascript by Jens Thiele '04
  (stripped down version)
*/

if (!EGachine.client) throw "This file must be run by egachine";
if (!EGachine.checkVersion(0,0,5)) throw "at least version 0.0.5 required";
if (!this.gl) throw "This game needs OpenGL";

// lighting on/off (1 = on, 0 = off) 
light = 1;

xrot = 0;   // x rotation
yrot = 0;  // y rotation
xspeed = 0.1; // x rotation speed
yspeed = 0.1; // y rotation speed

z = -5.0;  // depth into the screen.

// white ambient light at half intensity (rgba)
LightAmbient = [ 0.5, 0.5, 0.5, 1.0 ];

// super bright, full intensity diffuse light.
LightDiffuse = [ 1.0, 1.0, 1.0, 1.0 ];

// position of light (x, y, z, (position of light))
LightPosition = [ 0.0, 0.0, 2.0, 1.0 ];

blend = 1;                  // Turn blending on/off


EGachine.addResource("glass",'\
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\
/9k=\
');
    

var viewport=Video.getViewport();
Width=viewport.sx;
Height=viewport.sy;

gl.Enable(GL_TEXTURE_2D);                     // Enable texture mapping.
 
gl.ClearColor(0.0, 0.0, 0.0, 0.0);    	// This Will Clear The Background Color To Black
gl.ClearDepth(1.0);				// Enables Clearing Of The Depth Buffer
gl.DepthFunc(GL_LESS);			// The Type Of Depth Test To Do
gl.Enable(GL_DEPTH_TEST);			// Enables Depth Testing
gl.ShadeModel(GL_SMOOTH);			// Enables Smooth Color Shading

gl.MatrixMode(GL_PROJECTION);
gl.LoadIdentity();				// Reset The Projection Matrix

glu.Perspective(45.0,Width/Height,0.1,100.0);	// Calculate The Aspect Ratio Of The Window

gl.MatrixMode(GL_MODELVIEW);

// set up light number 1.
gl.Lightfv(GL_LIGHT1, GL_AMBIENT, LightAmbient);  // add lighting. (ambient)
gl.Lightfv(GL_LIGHT1, GL_DIFFUSE, LightDiffuse);  // add lighting. (diffuse).
gl.Lightfv(GL_LIGHT1, GL_POSITION,LightPosition); // set light position.
gl.Enable(GL_LIGHT1);                             // turn light 1 on.

gl.Enable(GL_LIGHTING);

// setup blending
gl.BlendFunc(GL_SRC_ALPHA,GL_ONE);			// Set The Blending Function For Translucency
gl.Color4f(1.0, 1.0, 1.0, 0.5);

texture=Video.getTextureID("glass"); // creates texture from resource


gl.Enable(GL_BLEND);		    // Turn Blending On
gl.Disable(GL_DEPTH_TEST);         // Turn Depth Testing Off

// The main drawing function.
function drawGLScene(){
  gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);		// Clear The Screen And The Depth Buffer
  gl.LoadIdentity();				// Reset The View

  gl.Translatef(0.0,0.0,z);                     // move z units out from the screen.

  gl.Rotatef(xrot,1.0,0.0,0.0);			// Rotate On The X Axis
  gl.Rotatef(yrot,0.0,1.0,0.0);			// Rotate On The Y Axis

  print("bind texture: "+texture);
  gl.BindTexture(GL_TEXTURE_2D, texture);   // choose the texture to use.

  gl.Begin(GL_QUADS);		                // begin drawing a cube

  // Front Face (note that the texture's corners have to match the quad's corners)
  gl.Normal3f( 0.0, 0.0, 1.0);                              // front face points out of the screen on z.
    
  gl.TexCoord2f(0.0, 0.0);
  gl.Vertex3f(-1.0, -1.0,  1.0);	// Bottom Left Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f( 1.0, -1.0,  1.0);	// Bottom Right Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f( 1.0,  1.0,  1.0);	// Top Right Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f(-1.0,  1.0,  1.0);	// Top Left Of The Texture and Quad

  // Back Face
  gl.Normal3f( 0.0, 0.0,-1.0);                              // back face points into the screen on z.
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f(-1.0, -1.0, -1.0);	// Bottom Right Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f(-1.0,  1.0, -1.0);	// Top Right Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f( 1.0,  1.0, -1.0);	// Top Left Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 0.0); 
  gl.Vertex3f( 1.0, -1.0, -1.0);	// Bottom Left Of The Texture and Quad

  // Top Face
  gl.Normal3f( 0.0, 1.0, 0.0);                              // top face points up on y.
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f(-1.0,  1.0, -1.0);	// Top Left Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 0.0); 
  gl.Vertex3f(-1.0,  1.0,  1.0);	// Bottom Left Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f( 1.0,  1.0,  1.0);	// Bottom Right Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f( 1.0,  1.0, -1.0);	// Top Right Of The Texture and Quad

  // Bottom Face
  gl.Normal3f( 0.0, -1.0, 0.0);                             // bottom face points down on y.
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f(-1.0, -1.0, -1.0);	// Top Right Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f( 1.0, -1.0, -1.0);	// Top Left Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 0.0); 
  gl.Vertex3f( 1.0, -1.0,  1.0);	// Bottom Left Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f(-1.0, -1.0,  1.0);	// Bottom Right Of The Texture and Quad

  // Right face
  gl.Normal3f( 1.0, 0.0, 0.0);                              // right face points right on x.
    
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f( 1.0, -1.0, -1.0);	// Bottom Right Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f( 1.0,  1.0, -1.0);	// Top Right Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f( 1.0,  1.0,  1.0);	// Top Left Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 0.0); 
  gl.Vertex3f( 1.0, -1.0,  1.0);	// Bottom Left Of The Texture and Quad

  // Left Face
  gl.Normal3f(-1.0, 0.0, 0.0);                              // left face points left on x.
    
  gl.TexCoord2f(0.0, 0.0); 
  gl.Vertex3f(-1.0, -1.0, -1.0);	// Bottom Left Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 0.0); 
  gl.Vertex3f(-1.0, -1.0,  1.0);	// Bottom Right Of The Texture and Quad
    
  gl.TexCoord2f(1.0, 1.0); 
  gl.Vertex3f(-1.0,  1.0,  1.0);	// Top Right Of The Texture and Quad
    
  gl.TexCoord2f(0.0, 1.0); 
  gl.Vertex3f(-1.0,  1.0, -1.0);	// Top Left Of The Texture and Quad

  gl.End();                                    // done with the polygon.

  xrot+=xspeed;		               // X Axis Rotation
  yrot+=yspeed;		               //  Y Axis Rotation

  // since this is double buffered, swap the buffers to display what just got drawn.
  Video.swapBuffers();
}

while (true)
{
  Input.poll();
  drawGLScene();
}
