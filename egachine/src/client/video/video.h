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
  struct Color;
  struct ViewportCoordinates;
  struct Coord2i;
  struct Rectangle;
  
  void init(int w=0,int h=0, bool fullscreen=true);

  void toggleFullscreen();
  void iconify();
  void resize(int sx, int sy);
  
  void drawLine(float x1,float y1,float x2,float y2);
  void swapBuffers();
  int createTexture(unsigned dsize, const char* data, const char *extension=0, const char *mimeType=0);
  void drawTexture(int tid,float w=1, float h=1);
  void drawText(const std::string &text, bool hcentered, bool vcentered);
  void drawQuad(float w=1, float h=1);
  
  void setColor(const Color &c);
  Color getColor();

  void setViewportCoordinates(const ViewportCoordinates &coords);
  ViewportCoordinates getViewportCoordinates();
  
  void setViewport(const Rectangle &r);
  Rectangle getViewport();
  
  void pushMatrix();
  void popMatrix();
  void translate(float x,float y);
  void scale(float x,float y);
  void rotate(float r);
  void clear();
  //! get screen (window) coordinates for this point
  Coord2i project(float x, float y);
  
  void deinit();

  struct Color {
    Color(float _r, float _g, float _b, float _a)
      : r(_r), g(_g), b(_b), a(_a)
    {}
    
    float r,g,b,a;
    
    void set()
    {
      setColor(*this);
    }

    Color &adjust(const Color &o)
    {
      r*=o.r;
      g*=o.g;
      b*=o.b;
      a*=o.a;
      return *this;
    }
  };

  struct ViewportCoordinates 
  {
    ViewportCoordinates(){}
    ViewportCoordinates(float _left, float _right, float _bottom, float _top, float _near, float _far)
      : left(_left), right(_right), bottom(_bottom), top(_top), near(_near), far(_far)
    {}
    
    float left,right,bottom,top,near,far;
    void set() const
    {
      setViewportCoordinates(*this);
    }
  };

  struct Coord2i
  {
    Coord2i()
    {}
    Coord2i(int _x,int _y)
      : x(_x), y(_y)
    {}
    int x,y;
  };

  struct Rectangle
  {
    Rectangle()
    {}
    Rectangle(int _x, int _y, int _sx, int _sy)
      : x(_x), y(_y), sx(_sx), sy(_sy)
    {}
    int x,y,sx,sy;
    void set() const
    {
      setViewport(*this);
    }
  };
};

#endif
