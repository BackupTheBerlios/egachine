#include "ejstimer.h"

TimeManager::TimeManager(JSContext* _cx, JSObject* _jsTimer, float precision)
  : svgl::Time::Manager(precision), cx(_cx), jsTimer(_jsTimer)
{
}

void
TimeManager::firstTimerHasChanged()
{
  forward();
}

void
TimeManager::reschedule()
{
  forward();
}

void
TimeManager::forward()
{
  jsdouble elapsed;
  if(getTimers().empty()) {
    jsval rval;
    // todo: error handling
    if (!JS_CallFunctionName(cx, jsTimer, "reschedule", 0, NULL, &rval))
      EJS_WARN("js error");
    else{
      if (!JS_ValueToNumber(cx, rval, &elapsed))
	EJS_WARN("js error");
      else
	timeElapsed(elapsed);
    }
    return;
  }

  Timers::const_iterator i = getTimers().begin();
  jsval jssec;
  // todo what about errors ?
  if (!JS_NewNumberValue(cx, (*i)->getDelta(), &jssec))
    EJS_WARN("js error");
  else{
    bool gc=JSVAL_IS_GCTHING(jssec);
    if (gc && (!JS_AddNamedRoot(cx, JSVAL_TO_GCTHING(jssec), "jssec")))
      EJS_WARN("js error");
    else{
      jsval args[]={jssec};
      jsval rval;
      if (!JS_CallFunctionName(cx, jsTimer, "reschedule", 1, args, &rval))
	EJS_WARN("js error");
      else{
	if (!JS_ValueToNumber(cx, rval, &elapsed))
	  EJS_WARN("js error");
	else
	  timeElapsed(elapsed);
      }
      if (gc) JS_RemoveRoot(cx, JSVAL_TO_GCTHING(jssec));
    }
  }
}
