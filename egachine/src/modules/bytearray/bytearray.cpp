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
   \brief ByteArray Object for javascript
   \author Jens Thiele

   Operations needed: (all operations must be safe - range checked)

   get size
   set size/resize (fill new bytes with zero or optionally given value)
   get byte
   set byte

   move bytes within byte array (like memmove)
   copy bytes from other byte array (like memcpy)

   ByteArray <-> js string:
     convert to / interpret as iso-8859-1 string (as 8-bit subset of ucs-2)
     convert to / interpret as js string (ucs-2) - odd number of bytes?
     set from ucs-2 js string
     set from ucs-2 js string (interpreted as 8-bit iso-8859-1 string)

   ByteArray <-> js array?

   toSource()?
*/

#include <ejsmodule.h>

extern "C" {

  static JSBool
  addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // todo:
    // we could do something similar as the native javascript array object
    // then the array could be indexed by []
    // this is because x[1] is treated like x.1 
    // (the array intercepts such calls) see jsarray.c in the spidermonkey
    // sources
    return JS_TRUE;
  }

  static
  JSClass bytearray_class = {
    "ByteArray",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  static
  JSBool
  bytearray_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }
    return JS_TRUE;
  }

#define FUNC(name, args) { #name,bytearray_##name,args,0,0}

  static JSFunctionSpec bytearray_methods[] = {
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsbytearray_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *proto = JS_InitClass(cx, global,
				   NULL,
				   &bytearray_class,
				   bytearray_cons, 0,
				   NULL, bytearray_methods,
				   NULL, NULL);
    if (!proto) EJS_THROW_ERROR(cx,global,"Could not init class");
    return JS_TRUE;
  }

  JSBool
  ejsbytearray_LTX_onUnLoad()
  {
    return JS_TRUE;
  }
}
