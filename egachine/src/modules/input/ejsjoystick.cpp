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
   \file input/ejsjoystick.cpp
   \brief sdl joy wrapper
   \author Jens Thiele
*/

#include "ejsjoystick.h"
#include <cassert>
#include <SDL.h>

extern "C" {

  static
  void
  joystick_finalize(JSContext *cx, JSObject *obj);
  
  static
  JSClass joystick_class = {
    "Joystick",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  joystick_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  static
  JSBool
  ejsjoystick_GetNative(JSContext* cx, JSObject * obj, SDL_Joystick* &native)
  {
    EJS_CHECK_CLASS(cx, obj, joystick_class);
    native=(SDL_Joystick *)JS_GetPrivate(cx,obj);
    if (!native)
      EJS_THROW_ERROR(cx,obj,"no valid Joystick object");
    return JS_TRUE;
  }

#define GET_NTHIS(cx,obj) SDL_Joystick* nthis=NULL;		\
    if (!ejsjoystick_GetNative(cx,obj,nthis)) return JS_FALSE

  static
  JSBool
  joystick_numAxes(JSContext *cx, JSObject *obj, uintN, jsval *, jsval *rval)
  {
    GET_NTHIS(cx,obj);
    *rval=INT_TO_JSVAL(SDL_JoystickNumAxes(nthis));
    return JS_TRUE;
  }

#undef GET_NTHIS
  
#define FUNC(name, args) { #name,joystick_##name,args,0,0},
  
  static JSFunctionSpec joystick_methods[] = {
    FUNC(numAxes,0)
    EJS_END_FUNCTIONSPEC
  };
  
#undef FUNC
  
  static
  JSBool
  joystick_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) EJS_THROW_ERROR(cx,obj,"must be called as constructor");
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    
    int index;
    if (!JS_ValueToECMAInt32(cx, argv[0], &index)) return JS_FALSE;
    if ((index<0)||(index>=SDL_NumJoysticks())) EJS_THROW_ERROR(cx,obj,"out of range");
    
    SDL_Joystick* joy=SDL_JoystickOpen(index);
    if (!joy) EJS_THROW_ERROR(cx,obj,"could not open joy");
    if (!JS_SetPrivate(cx,obj,(void *)joy)) return JS_FALSE;
    return JS_TRUE;
  }
  
  static
  void
  joystick_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &joystick_class);
    SDL_Joystick* joystick=(SDL_Joystick *)JS_GetPrivate(cx,obj);
    if (!joystick) return;
    int index=SDL_JoystickIndex(joystick);
    if (SDL_JoystickOpened(index))
      SDL_JoystickClose(joystick);
  }
  
  JSBool
  ejsjoystick_onLoad(JSContext *cx, JSObject *input)
  {
    JSObject *joystick = JS_InitClass(cx, input,
				      NULL,
				      &joystick_class,
				      joystick_cons, 0,
				      NULL, joystick_methods,
				      NULL, NULL);
    if (!joystick) return JS_FALSE;
    return JS_TRUE;
  }
}
