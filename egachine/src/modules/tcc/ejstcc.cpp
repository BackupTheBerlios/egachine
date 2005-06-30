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


  static
  const char* lastError=NULL;

  static
  TCCState *tccState=NULL;

  static
  void ejstcc_onerror(void*, const char* msg)
  {
    lastError=msg;
  }

  //! compile c-code
  /*!
    \todo: fix possible memory leaks
  */
  static
  JSBool
  ejstcc_compile
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    EJS_CHECK(tccState);

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
  ejstcc_relocate
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    EJS_CHECK(tccState);

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
  ejstcc_call_vv
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    EJS_CHECK(tccState);

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    
    lastError=NULL;

    typedef void (*void_func_void) (void);
    long unsigned int addr;
    if (tcc_get_symbol(tccState, &addr, ctype))
      EJS_THROW_ERROR(cx, obj, "Could net get symbol");
    void_func_void func;
    func=(void_func_void)addr;
    func();

    return JS_TRUE;
  }

  //! call c wrapper function
  static
  JSBool
  ejstcc_call
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    EJS_CHECK(tccState);

    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    
    lastError=NULL;

    typedef JSBool (*wrapper_func) (JSContext*, JSObject*, uintN, jsval*, jsval*);
    long unsigned int addr;
    if (tcc_get_symbol(tccState, &addr, ctype))
      EJS_THROW_ERROR(cx, obj, "Could net get symbol");
    wrapper_func func;
    func=(wrapper_func)addr;
    return func(cx,obj,argc-1,argv+1,rval);
  }
  
  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejstcc_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    tccState = tcc_new();
    if (!tccState) EJS_THROW_ERROR(cx, module, "Could not create tcc state");
    tcc_set_error_func(tccState, NULL, ejstcc_onerror);
    tcc_set_output_type(tccState, TCC_OUTPUT_MEMORY);

    if (!JS_DefineFunction(cx,module,"compile",ejstcc_compile,0,0)) return JS_FALSE;
    if (!JS_DefineFunction(cx,module,"relocate",ejstcc_relocate,0,0)) return JS_FALSE;
    if (!JS_DefineFunction(cx,module,"call",ejstcc_call,0,0)) return JS_FALSE;
    if (!JS_DefineFunction(cx,module,"callVV",ejstcc_call_vv,0,0)) return JS_FALSE;
    
    return JS_TRUE;
  }
  
  JSBool
  ejstcc_LTX_onUnLoad(JSContext *cx, JSObject *module)
  {
    if (tccState) {
      tcc_delete(tccState);
      tccState = NULL;
    }
    return JS_TRUE;
  }
#ifdef __cplusplus
}
#endif
