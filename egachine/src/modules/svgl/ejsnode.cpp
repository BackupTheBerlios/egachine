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
  \brief Javascript wrapper for node objects (using sinle JSCLASS)
  \author Jens Thiele
*/

#include "strutils.h"
#include <cassert>
#include <typeinfo>
#include <map>
#include "ejsnode.h"
#include "ejsnodelist.h"

#include <w3c/svg/SVGDocument.hpp>
#include <w3c/dom/Element.hpp>

// todo: gc? js and libgc
static JSObject* nodeProto = NULL;
typedef std::map<std::string, JSObject* > Prototypes;
static Prototypes prototypes;

// todo: could use a bimap here but
// we don't need reverse lookups very often
typedef std::map<dom::Node*, JSObject*> Wrappers;
static Wrappers wrappers;

//! nearly a duplicate of JS_InitClass
/*!
  the difference is that the constructor name is not taken from
  clasp
*/
JSObject*
ejs_InitClass(JSContext *cx, JSObject *obj, JSObject *parent_proto,
	      JSClass *clasp, 
	      const char* consname, JSNative constructor, uintN nargs,
	      JSPropertySpec *ps, JSFunctionSpec *fs,
	      JSPropertySpec *static_ps, JSFunctionSpec *static_fs)
{
  // FIXME: HACK
  // we want to use the same [[class]] but a different constructor
  // and prototype
  // alternatively we could nearly duplicate JS_InitClass (but perhaps
  // this would also require to use non-public spidermonkey API functions)
  const char *tmp=clasp->name;
  clasp->name=consname;
  JSObject* ret=NULL;
  ret=JS_InitClass(cx,obj,parent_proto, clasp,
			  constructor, nargs, ps, fs,
			  static_ps, static_fs);
  clasp->name=tmp;
  return ret;
}

//! register prototype for a sub-class of dom::Node
void
ejs_registerPrototype(const std::type_info &t, JSObject *proto)
{
  prototypes[t.name()]=proto;
}

#define GET_NTHIS dom::Node* nthis=NULL;		\
  if (!ejsnode_GetNative(cx,jsthis,argv,nthis)) return JS_FALSE

JSBool
jsvalToNode(JSContext* cx, JSObject* jsthis, jsval jsnode, dom::Node* &result);

