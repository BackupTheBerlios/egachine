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
   \brief ModuleLoader
   \author Jens Thiele
*/

#ifndef EJS_MODULELOADER_H
#define EJS_MODULELOADER_H

#include <jsapi.h>
#include <streambuf>
#include <string>

namespace EJSModuleLoader
{
  //! register functions with Javascript engine
  JSBool
  onLoad(JSContext *cx, JSObject *obj);

  //! evaluate script from stream
  JSBool
  evaluateScript(JSContext* cx, JSObject* obj, std::streambuf* src, const char* resname, jsval* rval);

  //! evaluate Javascript file
  JSBool
  evaluateScript(JSContext* cx, JSObject* obj, const char* filename, jsval* rval);

  JSBool
  onUnLoad();
}

#endif
