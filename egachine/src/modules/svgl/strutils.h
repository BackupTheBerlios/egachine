#ifndef EJS_STRUTILS_H
#define EJS_STRUTILS_H

#include <w3c/dom/Node.hpp>
#include <ejsmodule.h>

inline
JSBool
jsToDomString(JSContext* cx, jsval strIn, dom::String* &strOut)
{
  // todo: root string?!
  JSString *jsstr=JS_ValueToString(cx, strIn);
  if (!jsstr) return JS_FALSE;
  strOut=unicode::String::createStringUtf16(JS_GetStringChars(jsstr),JS_GetStringLength(jsstr));
  return JS_TRUE;
}

inline
JSBool
jsToDomStringLat1(JSContext* cx, jsval strIn, dom::String* &strOut)
{
  // todo: root string?!
  JSString *jsstr=JS_ValueToString(cx, strIn);
  if (!jsstr) return JS_FALSE;
  strOut=unicode::String::createStringLat1(JS_GetStringBytes(jsstr),JS_GetStringLength(jsstr));
  return JS_TRUE;
}

inline
JSBool
DomStringToJsval(JSContext* cx, dom::String* strIn, jsval* strOut)
{
  if (!strIn) {
    *strOut=JSVAL_NULL;
    return JS_TRUE;
  }
  JSString *s;
  if (strIn->getType()==dom::String::string_utf16) {
    if (!(s=JS_NewUCStringCopyN(cx, strIn->as_utf16(), strIn->getLength()))) return JS_FALSE;
    *strOut=STRING_TO_JSVAL(s);
  }else if (strIn->getType()==dom::String::string_lat1) {
    // hmm probably bug in svgl:
    // as_lat1() returned 0 and getLength()>0
    const char *c=strIn->as_lat1();
    unsigned l=strIn->getLength();
    if (!c) {
      if (l>0) EJS_WARN("probably bug in svgls string handling?");
      *strOut=OBJECT_TO_JSVAL(NULL);
      return JS_TRUE;
    }
    if (!(s=JS_NewStringCopyN(cx, c, l))) return JS_FALSE;
    *strOut=STRING_TO_JSVAL(s);
  }else
    // todo
    EJS_CHECK(0);
  return JS_TRUE;
}
#endif
