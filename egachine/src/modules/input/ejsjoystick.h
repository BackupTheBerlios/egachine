#ifndef EJS_JOYSTICK_H
#define EJS_JOYSTICK_H

#include "ejsmodule.h"

extern "C" {
  JSBool
  ejsjoystick_onLoad(JSContext *cx, JSObject *input);
}
#endif
