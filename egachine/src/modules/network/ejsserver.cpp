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
  \brief Server Class wrapper
  \author Jens Thiele
*/

#include "server.h"
#include "ejsserver.h"

extern "C" {

  static
  void
  server_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass server_class = {
    "Server",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  server_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &server_class) did not work
  // because JS_THREADSAFE was not defined correctly (defined or undefined)

#define GET_SERVER_OBJ Server* server=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &server_class)			\
      EJS_THROW_ERROR(cx,obj,"incompatible object type");	\
    server=(Server *)JS_GetPrivate(cx,obj);		\
    if (!server)						\
      EJS_THROW_ERROR(cx,obj,"no valid server object")

  static
  JSBool
  server_poll(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    try{
      GET_SERVER_OBJ;
      Timeout timeout(0);
      Timeout* timeoutPtr=&timeout;
      if (argc>0) {
	jsval inf=JS_GetPositiveInfinityValue(cx);
	// todo: will this always work?
	if (argv[0]==inf) {
	  timeoutPtr=NULL;
	}else{
	  jsdouble t;
	  if (!JS_ValueToNumber(cx,argv[0],&t)) return JS_FALSE;
	  timeout=t;
	}
      }
      *rval= server->listener.select(timeoutPtr) ? JSVAL_TRUE : JSVAL_FALSE;
    }catch(const CallbackError &error){
      // todo: perhaps use the error
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  static
  JSBool
  server_close(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    GET_SERVER_OBJ;
    delete server;
    JS_SetPrivate(cx,obj,NULL);
    return JS_TRUE;
  }

#undef GET_SERVER_OBJ



#define FUNC(name, args) { #name,server_##name,args,0,0}

  static JSFunctionSpec server_methods[] = {
    FUNC(poll,0),
    FUNC(close,0),
    EJS_END_FUNCTIONSPEC
  };
  
#undef FUNC

  static
  JSBool
  server_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }
    
    EJS_CHECK_TRUSTED(cx,obj);
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &server_class);
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    int32 port;
    if (!JS_ValueToInt32(cx,argv[0],&port))
      return JS_FALSE;
    Server* server=new Server(cx,obj,port);
    if ((!server->init())||(!JS_SetPrivate(cx,obj,(void *)server))) {
      delete server;
      return JS_FALSE;
    }
    return JS_TRUE;
  }
  
  static
  void
  server_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &server_class);
    Server* server=(Server *)JS_GetPrivate(cx,obj);
    if (server) delete server;
  }
}


JSBool
server_onLoad(JSContext *cx, JSObject *net)
{
  JSObject *proto = JS_InitClass(cx, net,
				 NULL,
				 &server_class,
				 server_cons, 0,
				 NULL, server_methods,
				 NULL, NULL);
  if (!proto) EJS_THROW_ERROR(cx,net,"Could not init class");
  return JS_TRUE;
}
