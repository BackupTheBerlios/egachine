#ifndef EJS_TIMER_H
#define EJS_TIMER_H

#include <ejsmodule.h>
#include <svgl/TimeManager.hpp>

//! class to forward timer manager to javascript
class TimeManager : public svgl::Time::Manager {
public:
  TimeManager(JSContext* _cx, JSObject* _jsTimer, float precision=0);
protected:
  virtual void firstTimerHasChanged();
  virtual void reschedule();

  void forward();

  JSContext* cx;
  JSObject* jsTimer;
};

#endif
