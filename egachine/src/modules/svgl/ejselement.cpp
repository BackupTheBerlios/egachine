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
    if (!ejselement_GetNative(cx,obj,element)) return JS_FALSE

  static
  JSBool
  element_setAttribute(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    GET_ELEMENT_OBJ;
    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    unicode::String* name=unicode::String::createStringUtf16(JS_GetStringChars(strtype));

    strtype=JS_ValueToString(cx, argv[1]);
    if (!strtype) return JS_FALSE;
    unicode::String* value=unicode::String::createStringUtf16(JS_GetStringChars(strtype));

    // todo: catch exceptions
    element->setAttribute(name,value);
    
    return JS_TRUE;
  }

  // TODO: this is a method of Node (SVGDocument is also a Node)
  // currently duplicated
  static
  JSBool
  element_appendChild(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_ELEMENT_OBJ;
    if (!JSVAL_IS_OBJECT(argv[0])) EJS_THROW_ERROR(cx,obj,"object as arg 0 required");
    
    // todo: node expected as agrument not neccessary an element
    dom::Element* celement=NULL;
    if (!ejselement_GetNative(cx,JSVAL_TO_OBJECT(argv[0]),celement)) return JS_FALSE;
    // todo: exception
    element->appendChild(celement);
    *rval=argv[0];
    return JS_TRUE;
  }
  
#undef GET_ELEMENT_OBJ

#define FUNC(name, args) { #name,element_##name,args,0,0}

  static JSFunctionSpec element_methods[] = {
    FUNC(setAttribute,2),
    FUNC(appendChild,1),
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
ejselement_GetNative(JSContext* cx, JSObject * obj, dom::Element* &native)
{
  if (JS_GET_CLASS(cx, obj) != &element_class)
    EJS_THROW_ERROR(cx,obj,"incompatible object type");
  native=(dom::Element *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::Element object");
  return JS_TRUE;
}
