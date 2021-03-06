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
#include <map>
#include <cassert>
#include <csignal>
#include <SDL_syswm.h>

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include "video.h"
#include "sdlgl.h"
#include "texture.h"
#include "boost/smart_ptr.hpp"
#include "fontdata.h"
#include "glfont.h"
#include "ejsmodule.h"

// Determine type of window manager we are probably using
#if defined(__unix__)
    #define WM_X11
#elif defined(__WIN32__)
    #define WM_WIN
#else
    #error Unknown window manager for default desktop resolution handling
#endif

typedef std::map<int, boost::shared_ptr<Texture> > Textures;
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

static
void resize(int width, int height, int m_flags)
{
  SDL_Surface* video;
  
  // Prevent A Divide By Zero By
  if (width==0) width=1;
  if (height==0) height=1;

  if (!(video=SDL_SetVideoMode(width, height, 0, m_flags))) {
    assert(SDL_GetError());
    EJS_WARN(SDL_GetError());
    throw Video::FatalError(SDL_GetError());
  }
  if (!(video->flags&SDL_OPENGL))
    EJS_WARN("could not get OpenGL context");
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
createWindow(int width, int height, bool fullscreen) 
{
  // todo: perhaps get as config parameters
  SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
  SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 1 );
  //  SDL_GL_SetAttribute( SDL_GL_ALPHA_SIZE, 1 );

  // todo check for available screen sizes
  // let user decide which one to use

  int sdlflags=SDL_OPENGL|SDL_ANYFORMAT;
#ifndef WIN32
  sdlflags|=SDL_RESIZABLE;
#endif
  if (fullscreen) sdlflags|=SDL_FULLSCREEN;
  try{
    ::resize(width,height,sdlflags);
  }catch(Video::FatalError){
    // try again without stencil buffer
    // (typically 16bit modes don't have a stencil buffer)
    SDL_GL_SetAttribute( SDL_GL_STENCIL_SIZE, 0 );
    ::resize(width,height,sdlflags);
    EJS_WARN("no stencil buffer");
  };

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
  if (SDL_GL_GetAttribute( SDL_GL_DOUBLEBUFFER, &db )) EJS_WARN("could not get attribute");
  if (!db) EJS_WARN("did not get double buffer");

  glViewport(0,0,video->w,video->h);

  glMatrixMode(GL_PROJECTION);		
  glLoadIdentity();
  glOrtho(0.0f,video->w,0.0f,video->h,-100.0f,100.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f(1.0,1.0,1.0);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_NORMALIZE);
  glDisable(GL_LIGHTING);
  glDisable(GL_CULL_FACE);
  glShadeModel(GL_FLAT);
  GL_ERRORS();
}

void Video::init(int width, int height,bool fullscreen)
{
  // hack to get sync to retrace (works at least on linux nvidia)
#ifdef HAVE_STDLIB_H
  putenv("__GL_SYNC_TO_VBLANK=1");
#endif

  textures=new Textures();
  if ( SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK|SDL_INIT_NOPARACHUTE) < 0 ) {
    throw Video::FatalError((std::string("Couldn't init SDL: ")+SDL_GetError()).c_str());
  }
  atexit(SDL_Quit);
  
  if ((!width)||(!height)) {
    // try to use current resolution
    EJS_INFO("try to use current screen resolution");
    
    // thanks to Stephan Ferraro
    // http://www.libsdl.org/pipermail/sdl/2003-December/058783.html

    SDL_SysWMinfo         info;

    SDL_VERSION(&info.version);
    
    if (SDL_GetWMInfo(&info)) {
#if defined(WM_X11)
      if ( info.subsystem == SDL_SYSWM_X11 ) {
	width = DisplayWidth(info.info.x11.display, 0);
	height = DisplayHeight(info.info.x11.display, 0);
      }else{
	EJS_WARN("using unknown subsystem:"<<info.subsystem);
      }
#elif defined (WM_WIN)
      width=GetSystemMetrics(SM_CXSCREEN);
      height=GetSystemMetrics(SM_CYSCREEN);
#else
#error implement it
#endif
    }else
      EJS_WARN("could not get window manager information");
  }

  createWindow(width,height,fullscreen);

  // load font texture
  int fontTextureID=createTexture(fontDataSize,(const char *)fontData);
  // create font
  font=new GLFont((*textures)[fontTextureID]);

#ifdef VIDEO_MOTION_BLUR
  // create texure for motion blur
  boost::shared_ptr<Texture> mbt(new Texture(motionBlurTextureSize));
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
  SDL_QuitSubSystem(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK);
  SDL_Quit();
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
  boost::shared_ptr<Texture> t(new Texture(dsize,data,extension,mimeType));
  int id=t->getTextureID();
  (*textures)[id]=t;
  return id;
}

void
Video::drawTexture(int tid, float w, float h)
{
  EJS_CHECK(textures);
  boost::shared_ptr<Texture> &t((*textures)[tid]);
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
  glPushMatrix();
  font->drawText(text,hcentered,vcentered);
  glPopMatrix();
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

int
Video::showMouseCursor(int toggle)
{
  return SDL_ShowCursor(toggle);
}
