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
   \brief Egachine utilities
   \author Jens Thiele
*/

#include <ejsmodule.h>
#include <cassert>

extern "C" {
  static
  JSBool
  hashObject(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    if (!JSVAL_IS_OBJECT(argv[0])) EJS_THROW_ERROR(cx,obj,"object required as argument");
    JSObject *o=JSVAL_TO_OBJECT(argv[0]);
    if (!o) return JS_FALSE;
    JS_GetObjectId(cx,o,rval);

    // this is needed for spiedermonkey v. <1.7
    // see news <4075FD39.3020107@meer.net>
    if (!(*rval&JSVAL_INT)) {
      assert(((jsid)o&JSVAL_TAGMASK)==0);
      *rval=(jsid)o|JSVAL_INT;
    }
    return JS_TRUE;
  }

#define FUNC(name, args) { #name,name,args,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(hashObject,0),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsegachine_LTX_onLoad(JSContext *cx, JSObject *obj)
  {
    return JS_DefineFunctions(cx, obj, static_methods);
  }
}
