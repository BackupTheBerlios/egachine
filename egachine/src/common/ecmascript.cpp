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
    // does not print the complete string if there is a \0 in it
    // std::cout << JS_GetStringBytes(s) << std::endl;

    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(s);
    if (!ctype) return JS_FALSE;
    unsigned len=JS_GetStringLength(s);
    std::cout.rdbuf()->sputn(ctype,len);
    return JS_TRUE;
  }
  
  static
  void printError(JSContext *cx, const char *message, JSErrorReport *report) {
    std::cerr << "JSERROR: "<< (report->filename ? report->filename : "NULL") << ":" << report->lineno << ":\n"
	      << "    " << message << "\n";
    if (report->linebuf) {
      std::string line(report->linebuf);
      if (line[line.length()-1]=='\n') line=line.substr(0,line.length()-1);
      std::cerr << "    \"" << line << "\"\n";
      if (report->tokenptr) {
	int where=report->tokenptr - report->linebuf;
	if ((where>=0)&&(where<80)) {
	  std::string ws(where+4,' ');
	  std::cerr << ws << "^\n";
	}
      }
    }
    std::cerr << "    Flags:";
    if (JSREPORT_IS_WARNING(report->flags)) std::cerr << " WARNING";
    if (JSREPORT_IS_EXCEPTION(report->flags)) std::cerr << " EXCEPTION";
    if (JSREPORT_IS_STRICT(report->flags)) std::cerr << " STRICT";
    std::cerr << " (Error number: " << report->errorNumber << ")\n";
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
    if (!JSVAL_IS_OBJECT(argv[0])) ECMA_THROW_ERROR("object required as argument");
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

static
JSClass global_class = {
  "global",0,
  JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
  JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
  ECMA_END_CLASS_SPEC
};

namespace ECMAScript
{
  JSRuntime *rt;
  JSContext *cx;
  JSObject  *glob;

  void
  handleExceptions()
  {
    if (JS_IsExceptionPending(ECMAScript::cx)) {
      jsval error;
      if (!JS_GetPendingException(ECMAScript::cx, &error)) {
	JGACHINE_ERROR("Could not get exception");
      }else{
	// todo: print stack trace
      }
      // we clear pending exceptions because we wish to use this context again
      JS_ClearPendingException(ECMAScript::cx);
    }
  }
  
  bool
  eval(jsval &rval, const char* script, unsigned scriptlen, const char* resname)
  {
    JGACHINE_CHECK(script);
    JGACHINE_CHECK(scriptlen>0);
    if (!JS_EvaluateScript(ECMAScript::cx, ECMAScript::glob, script, scriptlen, resname, 1, &rval)) {
      //      JGACHINE_WARN("script execution failed");
      handleExceptions();
      return false;
    }
    return true;
  }
  
  bool
  eval(jsval &rval, std::istream &in,const char* resname)
  {
    std::string script;
    char c;
    while (in.get(c)) script+=c;
    return eval(rval,script.c_str(),script.length(),resname);
  }

  bool
  eval(std::istream &in,const char* resname)
  {
    jsval rval;
    return eval(rval,in,resname);
  }

  int32
  evalInt32(const char* expression)
  {
    jsval rval;
    int32 res;
    std::ostringstream here;
    here << JGACHINE_HERE;
    JGACHINE_CHECK(ECMAScript::eval(rval, expression, strlen(expression),here.str().c_str())
		   &&JS_ValueToInt32(ECMAScript::cx, rval, &res));
    return res;
  }

  JSBool
  callFunction(jsval &rval, const char *objname, const char* fname, jsuint argc, jsval* argv)
  {
    JGACHINE_CHECK(objname);
    JGACHINE_CHECK(fname);
    if (argc>0) JGACHINE_CHECK(argv);
    
    jsval oval;
    if (!JS_GetProperty(ECMAScript::cx, ECMAScript::glob,objname,&oval))
      ECMA_ERROR("object does not exist");
    if (!JSVAL_IS_OBJECT(oval))
      ECMA_THROW_ERROR("not an object");

    JSObject *obj=JSVAL_TO_OBJECT(oval);
    if (obj==NULL) ECMA_THROW_ERROR("null pointer");
    if (!JS_CallFunctionName(ECMAScript::cx, obj, fname, argc, argv, &rval))
      ECMA_ERROR("error while calling function");
    return JS_TRUE;
  }

  JSBool
  callFunction(const char* objname, const char* fname, jsuint argc, jsval* argv)
  {
    jsval rval;
    return callFunction(rval,objname,fname,argc,argv);
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


  void
  parseConfig(const char* config)
  {
    JGACHINE_CHECK(config);
    bool gotsysconf=false;
#ifdef SYSCONFDIR
    {
      std::string c(JGACHINE_XSTR(SYSCONFDIR)"/egachine/");
      c+=config;
      std::ifstream sysin(c.c_str());
      if (sysin.good()) {
	gotsysconf=true;
	ECMAScript::eval(sysin,c.c_str());
      }
    }
#endif
    if (!gotsysconf) {
      // perhaps we are not installed - or run on a system
      // were SYSCONFDIR was not set (f.e. win32)
      std::string c("etc/");
      c+=config;
      std::ifstream sysin(c.c_str());
      if (sysin.good()) {
	gotsysconf=true;
	ECMAScript::eval(sysin,c.c_str());
      }
    }

    char* home=getenv("HOME");
    if (home) {
      std::string userconf(home);
      userconf+="/.egachine/";
      userconf+=config;
      std::ifstream userin(userconf.c_str());
      if (userin.good()) {
	ECMAScript::eval(userin,userconf.c_str());
      }
    }
  }

  void
  parseLib(const char* config)
  {
    JGACHINE_CHECK(config);
    bool gotsysconf=false;
#ifdef DATADIR
    {
      std::string c(JGACHINE_XSTR(DATADIR)"/egachine/");
      c+=config;
      std::ifstream sysin(c.c_str());
      if (sysin.good()) {
	gotsysconf=true;
	ECMAScript::eval(sysin,c.c_str());
      }
    }
#endif
    if (!gotsysconf) {
      // perhaps we are not installed - or run on a system
      // were DATADIR was not set (f.e. win32)
      std::ifstream sysin(config);
      if (sysin.good()) {
	gotsysconf=true;
	ECMAScript::eval(sysin,config);
      }else
	JGACHINE_FATAL((std::string("Could not find library file:")+config).c_str());
    }
  }

  // todo: this is still quite a hack
  JSBool
  jsThrow(JSContext *cx, const char* msg) 
  {
    // Attention: we do not root s
    // does JS_CallFunctionName root s for sure?
    JSString* s=JS_NewStringCopyZ(cx, msg);
    // todo:
    // this is not perfect - since it could be confusing to throw
    // an out of memory exception when there should be thrown a
    // different exception
    // on the other hand it is very unlikely that this ever happens
    if (!s) return JS_FALSE;
    jsval param[1];
    param[0]=STRING_TO_JSVAL(s);
    jsval rval;
    return JS_CallFunctionName(cx, ECMAScript::glob, "jsthrow", 1, param, &rval);
  }
}

