#include <iostream>
#include "jscpp.h"

using namespace JSCPP;

struct JS
{
  JS(JSContext *_cx, JSObject *_obj, uintN _argc, jsval *_argv, jsval *_rval)
    : cx(Context::object(_cx)), obj(_obj), argc(_argc), argv(_argv), rval(_rval)
  {}
  
  Context& cx;
  JSObject *obj;
  uintN argc;
  jsval *argv;
  jsval *rval;
};

struct print
{
  void apply(JS &js)
  {
    if (js.argc!=1) throw WrapError("Wrong number of arguments");
    Array<const char> array;
    js.cx.fromJS(js.argv[0],array);
    std::cout.rdbuf()->sputn(array.data,array.len);
  }
};

int sqr(int x)
{
  return x*x;
}

struct wrapSqr
{
  void apply(JS &js)
  {
    if (js.argc!=1) throw WrapError("Wrong number of arguments");
    int x;
    js.cx.fromJS(js.argv[0],x);
    *js.rval=js.cx.toJS(sqr(x));
  }
};

template <typename F>
static
JSBool
wrap(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
{
  JS js(cx,obj,argc,argv,rval);
  try{
    F f;
    f.apply(js);
    return JS_TRUE;
  }catch(const JSError&){
    return JS_FALSE;
  }catch(const WrapError& error){
    try{
      js.cx.jsThrow(error);
    }catch(const JSError&){
    }
    return JS_FALSE;
  }
}

int main()
{
  try{
    Runtime rt;
    Context cx(rt);
    cx.defineFunction("print", wrap<print>, 2, 0);
    cx.defineFunction("sqr", wrap<wrapSqr>, 2, 0);
    std::string script;
    char c;
    while (std::cin.get(c)) script+=c;
    jsval rval;
    cx.evaluateScript(script.c_str(), script.length(), "stdin", 1, &rval);
    return EXIT_SUCCESS;
  }catch(...){
    return EXIT_FAILURE;
  }
}
