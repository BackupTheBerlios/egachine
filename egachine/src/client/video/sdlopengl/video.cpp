/*
 * Copyright (C) 2004 Jens Thiele <karme@berlios.de>
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*!
   \file sdlopengl/video.cpp
   \brief 
   \author Jens Thiele
*/

#include <string>
#include <iostream>
#include <stdexcept>
#include <fstream>

#include <SDL.h>
#include "../video.h"
#include "sdlgl.h"
#include "texture.h"
#include "typedefs.h"
#include "smartptr.h"
#include <map>
#include "fontdata.h"
#include "glfont.h"
#include <cassert>
#include <csignal>

#include <SDL_syswm.h>

// Determine type of window manager we are probably using
#if defined(__unix__)
    #define WM_X11
#elif defined(__WIN32__)
    #define WM_WIN
#else
    #error Unknown window manager for default desktop resolution handling
#endif


#if defined(WIN32)
// #define NEED_WIN32_HACK 1
#include <SDL_main.h>
#include <windows.h>
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif


#ifdef NEED_WIN32_HACK

#define USE_MESSAGEBOX 1

/* Show an error message */
static 
void ShowError(const char *title, const char *message)
{
  /* If USE_MESSAGEBOX is defined, you need to link with user32.lib */
#ifdef USE_MESSAGEBOX
  MessageBox(NULL, message, title, MB_ICONEXCLAMATION|MB_OK);
#else
  fprintf(stderr, "%s: %s\n", title, message);
#endif
}
#endif // NEED_WIN32_HACK

#endif // WIN32

static bool m_lineSmooth=true;
static
Video::ViewportCoordinates viewportCoordinates;

typedef std::map<int, JGACHINE_SMARTPTR<Texture> > Textures;
// we do not use complex static data structures => pointer
static Textures* textures=NULL;
static GLFont* font=NULL;

#ifdef VIDEO_MOTION_BLUR

static int motionBlurTextureID=0;
static int motionBlurTextureSize=1024;
static int frame=0;


static
void
motionBlur()
{
  int m_width=SDL_GetVideoSurface()->w;
  int m_height=SDL_GetVideoSurface()->h;
  // motion blur
  Color tmp(getColor());
  glColor4f(1.0f,1.0f,1.0f,0.3f);
  glPushMatrix();
  glLoadIdentity();
  glTranslatef(m_width/2,m_height/2,0);
  glScalef(m_width,m_height,0);

  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D,motionBlurTextureID);

  const float w2=0.5;
  const float h2=0.5;

  glBegin(GL_QUADS);
  glTexCoord2f(0,0);
  glVertex2f(-w2,-h2);
  float right=float(m_width)/motionBlurTextureSize;
  float top=float(m_height)/motionBlurTextureSize;
  glTexCoord2f(right,0);
  glVertex2f(w2,-h2);
  glTexCoord2f(right,top);
  glVertex2f(w2,h2);
  glTexCoord2f(0,top);
  glVertex2f(-w2,h2);
  glEnd();

  glDisable(GL_TEXTURE_2D);

  // todo this should be time based
  //  if (!(++frame%3))
  (*textures)[motionBlurTextureID]->copyFromScreen(m_width,m_height);

  setColor(tmp);
  glPopMatrix();
}

#endif

static
bool
isFullscreen()
{
  SDL_Surface* video;
  video=SDL_GetVideoSurface();
  assert(video);
  return (video->flags)&SDL_FULLSCREEN;
}

static
void
setMouseCursor()
{
  if (isFullscreen())
    SDL_ShowCursor(SDL_DISABLE);
  else
    SDL_ShowCursor(SDL_ENABLE);
}

//! set projection matrix
/*
  \note this must be the only function manipulating the projection matrix!
  since we keep track of the current viewport coordinates
  we could also try to reconstruct the glOrtho call from the
  projection matrix
*/
void
Video::setViewportCoordinates(const Video::ViewportCoordinates &c)
{
  viewportCoordinates=c;
  glMatrixMode(GL_PROJECTION);		
  glLoadIdentity();
  glOrtho(c.left,c.right,c.bottom,c.top,c.near,c.far);
  GL_ERRORS();
  glMatrixMode(GL_MODELVIEW);
}

