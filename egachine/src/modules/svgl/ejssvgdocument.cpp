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
  \brief Javascript svgdocument object wrapper
  \author Jens Thiele
*/

#include <w3c/svg/SVGDocument.hpp>
#include <w3c/svg/SVGSVGElement.hpp>
#include <w3c/svg/SVGRectElement.hpp>
#include "ejssvgdocument.h"
#include "ejselement.h"
#include <cassert>

extern "C" {

  static
  void
  svgdocument_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass svgdocument_class = {
    "SVGDocument",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  svgdocument_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &svgdocument_class) did not work
  // because JS_THREADSAFE was not defined correctly (defined or undefined)

#define GET_SVGDOCUMENT_OBJ svg::SVGDocument* svgdocument=NULL;		\
    ejssvgdocument_GetNative(cx,obj,svgdocument);			\

  static
  JSBool
  svgdocument_createElement(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_SVGDOCUMENT_OBJ;

    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;

    // javascript uses ucs-2 encoding and svgl string does not support it?
    // for now we interpret ucs-2 as utf16 which should work?
    // TODO: check above, check for exceptions, how does svgl GC?
    
    dom::Element* element=svgdocument->createElement(unicode::String::createStringUtf16(JS_GetStringChars(strtype)));
    assert(element);
    
    // now create javascript wrapper object for element
    // TODO: what about polymorphism? shouldn't we create a specialized element?
    JSObject* njsobj=ejs_NewElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_addSample(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_SVGDOCUMENT_OBJ;

    svg::SVGSVGElement * thesvgelt = new svg::SVGSVGElement(svgdocument);
    thesvgelt->setWidth(450);
    thesvgelt->setHeight(450);

    svgdocument->appendChild(thesvgelt);

    svg::SVGRectElement * rect = new svg::SVGRectElement(svgdocument);

    double w=100,h=100;
    double x = 100;
    double y = 100;

    rect->setX(w/2);
    rect->setY(h/2);
    rect->setWidth(w);
    rect->setHeight(h);
    rect->setFill("blue");
    rect->setStroke(0,0,0);
    rect->setStrokeWidth(4);
    rect->setOpacity(.75);

    thesvgelt->appendChild(rect);

    return JS_TRUE;
  }

#undef GET_SVGDOCUMENT_OBJ

#define FUNC(name, args) { #name,svgdocument_##name,args,0,0}

  static JSFunctionSpec svgdocument_methods[] = {
    FUNC(createElement,1),
    FUNC(addSample,0),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC



  static
  JSBool
  svgdocument_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }
    if (!JS_SetPrivate(cx,obj,(void *)new svg::SVGDocument()))
      return JS_FALSE;
    return JS_TRUE;
  }

  static
  void
  svgdocument_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &svgdocument_class);
    svg::SVGDocument* svgdocument=(svg::SVGDocument *)JS_GetPrivate(cx,obj);
    if (!svgdocument) return;
    delete svgdocument;
  }

  JSBool
  ejssvgdocument_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *svgdocument = JS_InitClass(cx, global,
				   NULL,
				   &svgdocument_class,
				   svgdocument_cons, 0,
				   NULL, svgdocument_methods,
				   NULL, NULL);
    if (!svgdocument) return JS_FALSE;
    return JS_TRUE;
  }
}

JSBool
ejssvgdocument_GetNative(JSContext* cx, JSObject * obj, svg::SVGDocument* &native)
{
  if (JS_GET_CLASS(cx, obj) != &svgdocument_class)
    EJS_THROW_ERROR(cx,obj,"incompatible object type");
  native=(svg::SVGDocument *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid svgdocument object");
  return JS_TRUE;
}

