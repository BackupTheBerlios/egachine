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

#define GET_NTHIS(cx,obj,argv) svg::SVGDocument* nthis=NULL;	\
    ejssvgdocument_GetNative(cx,obj,argv,nthis)

  static
  JSBool
  svgdocument_createTextNode(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj,argv);

    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[0],value)) return JS_FALSE;

    dom::Text* text=nthis->createTextNode(value);
    assert(text);

    JSObject* jstext=ejs_WrapNode(cx,obj,text);
    if (!jstext) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(jstext);
    return JS_TRUE;
  }
  
  static
  JSBool
  svgdocument_createElement(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj,argv);

    dom::String* tag;
    if (!jsToDomString(cx, argv[0], tag)) return JS_FALSE;

    dom::Element* element=nthis->createElement(tag);
    assert(element);
    //    EJS_INFO(tag << " " << element);

    // TODO: perhaps create specialized wrapper object
    JSObject* njsobj=ejs_WrapElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_createElementNS(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    GET_NTHIS(cx,obj,argv);

    dom::String* nsstr=NULL;
    if (!jsToDomString(cx,argv[0],nsstr)) return JS_FALSE;
    dom::String* tag;
    if (!jsToDomString(cx, argv[1], tag)) return JS_FALSE;

    dom::Element* element=nthis->createElementNS(nsstr,tag);
    assert(element);

    JSObject* njsobj=ejs_WrapElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_getElementById
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    GET_NTHIS(cx,obj,argv);

    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[0],value)) return JS_FALSE;
    dom::Element* element=nthis->getElementById(value);
    // handle error !!
    EJS_CHECK(element);

    JSObject* njsobj=ejs_WrapElement(cx,obj,element);
    if (!njsobj) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(njsobj);
    return JS_TRUE;
  }

  static
  JSBool
  svgdocument_getDocumentElement
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,0,argc);
    GET_NTHIS(cx,obj,argv);
    dom::Element* element=nthis->getDocumentElement();
    assert(element);

    JSObject* njsobj=ejs_WrapElement(cx,obj,element);
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
    GET_NTHIS(cx,obj,argv);

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
    FUNC(createTextNode,1)
    FUNC(createElement,1)
    FUNC(createElementNS,2)
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
    // todo
    if (!JS_IsConstructing(cx)) EJS_THROW_ERROR(cx,obj,"not yet implemented");

    svg::SVGDocument* doc=NULL;

    try{
      if (argc==1) {
	// create from string or stream (problem with stream: svgl wants a istream and not a streambuf)

	// from string
	dom::String* value=NULL;
	if (!jsToDomStringLat1(cx,argv[0],value)) return JS_FALSE;

	svgl::Parser parser;
	doc=parser.parseFromString(value);
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
  }

  JSBool
  ejssvgdocument_onLoad(JSContext *cx, JSObject *module)
  {
    JSObject *svgdocument = ejs_InitNodeSubClass(cx, module,
						 typeid(svg::SVGDocument),
						 "SVGDocument",svgdocument_cons, 0,
						 NULL, svgdocument_methods);
    if (!svgdocument) return JS_FALSE;
    return JS_TRUE;
  }
}

JSBool
ejssvgdocument_GetNative
(JSContext* cx, JSObject * obj, jsval* argv, svg::SVGDocument* &native)
{
  dom::Node* node=NULL;
  if (!ejsnode_GetNative(cx, obj, argv, node)) return JS_FALSE;
  if (!(native=dynamic_cast<svg::SVGDocument*>(node)))
    EJS_THROW_ERROR(cx,obj,"no valid dom::SVGDocument object");
  return JS_TRUE;
}
