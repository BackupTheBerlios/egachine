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
#include "ejsnode.h"
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

  // todo: clean up

  // todo: is this class check good enough to make this dangerous cast safe?
  // Remember: if (JS_GET_CLASS(cx, obj) != &node_class) did not work
  // because JS_THREADSAFE was not defined correctly (defined or undefined)

#define GET_NODE_OBJ dom::Node* node=NULL;			\
    if (!ejsnode_GetNative(cx,obj,node)) return JS_FALSE

  // todo: this is a method of Node (Node->Node)
  static
  JSBool
  node_setNodeValue(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NODE_OBJ;
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    unicode::String* value=unicode::String::createStringUtf16(JS_GetStringChars(strtype));
    try{
      node->setNodeValue(value);
    }catch(const dom::DOMException &e){
      // todo: throw dom exception
      EJS_THROW_ERROR(cx,obj,e.what());
    }
    return JS_TRUE;
  }

  // TODO: this is a method of Node (SVGDocument is also a Node)
  // currently duplicated
  static
  JSBool
  node_appendChild(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NODE_OBJ;
    if (!JSVAL_IS_OBJECT(argv[0])) EJS_THROW_ERROR(cx,obj,"object as arg 0 required");
    
    // todo: node expected as agrument not neccessary an node
    dom::Node* cnode=NULL;
    if (!ejsnode_GetNative(cx,JSVAL_TO_OBJECT(argv[0]),cnode)) return JS_FALSE;
    // todo: exception
    node->appendChild(cnode);
    *rval=argv[0];
    return JS_TRUE;
  }
  
#undef GET_NODE_OBJ

#define FUNC(name, args) { #name,node_##name,args,0,0}

  static JSFunctionSpec node_methods[] = {
    FUNC(setNodeValue,1),
    //    FUNC(setAttribute,2),
    FUNC(appendChild,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC



  static
  JSBool
  node_cons
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
  node_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &node_class);
    dom::Node* node=(dom::Node *)JS_GetPrivate(cx,obj);
    if (!node) return;
    delete node;
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
ejsnode_GetNative(JSContext* cx, JSObject * obj, dom::Node* &native)
{
  if (JS_GET_CLASS(cx, obj) != &node_class)
    EJS_THROW_ERROR(cx,obj,"incompatible object type");
  native=(dom::Node *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid dom::Node object");
  return JS_TRUE;
}
