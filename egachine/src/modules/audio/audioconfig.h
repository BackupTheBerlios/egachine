/*
 * Copyright (C) 2002 Jens Thiele <karme@berlios.de>
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
   \file audioconfig.h
   \brief audio configuration
   \author Jens Thiele
*/

#ifndef AUDIOCONFIG_H
#define AUDIOCONFIG_H

#include <string>

//! class holding audio configuration settings
struct AudioConfig
{
  AudioConfig()
    : sdriver("sdlmixer"), srate(44100), sbits(16), sbuffers(512), stereo(true)
  {}
  
  std::string sdriver;
  std::string sdevice;
  int srate;
  int sbits;
  int sbuffers;
  bool stereo;
};

#endif