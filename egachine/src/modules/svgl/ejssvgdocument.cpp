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
  \brief Javascript svgdocument object wrapper
  \author Jens Thiele
*/

#include <w3c/dom/ext/DOMVisitor.hpp>
#include <w3c/svg/SVGSVGElement.hpp>
#include <w3c/svg/SVGRectElement.hpp>
#include <svgl/Parser.hpp>
#include "ejsallelements.h"
#include "strutils.h"
#include <cassert>

struct ElementFilter : public dom::DOMVisitor
{
  ElementFilter(dom::String* _filter) : filter(_filter)
  {}
  
  dom::String* filter;
  typedef std::vector<dom::Element *> Output;
  Output output;
  
  virtual void visitElement(dom::Element * node) 
  {
    EJS_CHECK(node->getTagName()&&filter);
    if (*(node->getTagName())==*filter) output.push_back(node);
    DOMVisitor::visitElement(node);
  }
};


extern "C" {

  static
  void
  svgdocument_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass svgdocument_class = {
    "SVGDocument",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  JS_FinalizeStub, // svgdocument_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

#define GET_NTHIS(cx,obj) svg::SVGDocument* nthis=NULL;	\
    ejssvgdocument_GetNative(cx,obj,nthis)

  // functions inherited from dom::Node
#define EJS_FUNC(x) svgdocument_##x
#include "nodefdefs.h"
#undef EJS_FUNC

  static
  JSBool
  svgdocument_createTextNode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj);

    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[0],value)) return JS_FALSE;

    dom::Text* text=nthis->createTextNode(value);
    assert(text);
    JSObject* jstext=ejs_NewText(cx,obj,text);
    // todo: delete text?
    if (!jstext) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(jstext);
    return JS_TRUE;
  }
  
  static
  JSBool
  svgdocument_createElement(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj);

    // todo: root string!
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;

    // javascript uses ucs-2 encoding and svgl string does not support it?
    // for now we interpret ucs-2 as utf16 which should work?
    // TODO: check above, check for exceptions, how does svgl GC?
    
    // TODO: shit
    //    dom::Element* element=nthis->createElement(unicode::String::createStringUtf16(JS_GetStringChars(strtype)));
    // it seems we must use internString otherwise createElement does not work as expected?
    // TODO: take a look at the details
    dom::Element* element=nthis->createElement(unicode::String::internStringUtf16(JS_GetStringChars(strtype),JS_GetStringLength(strtype)));
    assert(element);

    std::cerr << "tag name: " << (*element->getTagName()) << std::endl;
    std::cerr << "node name: " << (*element->getNodeName()) << std::endl;
    std::cerr << "node type: " << ((int)element->getNodeType()) << std::endl;
    
    // now create javascript wrapper object for element
    // TODO: what about polymorphism? shouldn't we create a specialized element? I think no
    // and who is the owner of the native object? gc -> crash?
    // perhaps we must map native to js object?
    // or is svgl always the owner of the native object?
    JSObject* njsobj=ejs_NewElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_createElementNS(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    GET_NTHIS(cx,obj);

    dom::String* nsstr=NULL;
    if (!jsToDomString(cx,argv[0],nsstr)) return JS_FALSE;
    
    // todo: shit
    JSString *tagstr=JS_ValueToString(cx, argv[1]);
    if (!tagstr) return JS_FALSE;

    // javascript uses ucs-2 encoding and svgl string does not support it?
    // for now we interpret ucs-2 as utf16 which should work?
    // TODO: check above, check for exceptions, how does svgl GC?
    
    // TODO: shit
    //    dom::Element* element=nthis->createElement(unicode::String::createStringUtf16(JS_GetStringChars(strtype)));
    // it seems we must use internString otherwise createElement does not work as expected?
    // TODO: take a look at the details
    dom::Element* element=nthis->createElementNS(nsstr,
						 unicode::String::internStringUtf16(JS_GetStringChars(tagstr),JS_GetStringLength(tagstr)));
    assert(element);

    std::cerr << "tag name: " << (*element->getTagName()) << std::endl;
    std::cerr << "node name: " << (*element->getNodeName()) << std::endl;
    std::cerr << "node type: " << ((int)element->getNodeType()) << std::endl;
    
    // now create javascript wrapper object for element
    // TODO: what about polymorphism? shouldn't we create a specialized element? I think no
    // and who is the owner of the native object? gc -> crash?
    // perhaps we must map native to js object?
    // or is svgl always the owner of the native object?
    JSObject* njsobj=ejs_NewElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  
  static
  JSBool
  svgdocument_addSample(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval*) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_NTHIS(cx,obj);

    svg::SVGSVGElement * thesvgelt = new svg::SVGSVGElement(nthis);
    thesvgelt->setWidth(450);
    thesvgelt->setHeight(450);

    nthis->appendChild(thesvgelt);

    svg::SVGRectElement * rect = new svg::SVGRectElement(nthis);

    double w=100,h=100;

    rect->setX(w/2);
    rect->setY(h/2);
    rect->setWidth(w);
    rect->setHeight(h);
    rect->setFill("blue");
    rect->setStroke(0,0,0);
    rect->setStrokeWidth(4);
    rect->setOpacity(.75);

    thesvgelt->appendChild(rect);

    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_getElementById
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj);

    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[0],value)) return JS_FALSE;
    dom::Element* element=nthis->getElementById(value);
    // handle error !!
    EJS_CHECK(element);

    // now create javascript wrapper object for element
    // TODO: what about polymorphism? shouldn't we create a specialized element?
    JSObject* njsobj=ejs_NewElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_getDocumentElement
  (JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_NTHIS(cx,obj);
    dom::Element* element=nthis->getDocumentElement();
    assert(element);
    // now create javascript wrapper object for element
    // TODO: what about polymorphism? shouldn't we create a specialized element?
    JSObject* njsobj=ejs_NewElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument__handleScripts
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj);

    // handle script tags
    // todo: why do we do this with native code at all?
    // svgl does not support getElementsByTagName yet :-(
    // => we use visitor
    ElementFilter filter(dom::String::createString("script"));
    filter.visitDocument(nthis);
    for (ElementFilter::Output::iterator it=filter.output.begin();it!=filter.output.end();++it) {
      dom::Element* element=*it;
      element->normalize();
      const dom::NodeList* nl=element->getChildNodes();
      for (unsigned i=0;i<nl->getLength();++i) {
	dom::Node* node;
	dom::Text* text;
	if ((node=nl->item(i))&&((text=dynamic_cast<dom::Text *>(node)))) {
	  std::cerr << "  node name:  " << (*node->getNodeName())      << std::endl;
	  std::cerr << "  node type:  " << ((int)node->getNodeType())  << std::endl;
	  std::cerr << "  node value: " << (*node->getNodeValue()) << std::endl;
	  dom::String* script=node->getNodeValue();
	  EJS_CHECK(JSVAL_IS_OBJECT(argv[0]));
	  if (script->getType()==dom::String::string_utf16) {
	    const jschar* str=script->as_utf16();
	    EJS_CHECK(str);
	    EJS_CHECK_TRUSTED(cx,obj);
	    // todo: is length really correct?
	    // todo: filename and line number
	    jsval dummy;
	    if (!JS_EvaluateUCScript(cx, JSVAL_TO_OBJECT(argv[0]), str, script->getLength(),NULL,0,&dummy))
	      return JS_FALSE;
	  }else if (script->getType()==dom::String::string_lat1) {
	    const char* str=script->as_lat1();
	    EJS_CHECK(str);
	    EJS_CHECK_TRUSTED(cx,obj);
	    // todo: is length really correct?
	    // todo: filename and line number
	    jsval dummy;
	    if (!JS_EvaluateScript(cx, JSVAL_TO_OBJECT(argv[0]), str, script->getLength(),NULL,0,&dummy))
	      return JS_FALSE;
	  }else{
	    EJS_WARN("encoding not yet supported: "<< script->getType());
	  }
	}
      }
    }
    return JS_TRUE;
  }
  
  