Video::ViewportCoordinates
Video::getViewportCoordinates()
{
  return viewportCoordinates;
}

static
void
setViewport(int x, int y, int sx, int sy)
{
  glViewport(x,y,sx,sy);
}

void
Video::setViewport(const Video::Rectangle &r)
{
  ::setViewport(r.x,r.y,r.sx,r.sy);
}

Video::Rectangle
Video::getViewport()
{
  GLint view[4];
  glGetIntegerv (GL_VIEWPORT, view);
  return Video::Rectangle(view[0],view[1],view[2],view[3]);
}


static
void resize(int width, int height, int m_flags)
{
  SDL_Surface* video;
  
  // Prevent A Divide By Zero By
  if (width==0) width=1;
  if (height==0) height=1;

  JGACHINE_MSG("Info:","::resize: "<<width<<","<<height);

  if (!(video=SDL_SetVideoMode(width, height, 0, m_flags))) {
    JGACHINE_WARN(SDL_GetError());
    JGACHINE_THROW((std::string("Couldn't set video mode: ")+SDL_GetError()).c_str());
  }
}


void
Video::resize(int width, int height)		
{
  SDL_Surface* video=SDL_GetVideoSurface();
  if (!video) return;
  ::resize(width,height,video->flags);
}

static
void
killWindow()
{
  // set all function pointers zero
#ifdef DLOPEN_OPENGL
  deinitGLSymbols();
#endif
  SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
  //  JGACHINE_MSG("Info:","SDL_Quit()");
  SDL_Quit();
}

static
void
createWindow(int width, int height, bool fullscreen) 
{
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

  // todo check for available screen sizes
  // let user decide which one to use

  int sdlflags=SDL_OPENGL|SDL_ANYFORMAT;
#ifndef WIN32
  sdlflags|=SDL_RESIZABLE;
#endif
  if (fullscreen) sdlflags|=SDL_FULLSCREEN;
  ::resize(width,height,sdlflags);

  SDL_Surface* video=SDL_GetVideoSurface();
  // todo
  assert(video);

  // hack for 8bit displays (especially for my sdl+osmesa+aalib and sdl+osmesa+fbcon hack)
  if (video->format->BitsPerPixel == 8) {
    SDL_Color colors[256];
    int i;
    //    int c=0;
    for(i=0;i<256;i++){
      colors[i].r=i; //(c&0xff0000)>>16;
      colors[i].g=i; //(c&0x00ff00)>>8;
      colors[i].b=i; //c&0x0000ff;
      //      c+=(0xffffff)>>8;
    }
    SDL_SetPalette(video, SDL_LOGPAL|SDL_PHYSPAL, colors, 0, 256);
  }

  // set window title (if there is a window)
  char *caption="EGachine";
  SDL_WM_SetCaption(caption,caption);

  setMouseCursor();

  int db=0;
  if (SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &db )) JGACHINE_WARN("could not get attribute");
  if (!db) JGACHINE_WARN("did not get double buffer");

  setViewport(0,0,video->w,video->h);
  setViewportCoordinates(Video::ViewportCoordinates(0.0f,video->w,0.0f,video->h,-100.0f,100.0f));

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  GL_ERRORS();
  glClear(GL_COLOR_BUFFER_BIT);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glColor3f(1.0,1.0,1.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  GL_ERRORS();


  glDisable(GL_NORMALIZE);
  GL_ERRORS();
  glDisable(GL_LIGHTING);
  GL_ERRORS();
  glDisable(GL_CULL_FACE);
  GL_ERRORS();
  float range[2];
  glGetFloatv(GL_LINE_WIDTH_RANGE,range);
  GL_ERRORS();
  //  std::cerr << range[0] << "<LINE_WIDTH<"<<range[1]<<"\n";
  if (!m_lineSmooth) {
    glDisable(GL_LINE_SMOOTH);
    GL_ERRORS();
  }else{
    glEnable(GL_LINE_SMOOTH);
    GL_ERRORS();
  }
  glShadeModel(GL_FLAT);
  GL_ERRORS();

  //  if (getGUIConfig().quality<=1) {glDisable(GL_DITHER);GL_ERRORS();}
}

