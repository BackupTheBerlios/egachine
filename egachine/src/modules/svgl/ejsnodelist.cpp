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
  \brief Javascript node object wrapper
  \author Jens Thiele
*/

#include <w3c/dom/NodeList.hpp>
#include "ejsallelements.h"
#include <cassert>

static JSObject* nodelistProto = NULL;

extern "C" {

  static
  void
  nodelist_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass nodelist_class = {
    "NodeList",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  nodelist_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

#define GET_NTHIS(cx,obj) dom::NodeList* nthis=NULL;		\
    if (!ejsnodelist_GetNative(cx,obj,nthis)) return JS_FALSE

  static
  JSBool
  nodelist_item(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj);
    uint32 item;
    if (!JS_ValueToECMAUint32(cx,argv[0],&item)) return JS_FALSE;
    dom::Node* node=nthis->item(item);
    if (!node) {
      *rval=OBJECT_TO_JSVAL(node);
      return JS_TRUE;
    }
    if (node->getNodeType()==dom::Node::ELEMENT_NODE) {
      dom::Element *element = dynamic_cast<dom::Element *>(nthis);
      EJS_CHECK(element);
      JSObject* ret=ejs_NewElement(cx,obj,element);
      if (!ret) return JS_FALSE;
      *rval=OBJECT_TO_JSVAL(ret);
    }else{
      JSObject* ret=ejs_NewNode(cx,obj,node);
      if (!ret) return JS_FALSE;
      *rval=OBJECT_TO_JSVAL(ret);
    }
    return JS_TRUE;
  }
  
  
#undef GET_NTHIS

#define FUNC(name, args) { #name,nodelist_##name,args,0,0}

  static JSFunctionSpec nodelist_methods[] = {
    FUNC(item,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  nodelist_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    return JS_TRUE;
  }

  static
  void
  nodelist_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &nodelist_class);
    dom::NodeList* nodelist=(dom::NodeList *)JS_GetPrivate(cx,obj);
    if (!nodelist) return;
    //    delete nodelist;
  }

  JSBool
  ejsnodelist_onLoad(JSContext *cx, JSObject *module)
  {
    nodelistProto = JS_InitClass(cx, module,
				     NULL,
				     &nodelist_class,
				     nodelist_cons, 0,
				     NULL, nodelist_methods,
				     NULL, NULL);
    if (!nodelistProto) return JS_FALSE;
    return JS_TRUE;
  }
}

JSObject*
ejs_NewNodeList(JSContext *cx, JSObject *obj, const dom::NodeList* nodelist)
{
  assert(nodelist);
  assert(nodelistProto);
  // todo: should we set parent?
  // this object is not rooted !!
  // todo: should we try to downcast?

  JSObject *res=JS_NewObject(cx,&nodelist_class, nodelistProto, NULL);
  if (!res) return NULL;
  if (!JS_SetPrivate(cx,res,(void *)nodelist)) return NULL;
  return res;
}

JSBool
ejsnodelist_class(JSContext *cx, JSObject *obj)
{
  return JS_GET_CLASS(cx, obj) == &nodelist_class;
}

JSBool
ejsnodelist_GetNative(JSContext* cx, JSObject * obj, dom::NodeList* &native)
{
  EJS_CHECK_CLASS(cx, obj, nodelist_class);
  native=(dom::NodeList *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::NodeList object");
  return JS_TRUE;
}
