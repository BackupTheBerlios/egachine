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
if (!EGachine.checkVersion(0,0,7)) throw "at least version 0.0.7 required";
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

addResources();

var viewport=Video.getViewport();
Width=viewport[2];
Height=viewport[3];

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

function addResources(){
EGachine.addResource(
({name:"glass", size:8050, z:true, data:"\
eJyVt2VQG94TqB1IILi7a3H34KV40eCuxd0Lxd1D8eKF4O4OhaLFtQR+tHiQtlhxePu/8364X++zM2f\
mzO55Zj+dnX1Zf/kBIFBTUlUCoKCgAFT+BeDlGUCu4G1v7WtvxxDg5OvIoOdoz6CsqqH9ggAoAECoQC\
AQFfTvAP0DDRMd7R/YGBjomLjYuLi42Dg4eAQkhHj4xPg4OEQURMSkZOTk5HiElNSUZFQkZORk/5Og/\
O8lGhoWOjoWGT4OPtn/My9DAEIMQDggHIjCBEAlRAESoryMAmgAACDK/wHw/wNCQwcDUVABGP+ysgQA\
VJT/tQ4CAUFYwH81KKhAEBohgIgRXQBMLK9DwmTtJRgBK8Vg1iV9bTNMxsIqJBzZvPDzDznUZ/GfgBo\
FgAr8v+T/06MC0cEogH9JCcJ/OhAIFROIiob2f0pQUAkBQBAjGpGAvLUXOjGTzj/18AKJ4M/mP6Qsuj\
beZMyvI8vOXzYAOP96QSUEEgJkAc8BtZVVzbrrSeuOVGqO1cevqgIX9fLYDNkpNSWGpJ+UqsCGVz9Uv\
t97CzD9eXDz7XoccBM7/q/wdiDRJM8FjYRQd31R9VOLe6I/6DDZq25uus6xsx5NoZaOUm7jBaCNOYux\
Z+dQqIUxcuRMVO0j/41vIPr2199Y1WoNuxXMxgKUEiuEo0ib/oofa2ENc3ZaxVkK8HAHtNolNQf/Jip\
5M1tk0vhBQotqAmJR/M0a9smNi+ExATLHvpdfkAbZ0p+8E7eQKbx6jjzUZNZe1v8kN/mFD4y89F9qu/\
LBY32wGCTwzZfAeihe2+HiFjN964/U5f0Ez5dNzMqraF9sBfN+k5N4ZMgAXFGdQ2aHrFON7p2TmfnBo\
s42QUN0gzrVJ4pfjKr2EiEQeMJxOWNkFUOGFrP0FUOsgk3CaqJ9Kn/MgU2p5gF1WH9NVuSiPdaZ48Of\
nyRl+epElUs3bTozK22D5Izu+W133N+jeYpw9wNnQ12BHx9SPqRWtLVgMiN70Dcjmv7WWIzCfnmGQqv\
bE+udkf+ZUN68wcqiDhoyK/6KUD9bpAhQmRZ61BVdprgjEAN/vHnSLBXCLc2RIMrTLJNbszp0TB+Zb6\
t3AnP6nVtJzfVpL12zqWuDS3Iq3/njzcEUlDTNk6yIJH3CpdGj648tCqsqDUqb1NIkEPiF8uopT7Gnt\
sm7nfDMduE5c6hExUfbsRSKH6RA6YvAsxKmgjqSVh7ucugMIYMt6AFYbK6Xj4UZzoFnjkgiay+L0alg\
OQAF3TwqxHTLK5iS8uyMq56bAnsWF742RcHGdovuOXurXEmzE3MrWnOJMx1StzW6PZU8fbZTNPw0Fk0\
rteudF6YJHQmYUCUAH3+XwOteAHZ6Y1cXkXNsHShsUpc1s8Rqxw91Tw+Byv1vW55lYJUL1xqNP4dfAM\
ocLFb+DQhWCpwws8KA4mqGfIiP6uKVy4Au66Xrsu0eYK8BP2R3kPa6UCG0SPyzSvU7nD46ctg9covno\
uOzwCOtPLVaAxYOELlNr0H8iI8L9Dk8p39V+ALYCSlrutbIp7IagVu5DT3OOJUx7C57FbIjq392iTzX\
hMLaZ9d0hH2XKHYiOJKFS5r0AOHrnJsBkPzuzRrs3JiBkjOKsX1Z62GpD90rneqmu5XIcJvGxF43twl\
M20e+T7643aEFuxeLUu+i8HO41eR4Pbqa3M56TbhOZliuoo3f0CrmDx1TBzrtkPJIaykwzyYjOCB+/k\
F1hIMwjruPkkYPRJh0uctfIeQFHBaSBbTQaw19XMvYORFaM1JsGlpJ6tw93PZRIXcwbfzf5etElWc+H\
4LdNTDV8lWQ7HSRlz/t36IwgjEux3xgzFtYFGxbpbaeVC91ajraY2i7EXzzJBJgd2GK2y/qkCGAeNvQ\
0WMyJaatr+4ddhX6fq1Me2qHTs0XRvlba6mpY0IVe39iNHLIoVgr7NFRMvFXWas8JzF45O5biecpfwm\
9t6DXzQotPKa0Ip/Iu8wpkx94MJI1Nxh2sejQUITthlJTnsxKZ58wNJGlTw2UXs5Qpi9quZ1M73HyvX\
xnN0vhZZOh/PGrrxAcWqdXStny38+xIeKJ9KBxbFK9lKDyzuUoylTR1NdBjOt4sgyfL+c51FJhurvWu\
a2twAxm6nggQnpgtJCQxJu1b3WmTe/ORdv03tLTxucs9kn4soIbZg6x5qSS15KwIfLbG7tj1HPo2vDf\
/V2naKmuWyDjiJJZwYFiPqlNfoFjq8D82YfFUO11Q3p6O2qAJ1aIRuwMwLx6tWVv+UwsY6UtRmr4u1F\
yuHQXm8pneCBVYHlUsKI24YKEeDztN+kH4jvH6ScNna5E0Tu7EmjfmLgT8T7FnPjQ3LbGvSCCO2dqKv\
+UpepXi+Kz2DDbWVTE3XHiWxtN+lggvxRSlVF6lD1SNkQuzeVoxdQAlKSaW2aZiLbpYmMwEvDTyXRFS\
qDOQpcSV22EG68/k30Ef3rwsLxT7XVmC5p03+buNlsaQt/QuNqqV8ND0UHNuCJRhDYvt3FKuuk7lbml\
mu6c2CjK4bu34fzAbNeqBxdLZdF8b31Bnb4hg61ZcO+Yhafuckk9EZaZk8vJQMeP+RaK14LctVEpo8E\
frqbyJUwUOTH091wuSzL0AdmwNDaajioe/qG2B7MqFjfQvVPi/BuKzDe73Al6BWeu51crhrHC6fU0a0\
SzVCa1mWjYGcV/2O82xEEZ3nKLLhnQL7+7Gr8FhhR/s9Xok24ogTdNnkO/Etw+xGJ36WSXaPc/96ims\
Hzgp/mQzD16xu0T1dMDIUNiBboc28GjftZ9eRe3P37xTjJWstPVGW/N1CyjRVqqy+CXqTWiaP+D+Jhf\
wNqKnt4THylNS+WYOm5ioDWzjyBWUACBtJz4ohPHYjORX3LdOIXVpoP0+dksxgugoXTh6i2GEFGLMG5\
752GguTyH8Qo/+1aUm8Og5L28wXCAM0ydaOwdcE8rcTj0lrYZ+fsETioidu98VU6ShS8cIYcJGTgbSf\
aUWb+ZrVyxy3C0MsZjSo+WnMoQSnZ4jtM6aSRa2jCp+sKawYpKOU1iH+pPBiycCtqNivpdr7WikRklA\
MUAyGwTnLG63dBrxd92mltwua1CCsqXFyfgbPtRQjh4koXfoWk6n3va5LWWsquOViFxGeyJDZ6YctjE\
xfLJoe/pR5IUma8NxzQR1afNiXDYp4NsS343iudL+tzdR5OzFXMJfdfPlYMGQbU8sz+hHtUG9L84yeR\
+fBAsPzzu2VsfE3o0cM0o+aLSeUNqucBAfJeiiCvYhIQnqKNBg3k3UKrPTUu8Qa/FHQ/qunR1qmXBpl\
Rt8t2ImHdGz9+HDxd8/yIMnNwMOTFuKHdfGcnTFmriyUpXXkIGkUnSGWePElv75ZFG7h3itj1CJOtz7\
Hnhvmx970Q4YKfK8PJf88peJ9NmJHtK7fqEZQ1nJXGBlw8+d06g/euP91msGfSWZwyDT3F9QvBRqBBu\
sfa3bjsChPgU8aT0KhlZIh/ho6gJhLNn5wUwi3R+z2gxHsWt+0ZqUbnLp/d6uR+5wmUxMKzMo//2XEq\
LRr03pJYCRk2HAH68okI0hFGbQz37DSapjXuOIYnOxqKeoYTG2ocbdoaeWle2a0aV0Su4isu0AlCuaI\
eiLe2YWaDnhPxDIxUpdm53hE+wxrsPwf/mEilL1KYyhnKsHquR694UlMQ/DvU0ZBh3rm/UXoNWoLzdk\
OX4gpI5Q/kFcPrONvtJKXszzzqyMSyPaeZ5L/Vr6yXhC4DfzTAwz9UkjLSBjMJpFSJgipMECuPLbCV4\
Nil+624l3bW8ra2OJvKrZUkrAVldpEZFiV+9nZwhWTQiq/C7XobUXLTgOp5XBfICUO067+30FHBn5tV\
/rZ1gy86BxeMva631SgXwq4z57Mh5R4lDHUh/cjk3ffUofSWtYEiRy03AqEuCVsXW+djjQZMluDCudk\
RmEqL4auH2E32SdDgz2Ny6c6kFDbcXDD72W46Eipq8m9ZhiAv2+El5fR05+0GGibd8087ruZESuzAvh\
3kwxLZ4znCAdMHErggOwbSAwwz2bJ2Wybh5jlSZ53+zpcHhPYkFwt/OuzAA5j0+P8rbkD418arPLcxd\
v5I2hJT4txta10czBBsFWmM3Hu7ff6FRMpn6KPS/21modb/VC4DI6ad7jtqNsi3pwAilQEETOmUvfqh\
Q1YlfsKrhngzrO8vxAkzDTfoQ26sV6fPKPt8Cr2H+GFruBbKq1zWUNp9Ae8DUXebHdmJ3GOhECu94mv\
EupMNT2UY/fHjPjt78Q4SGnGqmlqN5F3XwG0e2z5AKWUeBP6JP8nGX0sK+CQJneDo/cTPOHxqXQ3lFb\
oSdRGadKTnz3zurhdQ2DeOMSOGboTwcCUHfLpwa0Q55n1C/3nr6ICsdoghFw9dR1EbsxeqnFPoTyBmg\
5cAmxxuSkYTGI5mshYuPRSLiZt+darC0sazgVN0TcDq3XfJQ3KmOQWJ2xuNXF9kZNe97o3G6hiPJfcF\
n7So3u9v6FqJhzvNXcV8sp5Ruer/iPqfydwnv+MPs4ot1uyjZ7tvI4wtpg7oXoLcz8iTeppxFElxuE0\
MqHHrcNrrBAlb8hl0Tv2frIT0zWruHJlD9BUNVRbtWRLTu8h04WK+bl3Hd6xaMi6AcOjTb0WCqdU7Pk\
x8GC1CxRJ4N5bgzvM7f9CJim67OrWVjH6gIp+NVcncYELdwDGt6AUSZkus8Ksh7R+tB6zTSnZE1e177\
9F6FGY3Nlg3N3WX1hXQ/SQ6w8uU7Cjt3uTrvkgduEW0f0gY6IUVJvdBr3I9lkJrfWubvmpem9e3gqWQ\
p7ryVryQENU1YGWImpw4H8KpP/e+p8Gqdk+K0sL+mdgQe7xkh0aAmB9YJGElofdX8df5R7a1j1JAw+Y\
WgLflPEFsuAYgkqhmur8wBwQugfkmpuw2ZtfrKii+kJJujPW1DlTsAqFAzuDP7ArBtv6WfN2e1aUyFm\
v5pWZrRYJDkIMNNe/XbhoWrOOTJrzhQwK59G2Jb696+WKa4RCtuBSeioSh2ycqkN4fLUG/umnwdG59/\
ARB69vimxbQHUBcsqtLWNAQHJWag26gVtjRhUjVzq1WK70osUt/Ezfh1r0PwZvyys9haBP1I0tll3Ii\
mWAxc/wYUyMjEtLEUxVOSVBAdSKBta3VrNpKF0nm9HchW1xCvUKOu4y37Zcw0HBuBSGDrUsK69KPXHg\
k75/qRq6wBUTkrsrlDfaNHd8to9IAqfcUX+8QEyR/AUq66UK1OzVKRyjPhoDR9NpBTeQEQrM6s+Gp/f\
49+8vOt6JLIc+B/VDpvo7FuPx2S5p2W2LpEfc8BxlJNQkyrzwplnSvqa4mweiBtVSWoonRmX/wEBdxH\
7xyhNM9SjkKczas4Zdks2K5f7jcluIPr25ydzvOgjnBWbO9Wnt6LOJ6ZIbY08V0euy+7xBNJJO1eFM7\
EUsrCb2KBVx6IN/nxi4wkjnpnKC1KhttnFUvt5m5TVEhkkWrf99oTEGE2wYwQGZbXKeZa5OGbTzfKkp\
IXwyXWXQ/+P1xKruy4rR3VUqI6b/sHaRiOv/Zdk2aLhl1MCYUhH63U61AN6IzuyGEf0+a0HsU/O6upu\
jDr2jv9ImwFzsqgyT63y+QAmjSYR/TuqPMMRPXFzz4mA2R6ao60IJut9X7KRWms4zwaxCq9+REYAHY2\
Ck6hq9mqcTY1NSh70cJgeGdX8n+sMtocmU9lh0Omm681y0G3Tkya9W08pSqgVNGZtPVj7w3R8BJXzC8\
oyL/3zid1dJF0EWQ3RYRQQ3WbbPb1lF9lEjcE0t2Uyyst2cVBULZ1vO2H2aQhIlAFV1pTEv8Do3LA39\
S25uZKbkwW2G2EbFGPGYeMKGlsOT9z7p5OStxmMPK6JTuPY1449y/zw8C9qDME1Vtpf1ycdmBj2T5HB\
N1sCAmJwivnkI8X6ywAL9rWDC4cuSldQ9adaun8ylWbe0r59WKYmzw+ObH6zArVBwzPNtXljE5kH7OX\
mlZgRlTEhDQm1iZnQ5n8jbHIKcG4EYiIqoFw05DsmvhX3KVc5Rd7YAbiXzLflyTbW+JHyRRtoiOH8fD\
BjFn9+JTgrl15qFVbdQVGumDV1u35U1cqZqpvYkXg3tUyzRCPqhWgSdQdIhu0KkZPfCpCjgjLUWTx3G\
Kv1a1B6D6VZdmyOTE/NFTAuk4PU1iaLBrIgpzZapr/c8RJj8mziIGr5FCrXjJtYn1cf+qiOeLD7dNoW\
9Addzpxnpxf0uP8+j4zjNMO0S51IZHyRBm/oYmnpIL/qc6J89QvQFK/6Emkiz8Ru54b5XNm7V7PGH4u\
NJWGA3w2m+W/x8c0Qi3OvHZZTteSmIBu18ncJo8W73k2e+/8oMhCq2gP//NsUf7+tgihJrVWlTKCW6r\
CYLvmBaS98EpGWkZ1JPC06W+p+8SU9lkkVNekIGRxCfQvSsDzTJ2yO9x++AEdppCBn54n6mduHmrG1V\
PLrFLeGLjqY/Kk7SpD1SE/OLMmscTs/n57VD8fqeJrF6PLRyYt6n13iB5+gv891zI2oACsA87kI1IQe\
sXWVaXENa/4NeZyhLbo4G/z37u2rNil7x2vdpIxyLKb71Sm2L4A0u+zB+FHZGVllU+f5D1hL4AcDVR0\
nPPU09nT8Xkcvg/2MP3N397aC2a1DtlFeZGL4513npcZhGAg22C19LWyB3KKGeQpfD4SMw6yS3MtGrE\
ky/pcfuzM6cxmKe0AkLgHnty/ALa1ZGOWmFSxKROsLBMG+VaxhgV0ZIlgnEbWuU6XtJScRxwzwpznIN\
fpjzkBzMnFBuPIBoNpu3YRzAMleduLqKfdubAlAxG9ZethWKcyeCe9yHxux/Z6jupmMiJSWN6WU858P\
G0gbTZTHSObQtnx+Hdgbhd0Gsd7GCfQfip7Lswn7d6pOJYEnsHhlIVVo+pz5x04g/r33qU/pG2hBVNF\
XDIeiK5w9O+/G4QnHHFlseqtxLd8PbfwG4NSSHjwPUgYyrqmrRnzZuHv0ac1x1D0KBt87P0iS1A//F4\
w++jXG+Flf5L0mK2UPzN3Ty8AaZbI7wy5gmrRBBgMf5S7Vir1NKDKP+18FjE2PjxJNts/Ut6+LyD+hV\
7KfnoAVjEpqxLd8W7HiR8tSAf8VFIdonHyx4eKoDhhnm78HgxAovzsd57KIPzevv1hxNvRi0SE2tA2o\
/wHrVhpRLI6ZXHssBmdymxmexFpin3q+31OLyaq70JRxxJrdi07Dey7DfUX+/taY6I+p0G7hY6ECeHe\
+9dxTpVFkgaXj4p8Cqbfj2qLqArCGtWpufjvhpgHG5E19yYSuMJeMju93ppCD/cnDK2WvP7UT+91mqv\
hqc038hIsncwssp7bUNjVatKiua1iQBWBAhvMMFxDeJSlIPg9/33jWgp/RZ3mmwoD2X5ZDxriikA6Rq\
YWN71RqQqi11HpNL2046LncSKr7M7aFTZtf1R67azD1X5EFWs4WpBvc51wKCttmWvoi/A+uKnRUE35f\
RE9R3La+6RiSHbtYUjwek8VJ0pPHpXsgh8Hf5cyViGPemLH/Y6Nj0s7oBqCWlKkfDTpkqNvPKgZ8fB+\
gOnK8fxw3qKpnOBvFm8fEFt9fJUYPUX2ZhzsiU3mQXaAH6y8dh6Try6dVuHknDttrH1NKhC7N06PZRR\
M5oFA1oWu2ZoEWoowGr8AeqJ47u8dG6qyW3obPfRvxBoqjTmbkiy5NKA04ujC/raFCJUJtki/Ow+dsh\
ARrCmSP8afBtc6hJXI2e68RN4d94mNCqkbLk0viJ6n8tnydBg0GpUCBfl/DWoGsV3PpyuqFap7kKhys\
Y3WxIWNHoIHBn6JcRrtoTcEZL+WMcrhHt1FmPFk6fMFVVbVIJRxvPdouLokKf5KvbeluVlpb/deLGg4\
WKW5ebqIfaIqqc8z5Sxn2FMFFYL3vubC4+ZwLsnd1kEXVR/ilpKz71022olUYvMDuEe5JCjp0x+c/bM\
7+g9B5pzRSwNXST/L7hJ+EMOGgGvmjV5m1ewbhgOaF3TEBEqf2QYA9AolWfrB5fch43b3pU9sSwcY/d\
oh+m8iMUPtGbXEWf4C8M2l/vw3TmJQU7NeGatPamjiw4SSobuRHPR3kvkCEg93qVs+JbqYz9PpHEwvq\
fH/JUF575P6X37zn43mipLsjIzwhWHqLfyRRs5oUxiewGZIedpASz9HbW6PFzvDfnewM3oRb0ipKKte\
BQ/GhFF3sFGs9c7erVy/lypu55cqhs6laBEu66p0Mp4cfj27C60w9438ssxJEudFVBYGjkPOsMrflUU\
cBpy+7WXrMx2qR6fuEa2RZJj9wQ7csuZKtSJcLd+MlDlmf4QBU5XXj5O5iRlytnfwvvCwKv+btcGsut\
M4mde4XXzg+/DZxCqjHx5xbpsEtt781EUSyxUiE9Mgv/kmbhnVT5tPymtv8w953nolyDjePjQizuBol\
PFMdGr5gIj644w1s3wrnGLqV7ungQv+1CGknTVd+4p+qeKW4JS+JhDhhNQhM7TjvoN0DYUSLZT3tcTv\
iYV+Ee8MrhVTlChu7cbtjy+AFIpsDjnp5zESnszMqO6/AqZuA1uuN0iet3S7GOA0MWJ2nYnJHc1HB3D\
qocZXXXOGVEnaCUVtd1+n9h28eYk80eaU7h/3hFX/Fr7Qgfgc/0tGWKfDx0uvQu0tJGt91efUJbVF88\
cFVmxoSCsZtPs1bPuAHT1s+o2epqDjQVXRbFGUluyzEhfhURbR4ZO+7IOzKlr4aTYDsnl/n/LsvMHPX\
N2S9aBDrTZzpTVE3v5uP11C93XE1RWVaOyWsQ6Ulvt0ubntlW5tFFeDCqBoJEvmdokaEr8KV0WOisxw\
iuhlFEwyExk1286assmanUQaPCrrDOYkK78hSR1tDti9+V29tMxpl2CMzRIB//t7bbI0i02Jg2CFuNd\
Zx3/dpeNsDF3dnkf6F31s0rQxE2zgHff4gaY6XTAt1fLVq0Ol66QeioNLFvqVGJ80xM0iYj8Etl2dwE\
CoztkEphhhp3iSdQsi9W3ddLRSGU1jVZrHmPWmAHy41v0ir027uyEcpJvAyoFLtiWmQKtXazIf45zzr\
IZvIfTxCti0xtvzGa5IwaJMleV/Y7vt0b17Mxbeu+3CzpLNSZ4oAt0vD7ePojseGPKaW1Jv6KRKREST\
EHD16YVjDN8lt7xvfry8dz5qkK6tvLWxcaJj9yzvppeaiWjXDy1w2lbQXprDtR9oASBxri9qyK0Uwhy\
qaSIlnXhYEnAnj3l9wPl8WTYTP9JGwYjlB+Ra3BrFXrfB4iBp9dNnrvFJ9tgYucPOUPw1lfdyJtlbz2\
Pka0VDNFZpRJXtX+dJfMzZ8+uZ4yrPsZLvon46UIcWhzUioLQii7TyPvCLjYYI/TLD0W7b9lg9gYnNY\
S/kbF/vpGkQkguksdzxna5rQe4KBkr+wpiShd0qIwgsOASKn9VIikQi1TV13Ws4pZqiJroWBQB0gGfP\
Hz73SF5qY5VoykQdH7eNTZ7tuiBOnc9F+vzpKmToM/5LwV9cwhSuuT01SLnbsgjuUugJDj0pWHCarqy\
UD0uV1a4/sCy0UNhFfL4k0ggPiqL8QQaCIdK29as9BlltWMX0uKADu0Lq3hV41MmEj6c+s6oyG2Q1X4\
NNSpb7DSnsJPSzd1V0pAHA2XF4Po5BsuPg2gHG/XePmjLUcVWf+roGxyIiGU4tiWRlwPn1tZRjkr30W\
3NTHew5rHaxbGoBokpK6dbn9yVITVjWmaDevGV11qvqAL/8ARhFjwB0OEd86jmEltLjaI6zBd5QdhBr\
ki41k3VltCsbJ/jkx5i4yMJCZBZqg1GTOflNY1CAR1/fUD2sJEMMbtu8YDYkBwRcxu5d1j/cUmfPeJ6\
rOfXkITfgEHvR+aROJ92x10AxFnRwamiBC2Xx5xr77jjnY/tIcqYCHj8O/9cl9oOfUjadMs0xuZuDBP\
6rHvKU3/L+/Brlx+VBwFo7RQXNpJqSbe3N0aZNft7iuwpV0PY5yfbXbT3f+sOvOXVHJlAcF/V9M109C\
qEEs4iLAgcn0IM7cKAVG7qyulWZ860iZjeGYkeU4XGcrv+ylrfoz6/EgQVaV0N2IjJuUo3FBCM3m8p9\
QYeeF0BRTzG+fYnQpGsfog9ZtlFWc6jaGakySeQ1FwYfqBzz/I75tv2t6Jr7s8BpYqa6nEI2fXFg7OD\
SbE6ib77JhxnsLEZqLExLnwbC35+75ld49ECJEc3Zogyi1DQOayY0zxSb7lqkiubHs3P7rl64q8lWXr\
c+fF3Xa9S/Y8mm7WNNNmrbakHESvsGPz1gSWQY/LUCLPbd9z6NcNtc74SJlj3EUrKGiKM/tPULhLyqp\
hKGZt7Omon2Aoj195Tc1zzF19VaszBz8sDkXpJzoo2KVRZoEbI69AnZ5rRdvrla4SGPg2+ZvIu9U2Yr\
NXbXw6mfewE00mfH1hxRl7tR6Vq6PSuoUZW+69N5nBIMHC24TJX9ILfTkvCQTOXC/xRxfZdAFe9+4nj\
5mC9uwL8YYvsasVmsrSo9Oe5mGGjnNlJpERwiqJhfm+GaGcYf1rrLTXHD5mlbbHbSv/t5vyFF0VBXPl\
NCTz0LQydinM4DXs6Db7iUlaNPjngvjXP5pe2bgeP2t4phP3t3VQAfKBlpwKevJYAStLyDaL+1ABVqv\
erxlsm6DHdVR+VW7V5dPU97+f7/AZ+4CtI=\
"}));
};
