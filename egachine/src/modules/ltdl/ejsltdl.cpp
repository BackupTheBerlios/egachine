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
   \brief libtool module
   \author Jens Thiele

   This is an libtool module.
   It allows to load libtool modules at runtime and to lookup wrapper
   functions.
*/

#include <ejsmodule.h>
#include <ltdl.h>

#ifdef __cplusplus
extern "C" {
#endif


  static
  void
  ltmodule_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass ltmodule_class = {
    "Ltmodule",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  ltmodule_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &ltmodule_class) did not work
  // this is probably caused by incorrect definition (defined or undefined) 
  // of JS_THREADSAFE

#define GET_LTMODULE_OBJ lt_dlhandle ltmodule=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &ltmodule_class)			\
      EJS_THROW_ERROR(cx,obj,"incompatible object type");		\
    ltmodule=(lt_dlhandle)JS_GetPrivate(cx,obj);			\
    if (!ltmodule)							\
      EJS_THROW_ERROR(cx,obj,"no valid ltmodule object")

  static
  JSBool
  ltmodule_getWrapper(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,3,argc);
    GET_LTMODULE_OBJ;


    // input
    
    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    uintN nargs,flags;
    if (!JS_ValueToECMAUint32(cx, argv[1], &nargs)) return JS_FALSE;
    if (!JS_ValueToECMAUint32(cx, argv[2], &flags)) return JS_FALSE;


    // main work

    // We assume this is a wrapper function => the C function must have this signature:
    // JSBool (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
    JSNative cfunc=(JSNative)lt_dlsym(ltmodule,ctype);
    if (!cfunc) EJS_THROW_ERROR(cx, obj, "Symbol not found");

    // output

    // build a JS Function calling this function
    JSFunction * jsfunc;
    if (!(jsfunc=JS_NewFunction(cx, cfunc, nargs, flags, NULL, ctype)))
      return JS_FALSE;

    // return the js function object
    *rval=OBJECT_TO_JSVAL(JS_GetFunctionObject(jsfunc));
    return JS_TRUE;
  }

#undef GET_LTMODULE_OBJ



#define FUNC(name, args) { #name,ltmodule_##name,args,0,0}

  static JSFunctionSpec ltmodule_methods[] = {
    FUNC(getWrapper,3),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  ltmodule_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);

    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }

    EJS_CHECK(JS_GET_CLASS(cx, obj) == &ltmodule_class);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;

    lt_dlhandle handle;
    if (!(handle=lt_dlopenext(ctype))) {
      const char *error=lt_dlerror();
      EJS_CHECK(error);
      EJS_THROW_ERROR(cx,obj,error);
    }
    if (!JS_SetPrivate(cx,obj,(void *)handle)) return JS_FALSE;
    return JS_TRUE;
  }

  static
  void
  ltmodule_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &ltmodule_class);
    lt_dlhandle ltmodule=(lt_dlhandle)JS_GetPrivate(cx,obj);
    if (!ltmodule) return;
    if (lt_dlclose(ltmodule)) {
      const char *error=lt_dlerror();
      EJS_CHECK(error);
      EJS_WARN("failed to unload module:"<<error);
    }
  }

  //! self check function (used by test script)
  JSBool
  ejsltdl_LTX_selfcheck(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    uintN res=0;
    for (unsigned i=0;i<argc;++i) {
      uintN j;
      if (!JS_ValueToECMAUint32(cx, argv[i], &j)) return JS_FALSE;
      res+=j;
    }
    return JS_NewNumberValue(cx, res, rval);
  }

  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsltdl_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    EJS_CHECK_TRUSTED(cx,module);
    LTDL_SET_PRELOADED_SYMBOLS();
    if (lt_dlinit()) return JS_FALSE;
    JSObject *proto = JS_InitClass(cx, module,
				   NULL,
				   &ltmodule_class,
				   ltmodule_cons, 0,
				   NULL, ltmodule_methods,
				   NULL, NULL);
    if (!proto) EJS_THROW_ERROR(cx,module,"Could not init class");
    return JS_TRUE;
  }
  
  //! function called before module is unloaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejsltdl_LTX_onUnLoad()
  {
    if (lt_dlexit()) return JS_FALSE;
    return JS_TRUE;
  }

#ifdef __cplusplus
}
#endif
