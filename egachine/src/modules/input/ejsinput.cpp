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
  \file input/jsinput.cpp
  \brief Javascript input layer
  \author Jens Thiele
*/

#include <cassert>
#include "ejsmodule.h"
#include "ejsjoystick.h"
#include <SDL.h>



extern "C" {
  //! fetch input events
  /*!
    fetch input events and convert them to array of javascript objects
  */

#define SET_BOOL_PROP(event,name) do{					\
    jsval jstype=BOOLEAN_TO_JSVAL(event.name);				\
    if (!JS_SetProperty(cx, jseventobj, EJS_XSTR(name), &jstype))	\
      return JS_FALSE;							\
  }while(0)
  
#define SET_NUM_PROP(event,name) do{					\
    jsval jstype;							\
    if (!JS_NewNumberValue(cx, event.name, &jstype)) return JS_FALSE;	\
    if (!JS_SetProperty(cx, jseventobj, EJS_XSTR(name), &jstype))	\
      return JS_FALSE;							\
  }while(0)

  static
  JSBool
  getEvents(JSContext *cx, JSObject *, uintN, jsval *, jsval *rval)
  {
    JSObject * jsarray;
    if (!(jsarray=JS_NewArrayObject(cx, 0, NULL))) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(jsarray);
    SDL_Event event;
    unsigned i=0;
    while ( SDL_PollEvent(&event) ) {
      JSObject *jseventobj;
      if (!(jseventobj=JS_NewObject(cx, NULL, NULL, NULL))) return JS_FALSE;
      jsval jsevent=OBJECT_TO_JSVAL(jseventobj);
      if (!JS_SetElement(cx, jsarray, i, &jsevent)) return JS_FALSE;

      SET_NUM_PROP(event,type);

      switch (event.type) {
      case SDL_ACTIVEEVENT:
	SET_BOOL_PROP(event.active,gain);
	SET_NUM_PROP(event.active,state);
	break;
      case SDL_KEYDOWN:
      case SDL_KEYUP:
	SET_NUM_PROP (event.key,which);
	SET_BOOL_PROP(event.key,state);
	// platform dependent
	//	SET_NUM_PROP(event.key.keysym,scancode);
	SET_NUM_PROP (event.key.keysym,sym);
	SET_NUM_PROP (event.key.keysym,mod);
	SET_NUM_PROP (event.key.keysym,unicode);
	break;
      case SDL_MOUSEMOTION:
	SET_NUM_PROP (event.motion,which);
	SET_BOOL_PROP(event.motion,state);
	SET_NUM_PROP (event.motion,x);
	SET_NUM_PROP (event.motion,y);
	SET_NUM_PROP (event.motion,xrel);
	SET_NUM_PROP (event.motion,yrel);
	break;
      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
	SET_NUM_PROP (event.button,which);
	SET_NUM_PROP (event.button,button);
	SET_BOOL_PROP(event.button,state);
	SET_NUM_PROP (event.button,x);
	SET_NUM_PROP (event.button,y);
	break;
      case SDL_JOYAXISMOTION:
	SET_NUM_PROP (event.jaxis,which);
	SET_NUM_PROP (event.jaxis,axis);
	SET_NUM_PROP (event.jaxis,value);
	break;
      case SDL_JOYBALLMOTION:
	SET_NUM_PROP (event.jball,which);
	SET_NUM_PROP (event.jball,ball);
	SET_NUM_PROP (event.jball,xrel);
	SET_NUM_PROP (event.jball,yrel);
	break;
      case SDL_JOYHATMOTION:
	SET_NUM_PROP (event.jhat,which);
	SET_NUM_PROP (event.jhat,hat);
	SET_NUM_PROP (event.jhat,value);
	break;
      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
	SET_NUM_PROP (event.jbutton,which);
	SET_NUM_PROP (event.jbutton,button);
	SET_BOOL_PROP(event.jbutton,state);
      case SDL_QUIT:
	break;
      case SDL_VIDEORESIZE:
	SET_NUM_PROP (event.resize,w);
	SET_NUM_PROP (event.resize,h);
      case SDL_VIDEOEXPOSE:
	break;
      default:
	EJS_INFO("Got unknown event => we drop it ("<<unsigned(event.type)<<")");
      }
      ++i;
    }
    return JS_TRUE;
  }

  static
  JSBool
  enableKeyRepeat(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    int delay, interval;
    if (!JS_ValueToECMAInt32(cx, argv[0], &delay)) return JS_FALSE;
    if (!JS_ValueToECMAInt32(cx, argv[1], &interval)) return JS_FALSE;
    if (SDL_EnableKeyRepeat(delay, interval)!=0)
      EJS_THROW_ERROR(cx,obj,"failed");
    return JS_TRUE;
  }
  
  static
  JSBool
  enableUnicode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    int enable;
    if (!JS_ValueToECMAInt32(cx, argv[0], &enable)) return JS_FALSE;
    if ((enable<-1)||(enable>1)) EJS_THROW_ERROR(cx,obj,"value out of range");
    *rval=BOOLEAN_TO_JSVAL(SDL_EnableUNICODE(enable));
    return JS_TRUE;
  }

  static
  JSBool
  numJoysticks(JSContext *, JSObject *, uintN, jsval *, jsval *rval)
  {
    *rval=INT_TO_JSVAL(SDL_NumJoysticks());
    return JS_TRUE;
  }

#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(getEvents,0),
    FUNC(enableKeyRepeat,2),
    FUNC(enableUnicode,1),
    FUNC(numJoysticks,0),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsinput_LTX_onLoad(JSContext* cx, JSObject* global)
  {
    JSObject *obj = JS_DefineObject(cx, global,
				    "Input", NULL, NULL,
				    JSPROP_ENUMERATE);
    if (!obj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, obj, static_methods))
      return JS_FALSE;
    return ejsjoystick_onLoad(cx,obj);
  }
}
