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
  \brief extensible javascript shell
  \author Jens Thiele
*/

#include <iostream>
#include <string>
#include <fstream>

#include "macros.h"
#include "ejsmoduleloader.h"
#include "modules/ejsmodule.h"

extern "C" {

  //! error reporter we register with spidermonkey
  static
  void
  printError(JSContext *cx, const char *message, JSErrorReport *report) {
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

    jsval val;
    if (JS_GetPendingException(cx,&val) && JSVAL_IS_OBJECT(val)) {
      JSObject* obj=JSVAL_TO_OBJECT(val);
      jsval stack;
      if (JS_GetProperty(cx, obj, "stack", &stack)) {
	// print exception.stack
	
	JSString *s;
	if (!(s=JS_ValueToString(cx, stack))) {
	  EJS_WARN("Could not convert exception to string");
	  return;
	}
	// root the string
	if (!JS_AddRoot(cx,s)) {
	  EJS_WARN("Could not root string");
	  return;
	}
	
	// todo: we loose unicode information here
	char* ctype=JS_GetStringBytes(s);
	if (!ctype) {
	  EJS_WARN("Could not get string bytes");
	  EJS_CHECK(JS_RemoveRoot(cx,s));
	  return;
	}
	unsigned len=JS_GetStringLength(s);
	// todo somehow do nice indent
	std::cerr << "Stack:" << std::endl;
	std::cerr.rdbuf()->sputn(ctype,len);
	std::cerr << std::endl;
	EJS_CHECK(JS_RemoveRoot(cx,s));
      }
    }
  }

  static
  JSClass global_class = {
    "global",0,
    JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
    JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
    EJS_END_CLASS_SPEC
  };
}

//! the program
/*!
  \note we use a struct/class to make sure everything is always deinititalized (by the destructor)
*/
struct EJSShell
{
  JSRuntime *rt;
  JSContext *cx;
  JSObject  *glob;

  int
  initSpiderMonkey()
  {
    if (!(rt = JS_NewRuntime(1L * 1024L * 1024L))) return EXIT_FAILURE;
    if (!(cx = JS_NewContext(rt, 2<<13))) return EXIT_FAILURE;
    if (!(glob = JS_NewObject(cx, &global_class, NULL, NULL))) return EXIT_FAILURE;
    if (!JS_InitStandardClasses(cx, glob)) return EXIT_FAILURE;
    JS_SetErrorReporter(cx, printError);
    return EXIT_SUCCESS;
  }


#define INIT_ERROR(msg) do{			\
    std::cerr << "ejs: " << msg << std::endl;	\
    return EXIT_FAILURE;			\
  }while(0)

  int
  main(int argc, char** argv)
  {
    if (initSpiderMonkey()==EXIT_FAILURE)
      INIT_ERROR("Could not initialize spidermonkey");

    if (!copyargv(cx,glob,argc,argv))
      INIT_ERROR("Could not pass command line arguments to interpreter");


    // create ejs scope object
    JSObject *ejs;
    if (!(ejs = JS_DefineObject(cx, glob, "ejs", NULL, NULL, JSPROP_ENUMERATE)))
      INIT_ERROR("could not define ejs scope object");


    // get moduleloader up
    
    if (!EJSModuleLoader::onLoad(cx,ejs))
      INIT_ERROR("Could not initialize module loader");


    // evaluate onstartup.js file

    const char* path[]={
#ifdef SYSCONFDIR
      SYSCONFDIR"/"PACKAGE"/",
#endif
      "src/etc/",
      "etc/",
      NULL
    };

    jsval result;

    unsigned i;
    for (i=0;path[i];++i) {
      std::string fname(path[i]);
      fname+="onstartup.js";
      std::ifstream in(fname.c_str());
      if (in.good()) {
	if (!EJSModuleLoader::evaluateScript(cx, glob, fname.c_str(), &result))
	  INIT_ERROR("Error during execution of startup file");
	break;
      }
    }
    if (!path[i]) INIT_ERROR("could not find startup file: 'onstartup.js'");


    // now we are ready to run scripts

    if (argc>=2) {
      EJS_CHECK(argv[1]);
    }
    
    if (argc>=2) {
      std::string arg(argv[1]);
      if ((arg=="-h")||(arg=="--help")) {
	printUsage();
	return EXIT_SUCCESS;
      }
    }
    
    if ((argc<2)||(std::string(argv[1])=="-")) {
      if (!EJSModuleLoader::evaluateScript(cx,glob,std::cin.rdbuf(),"<stdin>",&result))
	return EXIT_FAILURE;
      return EXIT_SUCCESS;
    }
    
    if (!EJSModuleLoader::evaluateScript(cx,glob,argv[1],&result))
      return EXIT_FAILURE;
    return EXIT_SUCCESS;
  }

#undef INIT_ERROR
  
  //! copy argument vector to interpreter
  static
  JSBool
  copyargv(JSContext* cx, JSObject* obj, int argc, char** argv)
  {
    // TODO: make this in a cleaner way (we should remove the roots on error)
    // be careful with the GC
    int js_argc=argc-1;
    if (js_argc<0) js_argc=0;
    jsval v[js_argc];
    JSString* jsstr[js_argc];
    for (int i=1;i<argc;++i) {
      if (!(jsstr[i-1]=JS_NewStringCopyZ(cx, argv[i]))) return JS_FALSE;
      if (!JS_AddRoot(cx,jsstr[i-1])) return JS_FALSE;
      v[i-1]=STRING_TO_JSVAL(jsstr[i-1]);
    }
    JSObject* nobj;
    if (!(nobj=JS_NewArrayObject(cx, js_argc, v))) return JS_FALSE;
    if (!JS_AddRoot(cx,nobj)) return JS_FALSE;
    for (int i=1;i<argc;++i)
      if (!JS_RemoveRoot(cx,jsstr[i-1])) return JS_FALSE;
    if (!JS_DefineProperty(cx,obj,"argv",OBJECT_TO_JSVAL(nobj),NULL,NULL,JSPROP_ENUMERATE)) return JS_FALSE;
    if (!JS_RemoveRoot(cx,nobj)) return JS_FALSE;
    return JS_TRUE;
  }

  static
  void
  printUsage()
  {
    std::cerr << "Usage: ejs [-h|--help|-|FILE] [OPTION]...\n";
  }

  ~EJSShell()
  {
    if (!EJSModuleLoader::onUnLoad())
      EJS_WARN("EJSModuleLoader::onUnLoad() failed");

    if (cx) {
      JS_DestroyContext(cx);
      cx = NULL;
    }
    
    if (rt) {
      JS_DestroyRuntime(rt);
      rt = NULL;
    }
    JS_ShutDown();
  }
};

int
main(int argc, char** argv)
{
  EJSShell ejs;
  return ejs.main(argc,argv);
}
