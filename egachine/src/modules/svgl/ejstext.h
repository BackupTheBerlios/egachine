#ifndef EJS_TEXT_H
#define EJS_TEXT_H

#include <ejsmodule.h>
#include <w3c/dom/Text.hpp>

extern "C" {

  JSBool
  ejstext_onLoad(JSContext *cx, JSObject *global);
  
}

JSObject*
ejs_NewText(JSContext *cx, JSObject *obj, dom::Text* text);

JSBool
ejstext_class(JSContext* cx, JSObject* obj);

JSBool
ejstext_GetNative(JSContext* cx, JSObject * obj, dom::Text* &native);

#endif
