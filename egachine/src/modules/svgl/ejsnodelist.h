#ifndef EJS_NODELIST_H
#define EJS_NODELIST_H

#include <ejsmodule.h>
#include <w3c/dom/NodeList.hpp>

extern "C" {

  JSBool
  ejsnodelist_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_NewNodeList(JSContext *cx, JSObject *obj, const dom::NodeList* nodelist);

JSBool
ejsnodelist_class(JSContext *cx, JSObject *obj);

JSBool
ejsnodelist_GetNative(JSContext* cx, JSObject * obj, dom::NodeList* &native);

#endif
