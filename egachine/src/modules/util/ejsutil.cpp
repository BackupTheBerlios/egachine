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
   \brief Util module
   \author Jens Thiele
*/

#include <iostream>
#include <ejsmodule.h>
#include <cassert>

#ifdef __cplusplus
extern "C" {
#endif

  static
  JSBool
  getObjectID(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    JSObject *o;
    if ((!JSVAL_IS_OBJECT(argv[0]))
	||(!(o=JSVAL_TO_OBJECT(argv[0]))))
      EJS_THROW_ERROR(cx,obj,"object required as argument");
    if (!JS_GetObjectId(cx,o,rval)) return JS_FALSE;
    // this is needed for spiedermonkey v. <1.7
    // see news <4075FD39.3020107@meer.net>
    if (!(*rval&JSVAL_INT)) {
      assert(((jsid)o&JSVAL_TAGMASK)==0);
      *rval=(jsid)o|JSVAL_INT;
    }
    return JS_TRUE;
  }

  static
  JSBool
  isCompilableUnit
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    // no UC pendant?
    if (JS_BufferIsCompilableUnit(cx, obj, ctype, strlen(ctype)))
      *rval=JSVAL_TRUE;
    else
      *rval=JSVAL_FALSE;
    return JS_TRUE;
  }

#define FUNC(name, args) { #name,name,args,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(getObjectID,0),
    FUNC(isCompilableUnit,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  
  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsutil_LTX_onLoad(JSContext *cx, JSObject *util)
  {
    return JS_DefineFunctions(cx, util, static_methods);
  }
#ifdef __cplusplus
}
#endif
