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
   \file input/input.h
   \brief Input layer
   \author Jens Thiele
*/

namespace Input
{
  struct DevState
  {
    DevState() : x(0),y(0),buttons(0)
    {}
      
    char x,y,devno,buttons;

    bool operator==(const DevState &o) const
    {
      return (x==o.x)&&(y==o.y)&&(devno==o.devno)&&(buttons==o.buttons);
    }
    bool operator!=(const DevState &o) const
    {
      return !operator==(o);
    }
  };
  
  typedef short int Unicode;

  // callbacks
  void quitHandler();
  void toggleFullscreenHandler();
  void iconifyHandler();
  
  void devStateHandler(const DevState&);
  void charHandler(Unicode);
  void resizeHandler(int sx, int sy);

  // to implement
  void init();
  void charInput(bool active);
  unsigned attachedDevices();
  void poll();
  void deinit();
}
