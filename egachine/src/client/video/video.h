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
   \file video/video.h
   \brief Video subsystem Interface
   \author Jens Thiele
*/

#ifndef EGACHINE_VIDEO_H
#define EGACHINE_VIDEO_H

/*!
  The Video subsystem is a state machine and since the reference
  implementation uses OpenGL - it is heavily inspired by OpenGL.
*/

#include <string>

namespace Video
{
  struct Coord2i;
  
  void init(int w=0,int h=0, bool fullscreen=true);

  void toggleFullscreen();
  void iconify();
  void resize(int sx, int sy);
  
  void swapBuffers();
  int createTexture(unsigned dsize, const char* data, const char *extension=NULL, const char *mimeType=NULL);
  void drawTexture(int tid,float w=1, float h=1);
  void drawText(const std::string &text, bool hcentered, bool vcentered);

  //! get screen (window) coordinates for this point
  Coord2i project(float x, float y);
  
  void deinit();

  struct Coord2i
  {
    Coord2i()
    {}
    Coord2i(int _x,int _y)
      : x(_x), y(_y)
    {}
    int x,y;
  };

};

#endif
