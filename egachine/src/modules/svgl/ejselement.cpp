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
#include "ejsallelements.h"
#include "strutils.h"
#include <cassert>

static JSObject *elementProto = NULL;

JSBool
ejselement_GetNative(JSContext* cx, JSObject * obj, jsval* argv, dom::Element* &native);

extern "C" {

#define GET_NTHIS(cx,obj,argv) dom::Element* nthis=NULL;		\
    if (!ejselement_GetNative(cx,obj,argv,nthis)) return JS_FALSE

  static
  JSBool
  element_setAttribute(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    GET_NTHIS(cx,obj,argv);
    dom::String* name=NULL;
    if (!jsToDomString(cx,argv[0],name)) return JS_FALSE;
    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[1],value)) return JS_FALSE;

    /*
    EJS_INFO("Set Attribute: '"
	     << *name << "' ("<<name->getLength()<<")" 
	     << " to '" << *value <<"' ("<<value->getLength()<<")"); */

    // todo: catch exceptions
    nthis->setAttribute(name,value);

    // todo: hmmm
    if (svg::SVGElement *svgelement = dynamic_cast<svg::SVGElement *>(nthis)) {
      svgelement->updateStyle(NULL);
    }
    return JS_TRUE;
  }

  static
  JSBool
  element_getAttribute(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj,argv);

    dom::String* name=NULL;
    if (!jsToDomString(cx,argv[0],name)) return JS_FALSE;

    // todo: catch exceptions
    unicode::String* value=nthis->getAttribute(name);
    return DomStringToJsval(cx,value,rval);
  }

#undef GET_NTHIS

#define FUNC(name, args) { #name,element_##name,args,0,0},

  static JSFunctionSpec element_methods[] = {
    FUNC(setAttribute,2)
    FUNC(getAttribute,1)
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  element_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    return JS_TRUE;
  }
  
  JSBool
  ejselement_onLoad(JSContext *cx, JSObject *module)
  {
    elementProto = ejs_InitNodeSubClass(cx, module, 
					typeid(dom::Element),
					"Element", element_cons, 0,
					NULL, element_methods);
    if (!elementProto) return JS_FALSE;
    return JS_TRUE;
  }
}

JSBool
ejselement_GetNative(JSContext* cx, JSObject * obj, jsval* argv, dom::Element* &native)
{
  dom::Node* node=NULL;
  if (!ejsnode_GetNative(cx, obj, argv, node)) return JS_FALSE;
  if (!(native=dynamic_cast<dom::Element*>(node)))
    EJS_THROW_ERROR(cx,obj,"no valid dom::Element object");
  return JS_TRUE;
}

JSObject*
ejs_WrapElement(JSContext *cx, JSObject *obj, dom::Element* element)
{
  return ejs_WrapNode(cx, obj, element, elementProto);
}
