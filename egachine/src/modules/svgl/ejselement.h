#ifndef EJS_DOCUMENT_H
#define EJS_DOCUMENT_H

#include <ejsmodule.h>
#include <w3c/dom/Element.hpp>

extern "C" {

  JSBool
  ejselement_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_NewElement(JSContext *cx, JSObject *obj, dom::Element* element);

#endif
