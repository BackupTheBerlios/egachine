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
   \brief Monitorable object
   \author Jens Thiele
*/

#include <ejsmodule.h>

extern "C" {

  static
  JSBool
  addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // do we have a monitor?
    jsval prop;
    if (!JS_GetProperty(cx,obj,"monitor",&prop)) return JS_TRUE;
    if (JSVAL_IS_VOID(prop)) return JS_TRUE;
    if (!JSVAL_IS_OBJECT(prop)) EJS_THROW_ERROR(cx,obj,"monitor must be an object");
    JSObject *monitor;
    if (!JS_ValueToObject(cx, prop, &monitor)) return JS_FALSE;
    if (!monitor) EJS_THROW_ERROR(cx,obj,"monitor is null or undefined");

    // does monitor have corresponding callback?
    jsval cbProp;
    if (!JS_GetProperty(cx,monitor,"onAdd",&cbProp)) return JS_FALSE;
    if (JSVAL_IS_VOID(cbProp)) return JS_TRUE;
    JSFunction* cb;
    // todo: JS_ValueToFunction is deprecated - why?
    if (!(cb=JS_ValueToFunction(cx,cbProp))) return JS_FALSE;

    // call back
    jsval argv[]={OBJECT_TO_JSVAL(obj),id,*vp};
    if (!JS_CallFunction(cx,monitor,cb, sizeof(argv)/sizeof(argv[0]), argv, vp)) return JS_FALSE;

    jsval error;
    if (JS_GetPendingException(cx,&error)) {
      // pending exception - catch exception of type PropertyAddError
      if (!JSVAL_IS_OBJECT(error)) return JS_FALSE;
      // todo test if "correct" exception (this.propertyAddError)
      if (0) return JS_FALSE;
      JS_ClearPendingException(cx);
    }
    return JS_TRUE;
  }

  static
  JSBool
  delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // todo
    return JS_TRUE;
  }

  static
  JSBool
  getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // todo
    return JS_TRUE;
  }

  static
  JSBool
  setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // todo
    return JS_TRUE;
  }

  static
  JSClass monitorable_class = {
    "Monitorable",
    0,
    addProperty,  delProperty,  getProperty,  setProperty,
    JS_EnumerateStub, JS_ResolveStub,   JS_ConvertStub,   JS_FinalizeStub,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  static
  JSBool
  monitorable_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // called as function f.e. x=Test() - we act like x=new Test()
      // todo do we really want this?
      if (!(obj=JS_NewObject(cx,&monitorable_class,NULL,NULL))) return JS_FALSE;
      *rval=OBJECT_TO_JSVAL(obj);
    }

    // todo parse arguments
    switch(argc){
    case 0:
      break;
    case 1:
      break;
    default:
      EJS_THROW_ERROR(cx,obj,"Wrong number of args");
    }
    return JS_TRUE;
  }

  JSBool
  ejsmonitorable_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *proto = JS_InitClass(cx, global,
				   NULL,
				   &monitorable_class,
				   monitorable_cons, 0,
				   NULL, NULL,
				   NULL, NULL);
    if (!proto) EJS_THROW_ERROR(cx,global,"Could not init class");
    return JS_TRUE;
  }

  JSBool
  ejsmonitorable_LTX_onUnLoad()
  {
    return JS_TRUE;
  }
}

