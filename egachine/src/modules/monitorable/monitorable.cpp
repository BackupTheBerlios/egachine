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
#include <cassert>

//! get monitor if available
/*!
  \return monitor object or NULL

  \todo - we do not differentiate between and errors?
*/
static
JSObject*
getMonitor(JSContext *cx, JSObject *obj)
{
  jsval prop;
  JSObject *monitor=NULL;
  if ((!JS_GetProperty(cx,obj,"monitor",&prop))
      ||(JSVAL_IS_VOID(prop))
      ||(!JSVAL_IS_OBJECT(prop))
      ||(!JS_ValueToObject(cx, prop, &monitor)))
    return NULL;
  return monitor;
}

//! get monitor and callback function if available
static
JSBool
getCallback(JSContext *cx, JSObject *obj, const char* cbName, JSObject *&monitor, JSFunction *&cb)
{
  cb=NULL;
  monitor=getMonitor(cx,obj);
  if (!monitor) return JS_TRUE;
  
  // does monitor have corresponding callback?
  jsval cbProp;
  // todo: JS_ValueToFunction is deprecated - why?
  if ((!JS_GetProperty(cx,monitor,cbName,&cbProp))
      ||(JSVAL_IS_VOID(cbProp))
      ||(!(cb=JS_ValueToFunction(cx,cbProp))))
    return JS_TRUE;
  return JS_TRUE;
}

//! call callback function of monitor object if available
static
JSBool
callback(JSContext *cx, JSObject *obj, jsval id, jsval *vp, const char* cbName)
{
  JSObject* monitor;
  JSFunction* cb;
  JSBool ret=getCallback(cx,obj,cbName,monitor,cb);
  if (!cb) return ret;
  assert(monitor);
  
  // call back
  jsval argv[]={OBJECT_TO_JSVAL(obj),id,*vp};
  if (!JS_CallFunction(cx,monitor,cb, sizeof(argv)/sizeof(argv[0]), argv, vp))
    return JS_FALSE;
  
  jsval error;
  if (JS_GetPendingException(cx,&error)) {
    // pending exception - catch exception of type CallbackError or the like
    if (!JSVAL_IS_OBJECT(error)) return JS_FALSE;
    // todo test if "correct" exception
    if (0) return JS_FALSE;
    JS_ClearPendingException(cx);
  }
  return JS_TRUE;
}

extern "C" {

  static
  JSBool
  addProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    return callback(cx,obj,id,vp,"onAdd");
  }

  static
  JSBool
  delProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    return callback(cx,obj,id,vp,"onDelete");
  }

  static
  JSBool
  getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    // hmm trouble here because we to get the callback we get a property
    // => the callback to get a property is called again
    // => endless loop => crash
    // => we must treat the monitor property specially
    // todo: improve this
    JSString* s=NULL;
    char* cstr=NULL;
    if ((s=JS_ValueToString(cx, id))
	&&(cstr=JS_GetStringBytes(s))
	&&(!strcmp("monitor",cstr))) return JS_TRUE;
    return callback(cx,obj,id,vp,"onGet");
  }

  static
  JSBool
  setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    return callback(cx,obj,id,vp,"onSet");
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
    return JS_TRUE;
  }

  JSBool
  ejsmonitorable_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    if (!JS_InitClass(cx, module,
		      NULL,
		      &monitorable_class,
		      monitorable_cons, 0,
		      NULL, NULL,
		      NULL, NULL)) return JS_FALSE;
    return JS_TRUE;
  }
}
