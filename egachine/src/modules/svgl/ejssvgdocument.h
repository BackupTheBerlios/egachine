#ifndef EJS_SVGDOCUMENT_H
#define EJS_SVGDOCUMENT_H

#include <ejsmodule.h>

extern "C" {

JSBool
ejssvgdocument_onLoad(JSContext *cx, JSObject *global);
  
}

JSBool
ejssvgdocument_GetNative(JSContext* cx, JSObject * obj, svg::SVGDocument* &native);

#endif