#if 0
extern "C" {
  //! signal handler for fatal signals - to ensurce SDL_Quit is always called
  /*!
    \note
    wanted to use sdl's parachute mechanism which catches fatal signals
    that are not yet catched
    gcj catches signals
    (f.e. SIGSEGV and throws a java NullPointerException)
    => even on a segfault we should reach deinit
    (hmm perhaps user code should not be able to catch such exceptions!)
    but somehow SDL thinks gcj doesn't catch signals?
    => we do not use sdl's parachute

    some fatal signals are not caught by gcj nor by SDL

    should we call SDL_Quit - no matter what happens?
    but perhaps this could lead to more serious problems than
    the onces we have if SDL_Quit isn't called

    hmm for testing i tried raise but:
    raise(SIGSEGV);
    and
    ((char*)10)[9]='x';
    are not treated equally


    for now i am happy with gcj's handling of SIGSEGV
  */
  static
  void
  catchFatalSignal(int sig)
  {
    //    JGACHINE_MSG("Info:","SDL_Quit()");
    SDL_Quit();

    JGACHINE_FATAL("fatal signal");
  }
}
#endif

void Video::init(int width, int height,bool fullscreen)
{
#if defined(NEED_WIN32_HACK)
  // hack for mingw to replace/reproduce SDL_main stuff
  //  ShowError("WinMain() error", "will do preininit now");
  if ( SDL_Init(SDL_INIT_NOPARACHUTE) < 0 ) {
    std::string error(SDL_GetError());
    ShowError("WinMain() error", error.c_str());
    JGACHINE_THROW((std::string("Could not preinit SDL: ")+error).c_str());
  }
  SDL_SetModuleHandle(GetModuleHandle(NULL));
  // todo redirect stdout and stderr to files like SDL_main does
#endif

  textures=new Textures();
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE) < 0 ) {
    JGACHINE_THROW((std::string("Couldn't init SDL: ")+SDL_GetError()).c_str());
  }
  atexit(SDL_Quit);
  
#if 0
  signal(SIGILL, catchFatalSignal);
#endif
  
#ifdef DLOPEN_OPENGL
  loadGL(getGUIConfig().libGL);
  loadGLU(getGUIConfig().libGLU);
  lookupGLSymbols();
#endif  

  
  if ((!width)||(!height)) {
    // try to use current resolution
    JGACHINE_MSG("Info:", "try to use current resolution");
    
    // thanks to Stephan Ferraro
    // http://www.libsdl.org/pipermail/sdl/2003-December/058783.html

    SDL_SysWMinfo         info;

    SDL_VERSION(&info.version);
    
    if (SDL_GetWMInfo(&info)) {
#if defined(WM_X11)
      if ( info.subsystem == SDL_SYSWM_X11 ) {
	JGACHINE_MSG("Info:", "seems we are using X11");
	width = DisplayWidth(info.info.x11.display, 0);
	height = DisplayHeight(info.info.x11.display, 0);
      }else{
	JGACHINE_MSG("Info:", "using unknown subsystem:"<<info.subsystem);
      }
#elif defined (WM_WIN)
      // TODO
      width=GetSystemMetrics(SM_CXSCREEN);
      height=GetSystemMetrics(SM_CYSCREEN);
      JGACHINE_MSG("Info:", "w:"<<width<<" h:"<<height);
#else
#error implement it
#endif
    }else
      JGACHINE_MSG("Info:", "could not get window manager information");
  }

  createWindow(width,height,fullscreen);

  // load font texture
  int fontTextureID=createTexture(fontDataSize,(const char *)fontData);
  // create font
  font=new GLFont((*textures)[fontTextureID]);

#ifdef VIDEO_MOTION_BLUR
  // create texure for motion blur
  JGACHINE_SMARTPTR<Texture> mbt(new Texture(motionBlurTextureSize));
  assert(mbt.get());
  motionBlurTextureID=mbt->getTextureID();
  (*textures)[motionBlurTextureID]=mbt;
#endif
}

void Video::deinit()
{
  if (font) {
    delete font;
    font=NULL;
  }
  if (textures) {
    delete textures;
    textures=NULL;
  }
  killWindow();
}



void
Video::drawLine(float x1,float y1,float x2,float y2)
{
  glBegin(GL_LINES);
  glVertex2f(x1,y1);
  glVertex2f(x2,y2);
  glEnd();
}



