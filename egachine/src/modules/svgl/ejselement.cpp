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
  \brief Javascript element object wrapper
  \author Jens Thiele
*/

#include <w3c/svg/Element.hpp>
#include "ejselement.h"
#include <cassert>

extern "C" {

  static
  void
  element_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass element_class = {
    "Element",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  element_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &element_class) did not work
  // because JS_THREADSAFE was not defined correctly (defined or undefined)

#define GET_ELEMENT_OBJ dom::Element* element=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &element_class)			\
      EJS_THROW_ERROR(cx,obj,"incompatible object type");		\
    element=(dom::Element *)JS_GetPrivate(cx,obj);			\
    if (!element)							\
      EJS_THROW_ERROR(cx,obj,"no valid dom::Element object")
  
#undef GET_ELEMENT_OBJ

#define FUNC(name, args) { #name,element_##name,args,0,0}

  static JSFunctionSpec element_methods[] = {
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC



  static
  JSBool
  element_cons
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
  element_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &element_class);
    dom::Element* element=(dom::Element *)JS_GetPrivate(cx,obj);
    if (!element) return;
    delete element;
  }

  JSBool
  ejselement_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *element = JS_InitClass(cx, global,
				     NULL,
				     &element_class,
				     element_cons, 0,
				     NULL, element_methods,
				     NULL, NULL);
    if (!element) return JS_FALSE;
    return JS_TRUE;
  }
}

JSObject*
ejs_NewElement(JSContext *cx, JSObject *obj, dom::Element* element)
{
  assert(element);
  // todo: should we set parent?
  // this object is not rooted !!
  JSObject *res=JS_NewObject(cx,&element_class, NULL,NULL);
  if (!res) return NULL;
  return res;
}
