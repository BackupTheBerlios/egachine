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
   \file common/ecmascript.cpp
   \brief 
   \author Jens Thiele
*/

/* include the JS engine API header */
#include "ecmascript.h"
#include <assert.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include "error.h"
#include <sstream>

extern "C" {
  ECMA_BEGIN_VOID_FUNC(print) {
    ECMA_CHECK_NUM_ARGS(1);
    JSString *s=JS_ValueToString(cx, argv[0]);
    if (!s) return JS_FALSE;
    // root the string
    argv[0]=STRING_TO_JSVAL(s);
    std::cout << JS_GetStringBytes(s) << std::endl;
    return JS_TRUE;
  }
  
  static
  void printError(JSContext *, const char *message, JSErrorReport *report) {
    std::cerr << "JS Error in " << (report->filename ? report->filename : "NULL") 
	      << " on line " << report->lineno << ": " << message << std::endl;
  }

  //! hash value for object 
  /*!
    TODO: perhaps we should not expose such internals for security reasons
    perhaps redirect/randomize/remove some bits
    but we have to return always the same hash value for the same object!
  */
  ECMA_BEGIN_FUNC(hashObject) 
  {
    ECMA_CHECK_NUM_ARGS(1);
    if (!JSVAL_IS_OBJECT(argv[0])) ECMA_ERROR("object required as argument");
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
}

namespace ECMAScript
{
  JSRuntime *rt;
  JSContext *cx;
  JSObject  *glob;
  JSClass global_class = {
    "global",0,
    JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
    ECMA_END_CLASS_SPEC
  };
  
  int
  eval(std::istream &in,const char* resname)
  {
    std::string script;
    char c;
    while (in.get(c)) script+=c;
    
    jsval rval;
    if (!JS_EvaluateScript(ECMAScript::cx, ECMAScript::glob, script.c_str(), script.length(),resname, 1, &rval)) {
      std::cout << "script execution failed" << std::endl;
      return 2;
    }
    return 0;
  }
  
  bool init() 
  {
    /* initialize the JS run time, and return result in rt */
    if (!(rt = JS_NewRuntime(8L * 1024L * 1024L))) return false;
    
    /* establish a context */
    if (!(cx = JS_NewContext(rt, 2<<13))) return false;
    
    /* create the global object here */
    if (!(glob = JS_NewObject(cx, &global_class, NULL, NULL))) return false;
    
    /* initialize the built-in JS objects and the global object */
    if (!JS_InitStandardClasses(cx, glob)) return false;
    
    JS_SetErrorReporter(cx, printError);
    JS_DefineFunction(cx, glob, "print", print, 2, 0);
    JS_DefineFunction(cx, glob, "hashObject", hashObject, 2, 0);
    return true;
  }

  void copyargv(int argc, char** argv) 
  {
    // copy argument vector to interpreter
    // TODO: make this in a cleaner way
    // we now have to be careful with the GC
    int js_argc=argc-1;
    if (js_argc<0) js_argc=0;
    jsval v[js_argc];
    JSString* jsstr[js_argc];
    for (int i=1;i<argc;++i) {
      jsstr[i-1]=JS_NewStringCopyZ(ECMAScript::cx, argv[i]);
      JGACHINE_CHECK(jsstr[i-1]);
      JGACHINE_CHECK(JS_AddRoot(ECMAScript::cx,jsstr[i-1]));
      v[i-1]=STRING_TO_JSVAL(jsstr[i-1]);
    }
    JSObject *nobj=JS_NewArrayObject(ECMAScript::cx, js_argc, v);
    JGACHINE_CHECK(nobj);
    JGACHINE_CHECK(JS_AddRoot(ECMAScript::cx,nobj));
    for (int i=1;i<argc;++i)
      JGACHINE_CHECK(JS_RemoveRoot(ECMAScript::cx,jsstr[i-1]));
    JGACHINE_CHECK(JS_DefineProperty(ECMAScript::cx,ECMAScript::glob,"argv",OBJECT_TO_JSVAL(nobj),NULL,NULL,JSPROP_ENUMERATE));
    JGACHINE_CHECK(JS_RemoveRoot(ECMAScript::cx,nobj));
  }


  void
  setVersion(const char* varname)
  {
#ifndef PACKAGE_VERSION
#error PACKAGE_VERSION must be defined
#endif
    JGACHINE_CHECK(varname);
    std::ostringstream o;
    o << varname << "={string:\""PACKAGE_VERSION"\","
      << "major:" JGACHINE_XSTR(PACKAGE_MAJOR_VERSION) ","
      << "minor:" JGACHINE_XSTR(PACKAGE_MINOR_VERSION) ","
      << "micro:" JGACHINE_XSTR(PACKAGE_MICRO_VERSION) "};\n";
    std::istringstream i(o.str());
    ECMAScript::eval(i);
  }
  

  void deinit() 
  {
    if (cx) {
      JS_GC(cx);
      JS_DestroyContext(cx);
      cx = NULL;
    }
    
    /* Before exiting the application, free the JS run time */
    if (rt) {
      JS_DestroyRuntime(rt);
      rt = NULL;
    }
    JS_ShutDown();
  }
}