void 
Video::swapBuffers()
{
#ifdef VIDEO_MOTION_BLUR
  motionBlur();
#endif
  SDL_GL_SwapBuffers();
}

int
Video::createTexture(unsigned dsize, const char* data, const char *extension, const char *mimeType)
{
  JGACHINE_SMARTPTR<Texture> t(new Texture(dsize,data,extension,mimeType));
  int id=t->getTextureID();
  //  JGACHINE_MSG("Info:", "Texture ID: "<<id);
  (*textures)[id]=t;
  return id;
}

/*
int
Video::createTexture(const std::string &resname)
{
  // TODO: security / speed
  std::string res;
  std::ifstream in(resname.c_str());
  JGACHINE_CHECK(in.good());
  char c;
  while (in.get(c)) res+=c;
  return createTexture(res.length(),res.c_str());
}
*/

void
Video::drawQuad(float w, float h)
{
  const float w2=w/2;
  const float h2=h/2;

  glBegin(GL_QUADS);
  glVertex2f(-w2,-h2);
  glVertex2f(w2,-h2);
  glVertex2f(w2,h2);
  glVertex2f(-w2,h2);
  glEnd();
}

void
Video::drawTexture(int tid, float w, float h)
{
  JGACHINE_CHECK(textures);
  JGACHINE_SMARTPTR<Texture> &t((*textures)[tid]);
  if (!t.get()) return;
    
  glEnable(GL_TEXTURE_2D);

  // bool blend=(t->isTransparent());
  // if (blend) glEnable(GL_BLEND);
  // glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBindTexture(GL_TEXTURE_2D,tid);

  const float w2=w/2;
  const float h2=h/2;
  float* tc=t->texcoord;

  glBegin(GL_QUADS);
  glTexCoord2f(tc[0],tc[3]);
  glVertex2f(-w2,-h2);
  glTexCoord2f(tc[2],tc[3]);
  glVertex2f(w2,-h2);
  glTexCoord2f(tc[2],tc[1]);
  glVertex2f(w2,h2);
  glTexCoord2f(tc[0],tc[1]);
  glVertex2f(-w2,h2);
  glEnd();

  glDisable(GL_TEXTURE_2D);
  //if (blend) glDisable(GL_BLEND);
}

void
Video::drawText(const std::string &text, bool hcentered, bool vcentered)
{
  assert(font);
  pushMatrix();
  font->drawText(text,hcentered,vcentered);
  popMatrix();
}

void
Video::setColor(const Color &c)
{
  glColor4f(c.r,c.g,c.b,c.a);
}

Video::Color
Video::getColor()
{
  float c[4];
  glGetFloatv(GL_CURRENT_COLOR, c);
  return Color(c[0],c[1],c[2],c[3]);
}



void
Video::pushMatrix()
{
  glPushMatrix();
}

void
Video::popMatrix()
{
  glPopMatrix();
}

void
Video::translate(float x,float y)
{
  glTranslatef(x,y,0);
}

void
Video::scale(float x,float y)
{
  glScalef(x,y,0);
}

void
Video::rotate(float r)
{
  glRotatef(r,0,0,1);
}

void
Video::clear()
{
  glClear(GL_COLOR_BUFFER_BIT);
}

void
Video::toggleFullscreen()
{
  SDL_Surface *screen = SDL_GetVideoSurface();
  if (!screen) return;
  if (SDL_WM_ToggleFullScreen(screen)) {
    setMouseCursor();
  } else {
    // printf("Unable to toggle fullscreen mode\n");
  }
}

void
Video::iconify()
{
  SDL_WM_IconifyWindow();
}

Video::Coord2i
Video::project(float x, float y)
{
  GLint view[4];
  GLdouble model[16], proj[16], win[3];
  glGetIntegerv (GL_VIEWPORT, view);
  glGetDoublev (GL_MODELVIEW_MATRIX, model);
  glGetDoublev (GL_PROJECTION_MATRIX, proj);
  if (!gluProject(x,y,0,
		  model,proj,view,
		  &win[0],&win[1],&win[2])){
    // todo BUG: we are in trouble
    GL_ERRORS();
    JGACHINE_WARN("gluProject failed");
    win[0]=win[1]=0;
  }
  return Coord2i(win[0],win[1]);
}
