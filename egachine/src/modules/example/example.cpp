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
   \brief Example module
   \author Jens Thiele

   This is an example module. You have to implement one C function:
   onLoad
   optionally you can also implement onUnLoad
   (onUnLoad must not call any spidermonkey functions)
   
   the function names must be prepended with ejsmodulename_LTX_
*/

#include <iostream>
#include <jsapi.h>

#ifdef __cplusplus
extern "C" {
#endif

  //! our native Javascript example function
  static
  JSBool
  jsexample
  (JSContext *cx, JSObject *, uintN, jsval *, jsval *rval)
  {
    JSString *s;
    if (!(s=JS_NewStringCopyZ(cx, "example function"))) return JS_FALSE;
    *rval=STRING_TO_JSVAL(s);
    return JS_TRUE;
  }
  
  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsexample_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    std::cerr << "Example module loaded\n";
#define P(x) std::cerr << #x << " = " << (x) << std::endl
    P( (void*)ejsexample_LTX_onLoad );
#undef P
    std::cerr << "Registering Javascript function example()\n";
    if (!JS_DefineFunction(cx,global,"example",jsexample,0,0)) return JS_FALSE;
    return JS_TRUE;
  }
  
  //! function called before module is unloaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsexample_LTX_onUnLoad()
  {
    std::cerr << "Unloading example module\n";
    return JS_TRUE;
  }

#ifdef __cplusplus
}
#endif
