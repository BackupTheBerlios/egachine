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

/*
  parts are taken from spidermonkey example shell js.c
   

  * Version: MPL 1.1/GPL 2.0/LGPL 2.1
  *
  * The contents of this file are subject to the Mozilla Public License Version
  * 1.1 (the "License"); you may not use this file except in compliance with
  * the License. You may obtain a copy of the License at
  * http://www.mozilla.org/MPL/
  *
  * Software distributed under the License is distributed on an "AS IS" basis,
  * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
  * for the specific language governing rights and limitations under the
  * License.
  *
  * The Original Code is Mozilla Communicator client code, released
  * March 31, 1998.
  *
  * The Initial Developer of the Original Code is
  * Netscape Communications Corporation.
  * Portions created by the Initial Developer are Copyright (C) 1998
  * the Initial Developer. All Rights Reserved.
  *
  * Contributor(s):
  *
  * Alternatively, the contents of this file may be used under the terms of
  * either of the GNU General Public License Version 2 or later (the "GPL"),
  * or the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
  * in which case the provisions of the GPL or the LGPL are applicable instead
  * of those above. If you wish to allow use of your version of this file only
  * under the terms of either the GPL or the LGPL, and not to allow others to
  * use your version of this file under the terms of the MPL, indicate your
  * decision by deleting the provisions above and replace them with the notice
  * and other provisions required by the GPL or the LGPL. If you do not delete
  * the provisions above, a recipient may use your version of this file under
  * the terms of any one of the MPL, the GPL or the LGPL.
  *
  */

/*!
  \brief Util module
  \author Jens Thiele
   
*/

#include <iostream>
#include <ejsmodule.h>
#include <cassert>

#define HAVE_RLIMIT 1

#ifdef HAVE_RLIMIT
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>
#endif

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

  //! garbage collection
  /*!
    \return array [before, after, break]
  */
  
  static
  JSBool
  GC
  (JSContext *cx, JSObject *obj, uintN, jsval *, jsval *rval)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

#if 0
    JSRuntime *rt = JS_GetRuntime(cx);
    assert(rt);
    uint32 preBytes;
    preBytes = rt->gcBytes;
#endif

    JS_GC(cx);

#if 0
    // return some statistics
    // disabled because we can't do this with the public API
    // we would have to include jscntxt.h which in turn
    // includes private headers not shipped with the debian libsmjs-dev
    // package

    uint32 afterBytes = rt->gcBytes;
    uint32 brk=
#ifdef XP_UNIX
      (uint32)sbrk(0)
#else
      0
#endif
      ;
      
    JSObject *nobj=JS_NewArrayObject(cx, 0, NULL);
    if (!nobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(nobj);

    jsval n;

    if ((!JS_NewNumberValue(cx, preBytes, &n))
	|| (!JS_SetElement(cx, nobj, 0, &n))) return JS_FALSE;

    if ((!JS_NewNumberValue(cx, afterBytes, &n))
	|| (!JS_SetElement(cx, nobj, 1, &n))) return JS_FALSE;

    if ((!JS_NewNumberValue(cx, brk, &n))
	|| (!JS_SetElement(cx, nobj, 2, &n))) return JS_FALSE;
#endif

    return JS_TRUE;
  }

  static
  JSBool
  maybeGC
  (JSContext *cx, JSObject *obj, uintN, jsval *, jsval *)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

    JS_MaybeGC(cx);
    return JS_TRUE;
  }

  //! seal object (taken from spidermonkey js.c)
  static JSBool
  seal(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

    JSObject *target;
    JSBool deep = JS_FALSE;
    
    if (!JS_ConvertArguments(cx, argc, argv, "o/b", &target, &deep))
      return JS_FALSE;
    if (!target)
      return JS_TRUE;
    return JS_SealObject(cx, target, deep);
  }

  //! cloneFunction (taken from spidermonkey js.c - clone)
  static JSBool
  cloneFunction(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

    JSFunction *fun;
    JSObject *funobj, *parent, *clone;
    
    fun = JS_ValueToFunction(cx, argv[0]);
    if (!fun)
      return JS_FALSE;
    funobj = JS_GetFunctionObject(fun);
    if (argc > 1) {
      if (!JS_ValueToObject(cx, argv[1], &parent))
	return JS_FALSE;
    } else {
      parent = JS_GetParent(cx, funobj);
    }
    clone = JS_CloneFunctionObject(cx, funobj, parent);
    if (!clone)
      return JS_FALSE;
    *rval = OBJECT_TO_JSVAL(clone);
    return JS_TRUE;
  }

  //! clearScope (taken from spidermonkey js.c)
  static JSBool
  clearScope(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

    if (argc != 0 && !JS_ValueToObject(cx, argv[0], &obj))
      return JS_FALSE;
    JS_ClearScope(cx, obj);
    return JS_TRUE;
  }

  //! get or set js version
  static JSBool
  ejs_JSVersion(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    // todo: safe for untrusted code?
    EJS_CHECK_TRUSTED(cx,obj);

    JSVersion ret;
    JSString *s=NULL;
    if (argc > 0) {
      if (!(s=JS_ValueToString(cx,argv[0]))) return JS_FALSE;
      JSVersion nv=JS_StringToVersion(JS_GetStringBytes(s));
      EJS_INFO(nv);
      ret=JS_SetVersion(cx, nv);
    }else
      ret=JS_GetVersion(cx);
    if (!(s=JS_NewStringCopyZ(cx,JS_VersionToString(ret))))
      return JS_FALSE;
    *rval = STRING_TO_JSVAL(s);
    return JS_TRUE;
  }

#ifdef HAVE_RLIMIT
  //! set memory usage limit
  static JSBool
  setMemoryLimit(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    jsdouble nlimit;
    if (!JS_ValueToNumber(cx,argv[0],&nlimit)) return JS_FALSE;
    rlimit limit;
    limit.rlim_cur=limit.rlim_max=nlimit;
    *rval=BOOLEAN_TO_JSVAL((setrlimit(RLIMIT_AS,&limit)==0));
    return JS_TRUE;
  }
#endif
  
#define FUNC(name, args) { #name,name,args,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(getObjectID,0),
    FUNC(isCompilableUnit,1),
    FUNC(GC,0),
    FUNC(maybeGC,0),
    {"seal",            seal,           1, 0, 1},
    FUNC(cloneFunction,1),
    FUNC(clearScope,1),
    {"JSVersion",       ejs_JSVersion,  0, 0, 0},
#ifdef HAVE_RLIMIT
    FUNC(setMemoryLimit,1),
#endif
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