extern "C" {

  static
  void
  node_finalize(JSContext* cx, JSObject* obj);

  static JSBool
  node_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

  static JSBool
  node_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp);

  // keep enum, node_props and node_getProperty in sync!
  enum node_propid {
    CHILD_NODES,
    NODE_NAME,
    NODE_VALUE,
    NODE_TYPE
  };

  static JSPropertySpec node_props[] = {
    {"childNodes", CHILD_NODES, JSPROP_READONLY|JSPROP_SHARED},
    {"nodeName",   NODE_NAME,   JSPROP_READONLY},
    {"nodeValue",  NODE_VALUE,  JSPROP_SHARED},
    {"nodeType",   NODE_TYPE,   JSPROP_READONLY},
    {0}
  };

  static
  JSClass node_class = {
    "Node",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, node_getProperty, node_setProperty,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,   node_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

  JSBool
  node_appendChild
  (JSContext* cx, JSObject* jsthis, uintN argc, jsval* argv, jsval* rval)
  {
    GET_NTHIS;
    EJS_CHECK_NUM_ARGS(cx,jsthis,1,argc);
    dom::Node* child=NULL;
    if (!jsvalToNode(cx,jsthis,argv[0],child)) return JS_FALSE;
    assert(child);

    // todo: exceptions
    //  EJS_INFO(child);
    nthis->appendChild(child);

    *rval=argv[0];
    return JS_TRUE;
  }

  JSBool
  node_removeChild
  (JSContext* cx, JSObject* jsthis, uintN argc, jsval* argv, jsval* rval)
  {
    GET_NTHIS;
    EJS_CHECK_NUM_ARGS(cx,jsthis,1,argc);
    dom::Node* child=NULL;
    if (!jsvalToNode(cx,jsthis,argv[0],child)) return JS_FALSE;
    assert(child);

    // check if this node has hot this child
    dom::Node *node=nthis->getFirstChild();
    while (node) {
      if (node==child) {
	// todo: exceptions
	nthis->removeChild(child);
	return JS_TRUE;
      }
      node=node->getNextSibling();
    }
    EJS_THROW_ERROR(cx, jsthis, "not my child");
  }

  JSBool
  node_setNodeValue
  (JSContext* cx, JSObject* jsthis, uintN argc, jsval* argv, jsval* rval)
  {
    GET_NTHIS;
    EJS_CHECK_NUM_ARGS(cx,jsthis,1,argc);

    dom::String* value=NULL;
    if (!jsToDomString(cx,argv[0],value)) return JS_FALSE;

    try{
      nthis->setNodeValue(value);
    }catch(const dom::DOMException &e){
      EJS_THROW_ERROR(cx, jsthis, e.what());
    }
    return JS_TRUE;
  }

  JSBool
  node_getNodeValue
  (JSContext* cx, JSObject* jsthis, uintN argc, jsval* argv, jsval* rval)
  {
    GET_NTHIS;
    try{
      dom::String* value=nthis->getNodeValue();
      if (!DomStringToJsval(cx, value, rval)) return JS_FALSE;
    }catch(const dom::DOMException &e){
      EJS_THROW_ERROR(cx, jsthis, e.what());
    }
    return JS_TRUE;
  }

  JSBool
  node_normalize
  (JSContext* cx, JSObject* jsthis, uintN, jsval* argv, jsval*)
  {
    GET_NTHIS;
    try{
      nthis->normalize();
    }catch(const dom::DOMException &e){
      EJS_THROW_ERROR(cx, jsthis, e.what());
    }
    return JS_TRUE;
  }

  JSBool
  node_getNodeName
  (JSContext* cx, JSObject* jsthis, uintN, jsval* argv, jsval* rval)
  {
    GET_NTHIS;
    try{
      dom::String* value=nthis->getNodeName();
      if (!DomStringToJsval(cx, value, rval)) return JS_FALSE;
    }catch(const dom::DOMException &e){
      EJS_THROW_ERROR(cx, jsthis, e.what());
    }
    return JS_TRUE;
  }

#undef GET_NTHIS

#define FUNC(name, args) { #name,node_##name,args,0,0},

  static JSFunctionSpec node_methods[] = {
#include "nodefuncs.h"
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  node_cons
  (JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_THROW_ERROR(cx, obj, "not allowed");
    return JS_TRUE;
  }

  static
  void
  node_finalize(JSContext* cx, JSObject* obj)
  {
    assert(JS_GET_CLASS(cx, obj) == &node_class);
    dom::Node* node=(dom::Node *)JS_GetPrivate(cx,obj);
    if (!node) return;
    
    // reverse lookup in wrappers (could be improved by bimap)
    Wrappers::iterator it=wrappers.begin();
    while (it!=wrappers.end()) {
      if (it->second == obj) {
	wrappers.erase(it);
	return;
      }
      ++it;
    }
    EJS_INFO("not in map");
  }

  static JSBool
  node_getProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    EJS_CHECK_CLASS(cx, obj, node_class);

    //    EJS_INFO(JS_GetStringBytes(JS_ValueToString(cx,id)));
    
    if (!JSVAL_IS_INT(id)) return JS_TRUE;
    jsint slot=JSVAL_TO_INT(id);

    dom::Node* nthis=(dom::Node *)JS_GetPrivate(cx,obj);
    if (!nthis) return JS_TRUE;
    
    switch (slot) {
    case CHILD_NODES:
      {
	// create and return NodeList wrapper object
	const dom::NodeList* nl=nthis->getChildNodes();
	assert(nl);
	JSObject* r=ejs_NewNodeList(cx, obj, nl);
	if (!r) return JS_FALSE; // <- todo: right thing to report error in property hook?
	*vp=OBJECT_TO_JSVAL(r);
	break;
      }
    case NODE_NAME:
      {
	dom::String* nodeName=nthis->getNodeName();
	if (!DomStringToJsval(cx, nodeName, vp)) return JS_FALSE;
	break;
      }
    case NODE_VALUE:
      {
	dom::String* nodeValue=nthis->getNodeValue();
	if (!DomStringToJsval(cx, nodeValue, vp)) return JS_FALSE;
	break;
      }
    case NODE_TYPE:
      {
	dom::Node::NodeType t=nthis->getNodeType();
	*vp=INT_TO_JSVAL(t);
	break;
      }
    default:
      break;
    }
    return JS_TRUE;
  }

  static JSBool
  node_setProperty(JSContext *cx, JSObject *obj, jsval id, jsval *vp)
  {
    EJS_CHECK_CLASS(cx, obj, node_class);
    
    if (!JSVAL_IS_INT(id)) return JS_TRUE;
    jsint slot=JSVAL_TO_INT(id);

    dom::Node* nthis=(dom::Node *)JS_GetPrivate(cx,obj);
    if (!nthis) return JS_TRUE;
    
    switch (slot) {
    case NODE_VALUE:
      {
	dom::String* value=NULL;
	if (!jsToDomString(cx,*vp,value)) return JS_FALSE;
	try{
	  nthis->setNodeValue(value);
	}catch(const dom::DOMException &e){
	  EJS_THROW_ERROR(cx, obj, e.what());
	}
	break;
      }
    default:
      break;
    }
    return JS_TRUE;
  }

  JSBool
  ejsnode_onLoad(JSContext* cx, JSObject* module)
  {
    nodeProto = JS_InitClass(cx, module,
			     NULL,
			     &node_class,
			     node_cons, 0,
			     node_props, node_methods,
			     NULL, NULL);
    if (!nodeProto) return JS_FALSE;
    ejs_registerPrototype(typeid(dom::Node), nodeProto);
    return JS_TRUE;
  }
}

