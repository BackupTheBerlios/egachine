#ifndef EJS_SVGDOCUMENT_H
#define EJS_SVGDOCUMENT_H

#include <ejsmodule.h>
#include <w3c/svg/SVGDocument.hpp>

extern "C" {

JSBool
ejssvgdocument_onLoad(JSContext* cx, JSObject* global);
  
}

JSBool
ejssvgdocument_GetNative(JSContext* cx, JSObject*  obj, jsval* argv, svg::SVGDocument* &native);

#endif
