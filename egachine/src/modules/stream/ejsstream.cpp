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
  \file stream/ejsstream.cpp
  \brief Javascript stream object (wrapper around std::basic_streambuf)
  \author Jens Thiele

  \todo as always: js strings are ucs-2 !
*/

#include <cassert>
#include <ejsmodule.h>
#include <iostream>

extern "C" {

  static
  void
  stream_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass stream_class = {
    "Stream",
    JSCLASS_HAS_PRIVATE|JSCLASS_HAS_RESERVED_SLOTS(1),
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  stream_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &stream_class) did not work
  // because JS_THREADSAFE was not defined correctly (defined or undefined)

#define GET_STREAM_OBJ std::streambuf* stream=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &stream_class)				\
      EJS_THROW_ERROR(cx,obj,"incompatible object type"); \
    stream=(std::streambuf *)JS_GetPrivate(cx,obj);			\
    if (!stream)							\
      EJS_THROW_ERROR(cx,obj,"no valid stream object")

  static
  JSBool
  stream_write(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_STREAM_OBJ;

    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    int w=stream->sputn(ctype,strlen(ctype));
    return JS_NewNumberValue(cx,w,rval);
  }

  static
  JSBool
  stream_read(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_STREAM_OBJ;
    if (!JSVAL_IS_INT(argv[0])) return JS_FALSE;
    int toread=JSVAL_TO_INT(argv[0]);
    char* buf=(char *)JS_malloc(cx,toread+1);
    if (!buf) return JS_FALSE;
    int got=stream->sgetn(buf,toread);
    assert(got>=0);
    assert(got<=toread);
    
    if (got<toread) {
      buf=(char *)JS_realloc(cx,buf,got+1);
      if (!buf)
	return JS_FALSE;
    }
    buf[got]='\0';
    JSString *r=JS_NewString(cx, buf, got);
    if (!r) {
      JS_free(cx,buf);
      return JS_FALSE;
    }
    *rval=STRING_TO_JSVAL(r);
    return JS_TRUE;
  }

  static
  JSBool
  stream_sync(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval*)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_STREAM_OBJ;
    stream->pubsync();
    return JS_TRUE;
  }

  static
  JSBool
  stream_inAvailable(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_STREAM_OBJ;
    return JS_NewNumberValue(cx,stream->in_avail(),rval);
  }

#undef GET_STREAM_OBJ



#define FUNC(name, args) { #name,stream_##name,args,0,0}

  static JSFunctionSpec stream_methods[] = {
    FUNC(write,1),
    FUNC(read,1),
    FUNC(sync,0),
    FUNC(inAvailable,0),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC



  static
  JSBool
  stream_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }
    return JS_TRUE;
  }

  static
  void
  stream_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &stream_class);
    std::streambuf* stream=(std::streambuf *)JS_GetPrivate(cx,obj);
    if (!stream) return;
    jsval owner;
    EJS_CHECK(JS_GetReservedSlot(cx,obj,0,&owner));
    EJS_CHECK(JSVAL_IS_BOOLEAN(owner));
    if (owner==JSVAL_TRUE)
      delete stream;
  }

  JSBool
  ejsstream_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *stream = JS_InitClass(cx, global,
				   NULL,
				   &stream_class,
				   stream_cons, 0,
				   NULL, stream_methods,
				   NULL, NULL);
    if (!stream) return JS_FALSE;
    // create stdin, stdout, stderr streams
    JSObject *obj;
    
    if (!(obj=JS_DefineObject(cx, global,"stdin", &stream_class, NULL, JSPROP_ENUMERATE)))
      return JS_FALSE;
    if (!JS_SetPrivate(cx,obj,(void *)std::cin.rdbuf()))
      return JS_FALSE;
    // tell the stream object wether to delete this streambuf
    if (!JS_SetReservedSlot(cx,obj,0,JSVAL_FALSE))
      return JS_FALSE;

    if (!(obj=JS_DefineObject(cx, global,"stdout", &stream_class, NULL, JSPROP_ENUMERATE)))
      return JS_FALSE;
    if (!JS_SetPrivate(cx,obj,(void *)std::cout.rdbuf()))
      return JS_FALSE;
    // tell the stream object wether to delete this streambuf
    if (!JS_SetReservedSlot(cx,obj,0,JSVAL_FALSE))
      return JS_FALSE;

    if (!(obj=JS_DefineObject(cx, global,"stderr", &stream_class, NULL, JSPROP_ENUMERATE)))
      return JS_FALSE;
    if (!JS_SetPrivate(cx,obj,(void *)std::cerr.rdbuf()))
      return JS_FALSE;
    // tell the stream object wether to delete this streambuf
    if (!JS_SetReservedSlot(cx,obj,0,JSVAL_FALSE))
      return JS_FALSE;

    return JS_TRUE;
  }

  JSBool
  ejsstream_LTX_onUnLoad()
  {
    return JS_TRUE;
  }
}
