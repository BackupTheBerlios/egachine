/*
 * This code was created by Jeff Molofee '99 
 * (ported to Linux/SDL by Ti Leggett '01)
 * (ported to EGachine/Javascript by Jens Thiele, 2004)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 * 
 */

if (!EGachine.client) throw new Error("This file must be run by egachine");
if (!EGachine.checkVersion(0,0,6)) throw new Error("at least version 0.0.6 required");
if (!this.gl) throw new Error("This game needs OpenGL");

/* function to reset our viewport after a window resize */
function resizeWindow(width, height)
{
  if ( height == 0 ) height = 1;

  ratio = width / height;

  /* Setup our viewport. */
  gl.Viewport( 0, 0, width, height );

  /* change to the projection matrix and set our viewing volume. */
  gl.MatrixMode( GL_PROJECTION );
  gl.LoadIdentity( );

  /* Set our perspective */
  glu.Perspective( 45.0, ratio, 0.1, 100.0 );

  /* Make sure we're chaning the model view and not the projection */
  gl.MatrixMode( GL_MODELVIEW );

  /* Reset The View */
  gl.LoadIdentity( );

  return true;
}

/* Here goes our drawing code */
function drawGLScene()
{
  /* Clear The Screen And The Depth Buffer */
  gl.Clear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  /* Move Left 1.5 Units And Into The Screen 6.0 */
  gl.LoadIdentity();
  gl.Translatef( -1.5, 0.0, -6.0 );

  /* Rotate The Triangle On The Y axis ( NEW ) */
  gl.Rotatef( rtri, 0.0, 1.0, 0.0 );

  gl.Begin( GL_TRIANGLES );             /* Drawing Using Triangles       */
  gl.Color3f(   1.0,  0.0,  0.0 ); /* Red                           */
  gl.Vertex3f(  0.0,  1.0,  0.0 ); /* Top Of Triangle (Front)       */
  gl.Color3f(   0.0,  1.0,  0.0 ); /* Green                         */
  gl.Vertex3f( -1.0, -1.0,  1.0 ); /* Left Of Triangle (Front)      */
  gl.Color3f(   0.0,  0.0,  1.0 ); /* Blue                          */
  gl.Vertex3f(  1.0, -1.0,  1.0 ); /* Right Of Triangle (Front)     */

  gl.Color3f(   1.0,  0.0,  0.0 ); /* Red                           */
  gl.Vertex3f(  0.0,  1.0,  0.0 ); /* Top Of Triangle (Right)       */
  gl.Color3f(   0.0,  0.0,  1.0 ); /* Blue                          */
  gl.Vertex3f(  1.0, -1.0,  1.0 ); /* Left Of Triangle (Right)      */
  gl.Color3f(   0.0,  1.0,  0.0 ); /* Green                         */
  gl.Vertex3f(  1.0, -1.0, -1.0 ); /* Right Of Triangle (Right)     */

  gl.Color3f(   1.0,  0.0,  0.0 ); /* Red                           */
  gl.Vertex3f(  0.0,  1.0,  0.0 ); /* Top Of Triangle (Back)        */
  gl.Color3f(   0.0,  1.0,  0.0 ); /* Green                         */
  gl.Vertex3f(  1.0, -1.0, -1.0 ); /* Left Of Triangle (Back)       */
  gl.Color3f(   0.0,  0.0,  1.0 ); /* Blue                          */
  gl.Vertex3f( -1.0, -1.0, -1.0 ); /* Right Of Triangle (Back)      */

  gl.Color3f(   1.0,  0.0,  0.0 ); /* Red                           */
  gl.Vertex3f(  0.0,  1.0,  0.0 ); /* Top Of Triangle (Left)        */
  gl.Color3f(   0.0,  0.0,  1.0 ); /* Blue                          */
  gl.Vertex3f( -1.0, -1.0, -1.0 ); /* Left Of Triangle (Left)       */
  gl.Color3f(   0.0,  1.0,  0.0 ); /* Green                         */
  gl.Vertex3f( -1.0, -1.0,  1.0 ); /* Right Of Triangle (Left)      */
  gl.End( );                            /* Finished Drawing The Triangle */

  /* Move Right 3 Units */
  gl.LoadIdentity( );
  gl.Translatef( 1.5, 0.0, -6.0 );

  /* Rotate The Quad On The X axis ( NEW ) */
  gl.Rotatef( rquad, 1.0, 0.0, 0.0 );

  /* Set The Color To Blue One Time Only */
  gl.Color3f( 0.5, 0.5, 1.0);

  gl.Begin( GL_QUADS );                 /* Draw A Quad                      */
  gl.Color3f(   0.0,  1.0,  0.0 ); /* Set The Color To Green           */
  gl.Vertex3f(  1.0,  1.0, -1.0 ); /* Top Right Of The Quad (Top)      */
  gl.Vertex3f( -1.0,  1.0, -1.0 ); /* Top Left Of The Quad (Top)       */
  gl.Vertex3f( -1.0,  1.0,  1.0 ); /* Bottom Left Of The Quad (Top)    */
  gl.Vertex3f(  1.0,  1.0,  1.0 ); /* Bottom Right Of The Quad (Top)   */

  gl.Color3f(   1.0,  0.5,  0.0 ); /* Set The Color To Orange          */
  gl.Vertex3f(  1.0, -1.0,  1.0 ); /* Top Right Of The Quad (Botm)     */
  gl.Vertex3f( -1.0, -1.0,  1.0 ); /* Top Left Of The Quad (Botm)      */
  gl.Vertex3f( -1.0, -1.0, -1.0 ); /* Bottom Left Of The Quad (Botm)   */
  gl.Vertex3f(  1.0, -1.0, -1.0 ); /* Bottom Right Of The Quad (Botm)  */

  gl.Color3f(   1.0,  0.0,  0.0 ); /* Set The Color To Red             */
  gl.Vertex3f(  1.0,  1.0,  1.0 ); /* Top Right Of The Quad (Front)    */
  gl.Vertex3f( -1.0,  1.0,  1.0 ); /* Top Left Of The Quad (Front)     */
  gl.Vertex3f( -1.0, -1.0,  1.0 ); /* Bottom Left Of The Quad (Front)  */
  gl.Vertex3f(  1.0, -1.0,  1.0 ); /* Bottom Right Of The Quad (Front) */

  gl.Color3f(   1.0,  1.0,  0.0 ); /* Set The Color To Yellow          */
  gl.Vertex3f(  1.0, -1.0, -1.0 ); /* Bottom Left Of The Quad (Back)   */
  gl.Vertex3f( -1.0, -1.0, -1.0 ); /* Bottom Right Of The Quad (Back)  */
  gl.Vertex3f( -1.0,  1.0, -1.0 ); /* Top Right Of The Quad (Back)     */
  gl.Vertex3f(  1.0,  1.0, -1.0 ); /* Top Left Of The Quad (Back)      */

  gl.Color3f(   0.0,  0.0,  1.0 ); /* Set The Color To Blue            */
  gl.Vertex3f( -1.0,  1.0,  1.0 ); /* Top Right Of The Quad (Left)     */
  gl.Vertex3f( -1.0,  1.0, -1.0 ); /* Top Left Of The Quad (Left)      */
  gl.Vertex3f( -1.0, -1.0, -1.0 ); /* Bottom Left Of The Quad (Left)   */
  gl.Vertex3f( -1.0, -1.0,  1.0 ); /* Bottom Right Of The Quad (Left)  */

  gl.Color3f(   1.0,  0.0,  1.0 ); /* Set The Color To Violet          */
  gl.Vertex3f(  1.0,  1.0, -1.0 ); /* Top Right Of The Quad (Right)    */
  gl.Vertex3f(  1.0,  1.0,  1.0 ); /* Top Left Of The Quad (Right)     */
  gl.Vertex3f(  1.0, -1.0,  1.0 ); /* Bottom Left Of The Quad (Right)  */
  gl.Vertex3f(  1.0, -1.0, -1.0 ); /* Bottom Right Of The Quad (Right) */
  gl.End( );                            /* Done Drawing The Quad            */

  /* Draw it to the screen */
  Video.swapBuffers();

  /* Gather our frames per second */
  Frames++;

  t = Timer.getTimeStamp();
  if (t - t0 >= 1000000.0) {
    seconds = (t - t0) / 1000000.0;
    fps = Frames / seconds;
    Stream.stdout.write(Frames+" frames in "+seconds+" seconds = "+fps+" FPS\n");
    t0 = t;
    Frames = 0;
  }

  /* Increase The Rotation Variable For The Triangle ( NEW ) */
  rtri  += 0.2;
  /* Decrease The Rotation Variable For The Quad     ( NEW ) */
  rquad -=0.15;

  return true;
}

/* Enable smooth shading */
gl.ShadeModel( GL_SMOOTH );

/* Set the background black */
gl.ClearColor( 0.0, 0.0, 0.0, 0.0 );

/* Depth buffer setup */
gl.ClearDepth( 1.0 );

/* Enables Depth Testing */
gl.Enable( GL_DEPTH_TEST );

/* The Type Of Depth Test To Do */
gl.DepthFunc( GL_LEQUAL );

/* Really Nice Perspective Calculations */
gl.Hint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

rtri=0;
rquad=0;
t0=Timer.getTimeStamp();
Frames=0;

var viewport=Video.getViewport();

/* resize the initial window */
resizeWindow( viewport[2],viewport[3]);

/* wait for events */ 
while (true)
{
  Input.poll();
  drawGLScene( );
}
