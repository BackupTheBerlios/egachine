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
   \file sdlgl.h
   \brief all files using OpenGL should include this file and never the OpenGL headers
   \author Jens Thiele
*/

#ifndef SDLGL_H
#define SDLGL_H

#include <errno.h>
#include <iostream>

#define NO_SDL_GLEXT
#include <SDL_opengl.h>
#undef NO_SDL_GLEXT

#define GL_ERRORS() do{if (printGLErrors()) std::cout << "gl error at - see above";}while(0)

//! print OpenGL errors
/*
  \return the number of errors occured
*/
int
printGLErrors();

#endif
