#define FUNC(name,args) static						\
  JSBool EJS_FUNC(name)							\
    (JSContext* cx, JSObject* jsthis, uintN argc, jsval* argv, jsval* rval) \
  {									\
    GET_NTHIS(cx,jsthis);						\
    return ejs_Node_##name(cx,jsthis,nthis,argc,argv,rval);		\
  }
#include "nodefuncs.h"
#undef FUNC
