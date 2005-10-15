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
   \brief Tcc module
   \author Jens Thiele
*/

#include <iostream>
#include <ejsmodule.h>
#include <libtcc.h>

#ifdef __cplusplus
extern "C" {
#endif

  //! TODO: remove this
  static
  const char* lastError=NULL;

  //! TODO: remove this
  static
  void ejstcc_onerror(void*, const char* msg)
  {
    lastError=msg;
  }

  static
  void
  ejs_TCCState_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass ejs_TCCState_class = {
    "TCCState",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  ejs_TCCState_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &ejs_TCCState_class) did not work
  // this is probably caused by incorrect definition (defined or undefined) 
  // of JS_THREADSAFE

#define GET_EJS_TCCSTATE_OBJ TCCState* tccState=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &ejs_TCCState_class)			\
      EJS_THROW_ERROR(cx,obj,"incompatible object type");		\
    tccState=(TCCState *)JS_GetPrivate(cx,obj);				\
    if (!tccState)							\
      EJS_THROW_ERROR(cx,obj,"no valid TCCState object")

  static
  JSBool
  ejs_TCCState_compile
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_EJS_TCCSTATE_OBJ;

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    
    lastError=NULL;

    if (tcc_compile_string(tccState, ctype)) {
      std::string m("Compilation failed:");
      if (lastError)
	m+=lastError;
      else
	m+="reason unknown";
      EJS_THROW_ERROR(cx, obj, m.c_str());
    }
    
    return JS_TRUE;
  }

  //! link
  static
  JSBool
  ejs_TCCState_relocate
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_EJS_TCCSTATE_OBJ;

    if (tcc_relocate(tccState)) {
      std::string m("Relocation failed:");
      if (lastError)
	m+=lastError;
      else
	m+="reason unknown";
      EJS_THROW_ERROR(cx, obj, m.c_str());
    }

    return JS_TRUE;
  }

  //! call c function of type void(*)(void)
  static
  JSBool
  ejs_TCCState_callVV
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_EJS_TCCSTATE_OBJ;

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    
    lastError=NULL;

    typedef void (*void_func_void) (void);
    long unsigned int addr;
    if (tcc_get_symbol(tccState, &addr, ctype))
      EJS_THROW_ERROR(cx, obj, "Couldn't get symbol");
    void_func_void func;
    func=(void_func_void)addr;
    func();

    return JS_TRUE;
  }

  //! call c wrapper function
  static
  JSBool
  ejs_TCCState_call
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_EJS_TCCSTATE_OBJ;

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    
    lastError=NULL;

    typedef JSBool (*wrapper_func) (JSContext*, JSObject*, uintN, jsval*, jsval*);
    long unsigned int addr;
    if (tcc_get_symbol(tccState, &addr, ctype))
      EJS_THROW_ERROR(cx, obj, "Couldn't get symbol");
    wrapper_func func;
    func=(wrapper_func)addr;
    return func(cx,obj,argc-1,argv+1,rval);
  }

  static
  void
  ejs_TCCState_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &ejs_TCCState_class);
    TCCState* tccState=(TCCState *)JS_GetPrivate(cx,obj);
    if (!tccState) return;
    tcc_delete(tccState);
  }

  static
  JSBool
  ejs_TCCState_cons
  (JSContext *cx, JSObject *obj, uintN, jsval *, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // called as function f.e. x=Test() - we act like x=new Test()
      // todo do we really want this?
      if (!(obj=JS_NewObject(cx,&ejs_TCCState_class,NULL,NULL))) return JS_FALSE;
      *rval=OBJECT_TO_JSVAL(obj);
    }
    TCCState* tccState = tcc_new();
    if (!tccState) EJS_THROW_ERROR(cx, obj, "Couldn't create tcc state");
    tcc_set_error_func(tccState, NULL, ejstcc_onerror);
    tcc_set_output_type(tccState, TCC_OUTPUT_MEMORY);
    return JS_SetPrivate(cx,obj,(void *)tccState);
  }

#undef GET_EJS_TCCSTATE_OBJ

#define FUNC(name, args) { #name,ejs_TCCState_##name,args,0,0}

  static JSFunctionSpec ejs_TCCState_methods[] = {
    FUNC(compile,1),
    FUNC(relocate,0),
    FUNC(callVV,1),
    FUNC(call,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC


  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejstcc_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    if (!JS_InitClass(cx, module,
		      NULL,
		      &ejs_TCCState_class,
		      ejs_TCCState_cons, 0,
		      NULL, ejs_TCCState_methods,
		      NULL, NULL)) return JS_FALSE;

    return JS_TRUE;
  }
  
  JSBool
  ejstcc_LTX_onUnLoad(JSContext *cx, JSObject *module)
  {
    return JS_TRUE;
  }
#ifdef __cplusplus
}
#endif
