#include "jstime.h"
#include "ecmascript.h"
#include "time.h"

extern "C" {
  ECMA_BEGIN_FUNC_VOID(getTimeStamp) 
  {
    ECMA_CHECK_NUM_ARGS(0);
    Timer::TimeStamp usec=Timer::getTimeStamp();
    // TODO: we convert to double which might lead to trouble ;-)
    // it should not be a problem because time timestamps should be
    // small anyway:
    // l(60*60*24*1000000*365)/l(2)
    // 44.8420649150297469050581088654078025773
    // => after one year we use ~45bits
    // and it should be safe to use ~53bits
    return JS_NewNumberValue(cx,usec,rval);
  }

  ECMA_BEGIN_VOID_FUNC(uSleep)
  {
    ECMA_CHECK_NUM_ARGS(1);
    if (JSVAL_IS_INT(argv[0])) {
      Timer::uSleep(JSVAL_TO_INT(argv[0]));
      return JS_TRUE;
    }
    jsdouble d;
    if (!JS_ValueToNumber(cx,argv[0],&d))
      return JS_FALSE;
    // TODO: we convert from double which might lead to trouble ;-)
    // but we assume small values to sleep anyway
    Timer::uSleep(static_cast<Timer::TimeStamp>(d));
    return JS_TRUE;
  }
}

using namespace ECMAScript;

static JSFunctionSpec static_methods[] = {
  ECMA_FUNCSPEC(getTimeStamp,0),
  ECMA_FUNCSPEC(uSleep,1),
  ECMA_END_FUNCSPECS
};

static JSClass timer_class = {
  "Timer",0,
  JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
  JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
  ECMA_END_CLASS_SPEC
};

bool
JSTimer::init()
{
  JSObject *obj = JS_DefineObject(cx, glob, "Timer", &timer_class, NULL,
				  JSPROP_ENUMERATE);
  if (!obj) return JS_FALSE;
  return JS_DefineFunctions(cx, obj, static_methods);
}

bool
JSTimer::deinit()
{
  return true;
}
