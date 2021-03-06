#ifndef EJS_ELEMENT_H
#define EJS_ELEMENT_H

#include <ejsmodule.h>
#include <w3c/dom/Element.hpp>

extern "C" {

  JSBool
  ejselement_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_WrapElement(JSContext *cx, JSObject *obj, dom::Element* element);

JSBool
ejselement_class(JSContext *cx, JSObject *obj);

#if 0
JSBool
ejselement_GetNative(JSContext* cx, JSObject * obj, dom::Element* &native);
#endif

#endif
