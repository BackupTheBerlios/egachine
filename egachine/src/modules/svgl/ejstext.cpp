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
  \brief Javascript text object wrapper
  \author Jens Thiele
*/

#include "ejsallelements.h"
#include <cassert>

static JSObject *textProto = NULL;

extern "C" {

  static
  void
  text_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass text_class = {
    "Text",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  text_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

#define GET_NTHIS(cx,obj) dom::Text* nthis=NULL;		\
    if (!ejstext_GetNative(cx,obj,nthis)) return JS_FALSE

  // functions inherited from dom::Node
#define EJS_FUNC(x) text_##x
#include "nodefdefs.h"
#undef EJS_FUNC

#undef GET_NTHIS

#define FUNC(name, args) { #name,text_##name,args,0,0},

  static JSFunctionSpec text_methods[] = {
#include "nodefuncs.h"
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  text_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    return JS_TRUE;
  }

  static
  void
  text_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &text_class);
    dom::Text* text=(dom::Text *)JS_GetPrivate(cx,obj);
    if (!text) return;
    //    delete text;
  }

  JSBool
  ejstext_onLoad(JSContext *cx, JSObject *module)
  {
    textProto = JS_InitClass(cx, module,
			     NULL,
			     &text_class,
			     text_cons, 0,
			     NULL, text_methods,
			     NULL, NULL);
    if (!textProto) return JS_FALSE;
    return JS_TRUE;
  }
}

JSObject*
ejs_NewText(JSContext *cx, JSObject *obj, dom::Text* text)
{
  assert(text);
  assert(textProto);

  // todo: should we set parent?
  // this object is not rooted !!
  JSObject *res=JS_NewObject(cx,&text_class, textProto, NULL);
  if (!res) return NULL;
  if (!JS_SetPrivate(cx,res,(void *)text)) return NULL;
  return res;
}

JSBool
ejstext_class(JSContext* cx, JSObject *obj)
{
  return JS_GET_CLASS(cx, obj) == &text_class;
}

JSBool
ejstext_GetNative
(JSContext* cx, JSObject * obj, dom::Text* &native)
{
  EJS_CHECK_CLASS(cx, obj, text_class);
  native=(dom::Text *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::Text object");
  return JS_TRUE;
}
