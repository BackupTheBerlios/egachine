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

#include <w3c/dom/Node.hpp>
#include "ejsallelements.h"
#include <cassert>

extern "C" {

  static
  void
  node_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass node_class = {
    "Node",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  node_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

#define GET_NTHIS dom::Node* nthis=NULL;			\
    if (!ejsnode_GetNative(cx,obj,nthis)) return JS_FALSE

#define EJS_FUNC(x) node_##x
#include "nodefuncs.h"
#undef EJS_FUNC
  
#undef GET_NTHIS

#define FUNC(name, args) { #name,node_##name,args,0,0}

  static JSFunctionSpec node_methods[] = {
    FUNC(setNodeValue,1),
    FUNC(appendChild,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  node_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    return JS_TRUE;
  }

  static
  void
  node_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &node_class);
    dom::Node* node=(dom::Node *)JS_GetPrivate(cx,obj);
    if (!node) return;
    //    delete node;
  }

  JSBool
  ejsnode_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *node = JS_InitClass(cx, global,
				     NULL,
				     &node_class,
				     node_cons, 0,
				     NULL, node_methods,
				     NULL, NULL);
    if (!node) return JS_FALSE;
    return JS_TRUE;
  }
}

JSObject*
ejs_NewNode(JSContext *cx, JSObject *obj, dom::Node* node)
{
  assert(node);
  // todo: should we set parent?
  // this object is not rooted !!
  JSObject *res=JS_NewObject(cx,&node_class, NULL,NULL);
  if (!res) return NULL;
  if (!JS_SetPrivate(cx,res,(void *)node)) return NULL;
  return res;
}

JSBool
ejsnode_class(JSContext *cx, JSObject *obj)
{
  return JS_GET_CLASS(cx, obj) == &node_class;
}

JSBool
ejsnode_GetNative(JSContext* cx, JSObject * obj, dom::Node* &native)
{
  EJS_CHECK_CLASS(cx, obj, node_class);
  native=(dom::Node *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::Node object");
  return JS_TRUE;
}
