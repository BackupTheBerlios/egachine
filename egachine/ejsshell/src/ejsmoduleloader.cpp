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
   \brief Javascript ModuleLoader object
   \author Jens Thiele
*/

#include <ltdl.h>
#include <map>
#include <fstream>
#include <string>

#include "ejsmoduleloader.h"
#include "macros.h"
#include "modules/ejsmodule.h"

static 
bool
untrusted=false;

#define SECURITY_CHECK(cx,obj) do{		\
    if(untrusted) EJS_THROW_ERROR(cx,obj, "Security exception: not allowed in untrusted mode"); \
  }while(0)
  

class Module
{
public:
  Module(const char* _name, const char* _filename=NULL)
  {
    name=_name;
    handle=lt_dlopenext(_filename ? _filename : _name);
    if (!handle)
      storeLTError();
    onLoad = (OnLoadFunc)getSymbol("onLoad");
    onUnLoad = (OnUnLoadFunc)getSymbol("onUnLoad");
    if (!isOk()) unLoad();
  }
  
  JSBool
  callOnLoad(JSContext *cx, JSObject *obj)
  {
    if (!isOk()) return JS_FALSE;
    if (!((*onLoad)(cx,obj))) {
      error="module onLoad failed";
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  const std::string&
  getError() const
  {
    return error;
  }

  ~Module()
  {
    unLoad();
  }

private:
  typedef int (*OnLoadFunc)(JSContext*, JSObject*);
  typedef int (*OnUnLoadFunc)(void);


  void*
  getSymbol(const char* _sname)
  {
    EJS_CHECK(_sname);
    if (!isOk()) return NULL;
    std::string sname(name);
    sname+="_LTX_";
    sname+=_sname;
    void* s=lt_dlsym(handle,sname.c_str());
    if (!s) storeLTError();
    //    EJS_INFO("Got symbol: "<<sname<<" at "<<s);
    return s;
  }
  
  bool
  isOk() const
  {
    return error.empty();
  }

  void
  unLoad() 
  {
    if (!handle) return;
    
    if (!onUnLoad||!(*onUnLoad)())
      EJS_ERROR("module onUnLoad failed");
    if (lt_dlclose(handle)) {
      const char *e=lt_dlerror();
      EJS_CHECK(e);
      EJS_ERROR(e);
    }
    handle = NULL;
  }
  
  void
  storeLTError()
  {
    const char *e=lt_dlerror();
    EJS_CHECK(e);
    error=e;
  }

  lt_dlhandle handle;
  std::string error;
  OnLoadFunc onLoad;
  OnUnLoadFunc onUnLoad;
  //! name of the module
  std::string name;
};

typedef std::map<std::string, Module* > Modules;
static Modules* modules=NULL;

extern "C" {

  //! convert Javascript string to C character array
  /*!
    \todo Javascript strings are 2 byte characters
  */
#define STRING_TO_CHARVEC(val, ctype, len) do {			\
    JSString *strtype=JS_ValueToString(cx, val);		\
    if (!strtype) return JS_FALSE;				\
    if (!(ctype=JS_GetStringBytes(strtype))) return JS_FALSE;	\
    len=JS_GetStringLength(strtype);				\
  }while(0)

  //! load native module (shared object / dll)
  /*!
     \note we expect obj (this) to point to a sensible object (probably the global
     object and not ModuleLoader object)
  */
  static
  JSBool
  ejs_moduleloader_loadNative(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    SECURITY_CHECK(cx,obj);
    EJS_CHECK(modules);
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    
    char* name;
    size_t len;
    STRING_TO_CHARVEC(argv[0],name,len);

    char* filename;
    STRING_TO_CHARVEC(argv[1],filename,len);

    if (modules->find(std::string(name))==modules->end()) {
      Module* module=new Module(name,filename);
      if (!module->callOnLoad(cx,obj)) {
	std::string error=module->getError();
	delete module;
	module=NULL;
	EJS_THROW_ERROR(cx,obj,error.c_str());
      }
      (*modules)[name]=module;
    }else{
      std::stringstream error;
      error << "Module '" << name << "' already loaded";
      EJS_THROW_ERROR(cx,obj,error.str().c_str());
    }
    return JS_TRUE;
  }

  static
  JSBool
  ejs_moduleloader_loadScript(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    SECURITY_CHECK(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    
    char* script;
    size_t len;
    STRING_TO_CHARVEC(argv[0],script,len);
    
    return EJSModuleLoader::evaluateScript(cx, obj, script, rval);
  }

  static
  JSBool
  ejs_moduleloader_disable(JSContext *, JSObject *, uintN, jsval *, jsval *)
  {
    untrusted=true;
    return JS_TRUE;
  }

  //! like loadScript but enters untrusted mode
  static
  JSBool
  ejs_moduleloader_loadUntrusted(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    SECURITY_CHECK(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    ejs_moduleloader_disable(cx,obj,argc,argv,rval);

    char* script;
    size_t len;
    STRING_TO_CHARVEC(argv[0],script,len);

    return EJSModuleLoader::evaluateScript(cx, obj, script, rval);
  }
  
  static
  JSBool
  ejs_moduleloader_fileIsReadable(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    SECURITY_CHECK(cx,obj);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    
    char* script;
    size_t len;
    STRING_TO_CHARVEC(argv[0],script,len);

    std::ifstream f(script);
    JSBool readable=f.good();
    *rval=BOOLEAN_TO_JSVAL(readable);
    return JS_TRUE;
  }
  
#define JSFUNC(name, args) { #name,ejs_moduleloader_##name,args,0,0}
  static JSFunctionSpec static_methods[] = {
    JSFUNC(loadNative,2),
    JSFUNC(loadScript,1),
    JSFUNC(loadUntrusted,1),
    JSFUNC(fileIsReadable,1),
    JSFUNC(disable,0),
    EJS_END_FUNCTIONSPEC
  };
#undef JSFUNC
}

JSBool
EJSModuleLoader::onLoad(JSContext *cx, JSObject *obj)
{
  EJS_CHECK(!modules);
  modules=new Modules();

  if (lt_dlinit()) return JS_FALSE;

  JSObject *newobj;
  if (!(newobj = JS_DefineObject(cx, obj, "ModuleLoader", NULL, NULL, JSPROP_ENUMERATE)))
    return JS_FALSE;

  return JS_DefineFunctions(cx, newobj, static_methods);
}

// todo: this is quite stupid
JSBool
EJSModuleLoader::evaluateScript(JSContext* cx, JSObject* obj, std::streambuf* src, const char* resname, jsval* rval) 
{
  typedef std::streambuf stream;
  std::string script;
  if (!src) EJS_THROW_ERROR(cx,obj,"invalid source stream");
  stream::char_type c;
  
  while ((c=src->sbumpc())!=stream::traits_type::eof())
    script+=stream::traits_type::to_char_type(c);
  return JS_EvaluateScript(cx, obj, script.c_str(), script.length(), resname, 1, rval);
}

JSBool
EJSModuleLoader::evaluateScript(JSContext* cx, JSObject* obj, const char* filename, jsval* rval) 
{
  if (!filename) EJS_THROW_ERROR(cx,obj,"filename required");
  std::ifstream in(filename);
  if (!in.good()) {
    std::string error("Could not open file: \"");
    error+=filename;
    error+='\"';
    EJS_THROW_ERROR(cx,obj,error.c_str());
  }
  return EJSModuleLoader::evaluateScript(cx,obj,in.rdbuf(),filename,rval);
}

JSBool
EJSModuleLoader::onUnLoad()
{
  // unload modules
  if (modules) {
    Modules::const_iterator it(modules->begin());
    while (it!=modules->end()){
      delete it->second;
      ++it;
    }
    delete modules;
    modules=NULL;
  }
  if (lt_dlexit()) return JS_FALSE;
  return JS_TRUE;
}
