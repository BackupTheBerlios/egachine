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
   \brief Timer class wrapper
   \author Jens Thiele
*/

#include <ejsmodule.h>
#include "timestamp.h"

extern "C" {
  static
  JSBool
  getTimeStamp(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
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

  static
  JSBool
  uSleep(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
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

#define FUNC(name, args) { #name,name,args,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(getTimeStamp,0),
    FUNC(uSleep,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejstimer_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    Timer::init();
    return JS_DefineFunctions(cx, module, static_methods);
  }

  JSBool
  ejtimer_LTX_onUnLoad()
  {
    Timer::deinit();
    return JS_TRUE;
  }
}
