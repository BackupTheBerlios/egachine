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
  \brief posix module
  \author Jens Thiele
   
*/

#include <ejsmodule.h>

#ifdef __cplusplus
extern "C" {
#endif

  static JSBool
  ejs_execv(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    // todo: we loose unicode information here
    const char* filename=JS_GetStringBytes(strtype);
    if (!filename) return JS_FALSE;

    if (!JSVAL_IS_OBJECT(argv[1])) EJS_THROW_ERROR(cx,obj,"array object required");
    JSObject *aobj=JSVAL_TO_OBJECT(argv[1]);
    jsuint l;
    if (!JS_GetArrayLength(cx, aobj, &l)) return JS_FALSE;
    char* eargv[l+1];
    for (jsuint i=0;i<l;++i) {
      jsval elem;
      if (!JS_GetElement(cx, aobj, i ,&elem)) return JS_FALSE;
      strtype=JS_ValueToString(cx, elem);
      // todo: we loose unicode information here
      eargv[i]=JS_GetStringBytes(strtype);
      if (!eargv[i]) return JS_FALSE;
    }
    eargv[l]=NULL;

    execv(filename, eargv);

    // some error occured
    EJS_THROW_ERROR(cx,obj,"execv failed");
  }

  static JSBool
  ejs_system(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    // todo: we loose unicode information here
    const char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;

    int r=system(ctype);
    if (r==-1) EJS_THROW_ERROR(cx,obj,"system failed");

    return JS_NewNumberValue(cx,r,rval);
  }

#define FUNC(name, args) { #name,ejs_##name,args,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(execv,2),
    FUNC(system,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC
  
  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsposix_LTX_onLoad(JSContext *cx, JSObject *util)
  {
    return JS_DefineFunctions(cx, util, static_methods);
  }
#ifdef __cplusplus
}
#endif
