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
#include <w3c/svg/SVGElement.hpp>
#include "ejselement.h"
#include "ejstext.h"
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

#define GET_NTHIS dom::Element* nthis=NULL;		\
    if (!ejselement_GetNative(cx,obj,nthis)) return JS_FALSE

  // functions inherited from dom::Node
#define EJS_FUNC(x) element_##x
#include "nodefuncs.h"
#undef EJS_FUNC
  
  static
  JSBool
  element_setAttribute(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    GET_NTHIS;
    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    unicode::String* name=unicode::String::createStringUtf16(JS_GetStringChars(strtype));

    strtype=JS_ValueToString(cx, argv[1]);
    if (!strtype) return JS_FALSE;
    unicode::String* value=unicode::String::createStringUtf16(JS_GetStringChars(strtype));

    // todo: catch exceptions
    nthis->setAttribute(name,value);

    // todo: hmmm
    if (svg::SVGElement *svgelement = dynamic_cast<svg::SVGElement *>(nthis)) {
      svgelement->updateStyle(NULL);
    }
    return JS_TRUE;
  }


#undef GET_NTHIS

#define FUNC(name, args) { #name,element_##name,args,0,0}

  static JSFunctionSpec element_methods[] = {
    FUNC(setNodeValue,1),
    FUNC(appendChild,1),
    FUNC(setAttribute,2),
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
  if (!JS_SetPrivate(cx,res,(void *)element)) return NULL;
  return res;
}

JSBool
ejselement_class(JSContext* cx, JSObject *obj)
{
  return JS_GET_CLASS(cx, obj) == &element_class;
}

JSBool
ejselement_GetNative(JSContext* cx, JSObject * obj, dom::Element* &native)
{
  EJS_CHECK_CLASS(cx, obj, element_class);
  native=(dom::Element *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::Element object");
  return JS_TRUE;
}
