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
   \file common/ecmascript.h
   \brief 
   \author Jens Thiele
*/

#ifndef EGACHINE_ECMASCRIPT_H
#define EGACHINE_ECMASCRIPT_H

/* include the JS engine API header */
#include <jsapi.h>
#include <istream>

namespace ECMAScript
{
  extern JSRuntime *rt;
  extern JSContext *cx;
  extern JSObject  *glob;
  
  bool init();
  void deinit();

  //! if there are pending exceptions print stacktrace and clear them
  void
  handleExceptions();

  bool
  eval(jsval &rval, const char* script, unsigned scriptlen, const char* resname=NULL);

  //! read script from stream and evaluate it
  /*!
    \param rval the value returned from the script
    \param in the stream to read the script from
    \param resname resource associated with the script - used for error reporting

    \return true on success, false on error
  */
  bool
  eval(jsval &rval, std::istream &in,const char* resname=NULL);

  bool
  eval(std::istream &in,const char* resname=NULL);

  //! evaluate expression returning a number which fits into an int32
  int32
  evalInt32(const char* expression);

  //! call a member function/property by name
  JSBool
  callFunction(jsval &rval, const char *objname, const char* fname, jsuint argc=0, jsval* argv=NULL);
  
  //! call a member function/property by name
  JSBool
  callFunction(const char* objname, const char* fname, jsuint argc=0, jsval* argv=NULL);

  void
  copyargv(int argc, char** argv);

  void
  setVersion(const char* varname);

  void
  parseConfig(const char* config);

  void
  parseLib(const char* config);

  JSBool
  jsThrow(JSContext *cx, const char* msg);

  struct CallbackError
  {};
};

// macros to help with writing wrappers

//! start wrapper function not returning a value
#define ECMA_BEGIN_VOID_FUNC(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *argv, jsval *)

//! start wrapper function not getting any arguments
#define ECMA_BEGIN_FUNC_VOID(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *, jsval *rval)

//! start wrapper function not getting any arguments
#define ECMA_BEGIN_VOID_FUNC_VOID(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *, jsval *)

//! start wrapper function
#define ECMA_BEGIN_FUNC(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *argv, jsval *rval)

//! start wrapper method
#define ECMA_BEGIN_VOID_METHOD_VOID(name) static JSBool name (JSContext *cx, JSObject *obj, uintN argc, jsval *, jsval *)

//! start wrapper method
#define ECMA_BEGIN_METHOD(name) static JSBool name (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)

//! check number of arguments (only usable in wrapper function - see ECMA_BEGIN_FUNC(name) )
//#define ECMA_CHECK_NUM_ARGS(numargs) do{if(argc!=numargs) {JS_ReportError(cx,"Wrong number of arguments: expect %d, got %d",numargs,argc);return JS_FALSE;}}while(0)
#define ECMA_CHECK_NUM_ARGS(numargs) do{if(argc!=numargs) ECMA_THROW_ERROR("Wrong number of args");}while(0)

//! report error (only usable in wrapper function - see ECMA_BEGIN_FUNC(name) )
#define ECMA_THROW_ERROR(msg) do{return ECMAScript::jsThrow(cx,msg);}while(0)

//! report error (only usable in wrapper function and when a JS function returned JS_FALSE
#define ECMA_ERROR(msg) return JS_FALSE

//! function spec
#define ECMA_FUNCSPEC(name,numargs) { #name,name,numargs,0,0}

//! function spec with extra arguments
#define ECMA_FUNCSPEC_EXTRA(name,numargs,extraargs) { #name,name,numargs,0,extraargs}

//! end function spec array
#define ECMA_END_FUNCSPECS   {0,0,0,0,0}

#define ECMA_END_CLASS_SPEC 0,0,0,0,0,0,0,0

//! convert function arguments to floating point (jsdouble) array
#define ECMA_ARGS_TO_FLOAT_ARRAY(numargs,arrayname) ECMA_CHECK_NUM_ARGS(numargs);\
    jsdouble arrayname[numargs]; \
    for (int i=0;i<numargs;++i) \
      if (!JS_ValueToNumber(cx,argv[i],&(arrayname[i]))) \
	ECMA_FERROR("Argument %d is not a number",i)


#define ECMA_BEGIN_STATIC_VOID_FUNC_VOID(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *, jsval *)
#define ECMA_BEGIN_STATIC_VOID_FUNC(name) static JSBool name (JSContext *cx, JSObject *, uintN argc, jsval *argv, jsval *)

//! wrap a native function (without return value) in namespace ns without arguments
#define ECMA_VOID_FUNC_VOID(ns,name) ECMA_BEGIN_STATIC_VOID_FUNC_VOID(name){ECMA_CHECK_NUM_ARGS(0);ns::name();return JS_TRUE;}

//! wrap a native function (without return value) in namespace ns which takes 1 floating point argument(s) (jsdouble)
#define ECMA_VOID_FUNC_FLOAT1(ns,name) ECMA_BEGIN_STATIC_VOID_FUNC(name) { ECMA_ARGS_TO_FLOAT_ARRAY(1,d);ns::name(d[0]);return JS_TRUE;}

//! wrap a native function (without return value) in namespace ns which takes 2 floating point argument(s) (jsdouble)
#define ECMA_VOID_FUNC_FLOAT2(ns,name) ECMA_BEGIN_STATIC_VOID_FUNC(name) { ECMA_ARGS_TO_FLOAT_ARRAY(2,d);ns::name(d[0],d[1]);return JS_TRUE;}

//! wrap a native function (without return value) in namespace ns which takes 3 floating point argument(s) (jsdouble)
#define ECMA_VOID_FUNC_FLOAT3(ns,name) ECMA_BEGIN_STATIC_VOID_FUNC(name) { ECMA_ARGS_TO_FLOAT_ARRAY(3,d);ns::name(d[0],d[1],d[2]);return JS_TRUE;}

//! wrap a native function (without return value) in namespace ns which takes 4 floating point argument(s) (jsdouble)
#define ECMA_VOID_FUNC_FLOAT4(ns,name) ECMA_BEGIN_STATIC_VOID_FUNC(name) { ECMA_ARGS_TO_FLOAT_ARRAY(4,d);ns::name(d[0],d[1],d[2],d[3]);return JS_TRUE;}

// TODO: bug? - len correct? number of chars in the unicode string?
#define ECMA_STRING_TO_CHARVEC(val,ctype,len) do{if (!JSVAL_IS_STRING(val)) ECMA_THROW_ERROR("string required"); \
    JSString *strtype=JS_ValueToString(cx, val); \
    if (!strtype) return JS_FALSE; \
    // todo: we loose unicode information here \
    ctype=JS_GetStringBytes(strtype); \
    if (!ctype) return JS_FALSE; \
    len=JS_GetStringLength(strtype); \
  }while(0)

#endif
