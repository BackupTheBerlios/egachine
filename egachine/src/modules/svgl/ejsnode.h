#ifndef EJS_NODE_H
#define EJS_NODE_H

#include <ejsmodule.h>
#include <w3c/dom/Node.hpp>

extern "C" {

  JSBool
  ejsnode_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_InitNodeSubClass(JSContext *cx, JSObject *obj,
		     const std::type_info &t,
		     const char* consname, JSNative constructor, uintN nargs,
		     JSPropertySpec *ps, JSFunctionSpec *fs,
		     JSPropertySpec *static_ps=NULL, JSFunctionSpec *static_fs=NULL);

//! wrap any node object
/*
  C++ rtti is used to determine which wrapper to create
  if proto is NULL
  (this only works if you registered a prototype for this object !!
*/
JSObject*
ejs_WrapNode(JSContext *cx, JSObject *obj, dom::Node* node, JSObject* proto=NULL);

JSClass&
ejs_getNodeClass();

JSBool
ejsnode_GetNative(JSContext* cx, JSObject * obj, jsval* argv, dom::Node* &native);

#endif