#undef GET_NTHIS

#define FUNC(name, args) { #name,svgdocument_##name,args,0,0},

  static JSFunctionSpec svgdocument_methods[] = {
#include "nodefuncs.h"
    FUNC(createTextNode,1)
    FUNC(createElement,1)
    FUNC(createElementNS,2)
    FUNC(addSample,0)
    FUNC(getElementById,1)
    FUNC(getDocumentElement,0)
    FUNC(_handleScripts,1)
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  svgdocument_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // todo
      EJS_THROW_ERROR(cx,obj,"not yet implemented");
    }
    svg::SVGDocument* doc=NULL;

    try{
      if (argc==1) {
	// create from string or stream (problem with stream: svgl wants a istream and not a streambuf)

	// from string
	dom::String* value=NULL;
	if (!jsToDomStringLat1(cx,argv[0],value)) return JS_FALSE;

	svgl::Parser parser;
	if ((doc=parser.parseFromString(value)))
	  // todo: remove?
	  doc->updateStyle();
      }else{
	// create new empty document
	doc=new svg::SVGDocument();
      }
      // todo: improve error handling
    }catch(const std::exception& e){
      if (doc) {delete doc;doc=NULL;};
      EJS_THROW_ERROR(cx,obj,e.what());
    }catch(...){
      if (doc) {delete doc;doc=NULL;};
      EJS_THROW_ERROR(cx,obj,"unknown native exception");
    }
    if (!doc) EJS_THROW_ERROR(cx,obj,"failed to create document");
    if (!JS_SetPrivate(cx,obj,(void *)doc)) return JS_FALSE;
    return JS_TRUE;
  }

  static
  void
  svgdocument_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &svgdocument_class);
    svg::SVGDocument* nthis=(svg::SVGDocument *)JS_GetPrivate(cx,obj);
    if (!nthis) return;
    //    delete nthis;
  }

  JSBool
  ejssvgdocument_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *svgdocument = JS_InitClass(cx, global,
					 NULL,
					 &svgdocument_class,
					 svgdocument_cons, 0,
					 NULL, svgdocument_methods,
					 NULL, NULL);
    if (!svgdocument) return JS_FALSE;
    return JS_TRUE;
  }
}

JSBool
ejssvgdocument_GetNative
(JSContext* cx, JSObject * obj, svg::SVGDocument* &native)
{
  EJS_CHECK_CLASS(cx, obj, svgdocument_class);
  native=(svg::SVGDocument *)JS_GetPrivate(cx,obj);
  if (!native)
    EJS_THROW_ERROR(cx,obj,"no valid svgdocument object");
  return JS_TRUE;
}
