#ifndef EJS_NODE_H
#define EJS_NODE_H

#include <ejsmodule.h>
#include <w3c/dom/Node.hpp>

extern "C" {

  JSBool
  ejsnode_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_NewNode(JSContext *cx, JSObject *obj, dom::Node* node);

JSBool
ejsnode_class(JSContext *cx, JSObject *obj);

JSBool
ejsnode_GetNative(JSContext* cx, JSObject * obj, dom::Node* &native);

#define FUNC(name,args) JSBool \
  ejs_Node_##name \
  (JSContext* cx, JSObject* jsthis, dom::Node* nthis, uintN argc, jsval* argv, jsval* rval);

#include "nodefuncs.h"

#undef FUNC

#endif
