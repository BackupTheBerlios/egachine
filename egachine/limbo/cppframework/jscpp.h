#ifndef JSCPP_H
#define JSCPP_H

#include <jsapi.h>
#include <cassert>
#include <stdexcept>

namespace JSCPP
{
  //! exception thrown if a JS_ function returned with an error
  struct JSError : public std::exception
  {
    const char* what() const throw()
    {
      return "JS error";
    }
  };
  
  //! exception you should throw on error in your wrappers
  struct WrapError : public std::runtime_error
  {
    WrapError(const std::string &arg) : std::runtime_error(arg)
    {}
  };

  static
  void printError(JSContext *, const char *message, JSErrorReport *report) {
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
  
  struct Runtime
  {
    static int runtimes;

    Runtime(uint32 maxbytes=4*1024*1024) 
    {
      rt=JS_NewRuntime(maxbytes);
      if (rt) ++runtimes;
    }
    ~Runtime()
    {
      if (rt) {
	JS_DestroyRuntime(rt);
	rt = NULL;
	if (!(--runtimes))
	  JS_ShutDown();
      }
    }
    
    JSRuntime *rt;
  };
  int Runtime::runtimes=0;

  template <typename E>
  struct Array
  {
    Array<E>(const char* _data, unsigned _len)
      : data(_data), len(_len)
    {}
    Array<E>()
      : data(NULL), len(0)
    {}
    
    E* data;
    unsigned len;
  };
  
  struct Context
  {
    struct Root
    {
      Root(Context& _cx, void* _val) : cx(_cx), val(_val)
      {
	if (!JS_AddRoot(cx.cx,val)) throw JSError();
      }
      
      ~Root()
      {
	if (!JS_RemoveRoot(cx.cx,val)) throw JSError();
      }
      
      Context &cx;
      void* val;
    };

    static
    Context& object(JSContext* cx)
    {
      Context* res=(Context *)JS_GetContextPrivate(cx);
      assert(res);
      return *res;
    }
    
    Context(JSCPP::Runtime &rt, size_t stackSize=2<<13)
    {
      if (!(cx=JS_NewContext(rt.rt, stackSize))) throw JSError();
      JS_SetContextPrivate(cx,(void *)this);
      glob=newObject();
      initStandardClasses();
      setErrorReporter(printError);
      const char* script="function jsThrow(error){throw error;};";
      // todo: this returns a function
      // => we could use the rval to call jsThrow and not call it by name
      jsval rval;
      evaluateScript(script,strlen(script),"Context",1,&rval);
    }

    ~Context()
    {
      if (cx) {
	JS_DestroyContext(cx);
	cx = NULL;
      }
    }
    
    JSContext *cx;
    JSObject  *glob;

    void
    initStandardClasses(JSObject *obj=NULL)
    {
      if (!obj) obj=glob;
      if (!JS_InitStandardClasses(cx,obj)) throw JSError();
    }

    void
    setErrorReporter(JSErrorReporter er)
    {
      JS_SetErrorReporter(cx,er);
    }
    
    JSFunction *
    defineFunction(JSObject *obj, const char *name, JSNative call,
		   uintN nargs, uintN attrs)
    {
      return JS_DefineFunction(cx,obj,name,call,nargs,attrs);
    }

    JSFunction *
    defineFunction(const char *name, JSNative call,
		   uintN nargs, uintN attrs)
    {
      JSFunction* res=defineFunction(glob,name,call,nargs,attrs);
      if (!res) throw JSError();
      return res;
    }
    
    void
    evaluateScript(JSObject *obj,
		   const char *bytes, uintN length,
		   const char *filename, uintN lineno,
		   jsval *rval)
    {
      if (!JS_EvaluateScript(cx,obj,bytes,length,filename,lineno,rval))
	throw JSError();
    }

    void
    evaluateScript(const char *bytes, uintN length,
		   const char *filename, uintN lineno,
		   jsval *rval)
    {
      evaluateScript(glob,bytes,length,filename,lineno,rval);
    }

    void
    callFunctionName(JSObject *obj, const char *name, uintN argc,
		     jsval *argv, jsval *rval)
    {
      if (JS_CallFunctionName(cx,obj,name,argc,argv,rval)!=JS_TRUE)
	throw JSError();
    }
    
    void
    callFunctionName(const char *name, uintN argc,
		     jsval *argv, jsval *rval)
    {
      callFunctionName(glob,name,argc,argv,rval);
    }

    void
    jsThrow(const WrapError &error)
    {
      // todo: this could throw an exception
      jsval jserror=toJS(error);
      jsval rval;
      // this will throw an JSError
      callFunctionName("jsThrow",1,&jserror,&rval);
    }
    
    JSObject*
    newObject(JSClass *clasp=NULL, JSObject *proto=NULL, JSObject *parent=NULL)
    {
      JSObject* res=JS_NewObject(cx,clasp,proto,parent);
      if (!res) throw JSError();
      return res;
    }
    
    void
    fromJS(jsval in, int32 &out) 
    {
      if (JS_ValueToInt32(cx,in,&out)==JS_FALSE) throw JSError();
    }

    void
    fromJS(jsval in, double &out)
    {
      if (JS_ValueToNumber(cx,in,&out)==JS_FALSE) throw JSError();
    }

    void
    fromJS(jsval in, Array<const char> &out)
    {
      JSString *s=JS_ValueToString(cx, in);
      if (!s) throw JSError();
      Root root(*this,(void *)s);
      out.data=JS_GetStringBytes(s);
      if (!out.data) throw JSError();
      out.len=JS_GetStringLength(s);
    }

    jsval toJS(int32 in) 
    {
      jsval r;
      if (JS_NewNumberValue(cx,in,&r)==JS_FALSE) throw JSError();
      return r;
    }

    jsval toJS(const char* str)
    {
      JSString* s=JS_NewStringCopyZ(cx, str);
      if (!s) throw JSError();
      return STRING_TO_JSVAL(s);
    }
    
    jsval toJS(const WrapError &error)
    {
      jsval str=toJS(error.what());
      jsval rval;
      callFunctionName("Error",1,&str,&rval);
      return rval;
    }
  };
}


#endif