JSObject*
ejs_getPrototype(const std::type_info &t)
{
  Prototypes::iterator it=prototypes.find(t.name());
  if (it!=prototypes.end()) return it->second;
  return NULL;
}

JSObject*
ejs_getPrototype(dom::Node* node)
{
  JSObject* proto=ejs_getPrototype(typeid(*node));
  if (proto) return proto;
  
  // TODO: shit
  // http://www-h.eng.cam.ac.uk/help/tpl/languages/C++/Thinking_in_C++/tic0295.html ?

  // we could also use node->getNodeType()

  if (dynamic_cast<svg::SVGDocument*>(node))
    return ejs_getPrototype(typeid(svg::SVGDocument));

  if (dynamic_cast<dom::Element*>(node))
    return ejs_getPrototype(typeid(dom::Element));
  
  proto=ejs_getPrototype(typeid(dom::Node));
  if (!proto) {
    EJS_ERROR("did not find prototype: "<<typeid(dom::Node).name()<<" "<< typeid(*node).name());
    EJS_FATAL("see above");
  }
  return proto;
}

//! wrap any node
JSObject*
ejs_WrapNode(JSContext* cx, JSObject* obj, dom::Node* node, JSObject* proto)
{
  assert(node);
  Wrappers::iterator it=wrappers.find(node);
  if (it!=wrappers.end()) return it->second;
  
  // todo: should we set parent?
  // this object is not rooted !!

  if (!proto) proto=ejs_getPrototype(node);
  assert(proto);
  
  JSObject* res=JS_NewObject(cx, &node_class, proto, obj);

  // todo: report error
  EJS_CHECK(res);
  if (!JS_SetPrivate(cx,res,(void* )node)) return NULL;
  wrappers[node]=res;
  return res;
}

JSBool
ejsnode_GetNative(JSContext* cx, JSObject* obj, jsval* argv, dom::Node* &native)
{
  EJS_CHECK_CLASS4(cx, obj, node_class, argv);
  native=(dom::Node* )JS_GetPrivate(cx,obj);
  assert(native);
  return JS_TRUE;
}


JSObject*
ejs_InitNodeSubClass(JSContext *cx, JSObject *obj,
		     const std::type_info &t,
		     const char* consname, JSNative constructor, uintN nargs,
		     JSPropertySpec *ps, JSFunctionSpec *fs,
		     JSPropertySpec *static_ps, JSFunctionSpec *static_fs)
{
  assert(nodeProto);
  JSObject* proto=ejs_InitClass(cx,obj,nodeProto,
				&node_class,
				consname, constructor, nargs,
				ps, fs, static_ps, static_fs);
  if (!proto) return NULL;
  ejs_registerPrototype(t, proto);
  return proto;
}

JSClass&
ejs_getNodeClass()
{
  return node_class;
}

JSBool
jsvalToNode(JSContext* cx, JSObject* jsthis, jsval jsnode, dom::Node* &result)
{
  JSObject* jsobj=NULL;
  if ((!JSVAL_IS_OBJECT(jsnode))||(!(jsobj=JSVAL_TO_OBJECT(jsnode))))
    EJS_THROW_ERROR(cx,jsthis,"no object");
  
  EJS_CHECK_CLASS(cx, jsobj, node_class);
  result=(dom::Node *)JS_GetPrivate(cx,jsobj);
  return JS_TRUE;
}
