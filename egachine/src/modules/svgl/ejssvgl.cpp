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
  \brief svgl module
  \author Jens Thiele

  This module wraps svgl
*/

#include <svgl/InitHelper.hpp>
#include <svgl/DisplayManager.hpp>
#include <svgl/AnimationManager.hpp>
#include <svgl/AnimationInfo.hpp>
#include <svgl/GLInfo.hpp>
#include <svgl/getattr.hpp>
#include <svgl/Context.hpp>
#include <svgl/ExternalEntityManager.hpp>
#include <svgl/PickManager.hpp>

#include <w3c/svg/SVGDocument.hpp>
#include <w3c/svg/SVGSVGElement.hpp>

#include <ejsmodule.h>
#include "ejsallelements.h"
#include "ejstimer.h"

// TODO: this is not portable
#include "GL/gl.h"
#include "GL/glu.h"

static svgl::DisplayManager * displayManager=NULL;
static svgl::InitHelper * initHelper=NULL;

class RedisplayListener : public svgl::Animation::RedisplayEventListener {
public:
  RedisplayListener(JSContext* _cx, JSObject* _obj) : cx(_cx), obj(_obj)
  {}
  
  virtual void doit(const svgl::Animation::RedisplayEvent&) {
    jsval rval;
    // todo: error handling
    if (!JS_CallFunctionName(cx, obj, "redisplay", 0, NULL, &rval))
      EJS_WARN("js error");
  }

protected:
  JSContext* cx;
  JSObject* obj;
};

//! selected document
svg::SVGDocument * csvgdoc=NULL;
//! root svg element of selected document
svg::SVGSVGElement* csvgelt=NULL;

extern "C" {

  static
  JSBool
  selectDocument(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    csvgdoc=NULL;
    if (!JSVAL_IS_OBJECT(argv[0]))
      EJS_THROW_ERROR(cx,obj,"object required as first argument");
    if (!ejssvgdocument_GetNative(cx,JSVAL_TO_OBJECT(argv[0]),argv, csvgdoc))
      return JS_FALSE;
    EJS_CHECK(csvgdoc);

    csvgelt = csvgdoc->GET_SIMPLE_VAL(RootElement);
    if (csvgelt) {
      // todo: why?
      csvgelt->setOwnerAndViewPort(csvgelt, csvgelt);

      csvgdoc->updateStyle();

      EJS_CHECK(initHelper && initHelper->context);
      svgl::Context * svglContext=initHelper->context;
      // todo: get a name / and this should probably be done on document creation
      svglContext->externalEntityManager->register_(csvgdoc,unicode::String::createString(""));

      initHelper->animinfo->animationManager->stop();
      initHelper->animinfo->animationManager->unsubscribe_all();

      csvgelt->animationTraverse(initHelper->animinfo);

      svg::SVGLength widthl = csvgelt->GETVAL(Width);
      svg::SVGLength heightl = csvgelt->GETVAL(Height);
      const svg::SVGRect& viewbox = csvgelt->GETVAL(ViewBox);
      float width, height;
      width = widthl.computeValue(svglContext->dpi, viewbox.getWidth(), svglContext->fontSize, svglContext->fontXHeight);
      height = heightl.computeValue(svglContext->dpi, viewbox.getHeight(), svglContext->fontSize, svglContext->fontXHeight);
      // todo: why do we do this?
      svglContext->setViewportWidthHeight(width, height);
      svglContext->svgOwner = csvgelt;

      // get window size
      float winWidth=initHelper->glinfo->winWidth;
      float winHeight=initHelper->glinfo->winHeight;

      // set zoom and pan to fit (centered and keeping aspect ratio)
      float zoomw=winWidth/width;
      float zoomh=winHeight/height;
      if (zoomw>zoomh) {
	initHelper->glinfo->zoom=zoomh;
	initHelper->glinfo->xpan=-(winWidth/zoomh-width)/2;
      }else{
	initHelper->glinfo->zoom=zoomw;
	initHelper->glinfo->ypan=-(winHeight/zoomw-height)/2;
      }
    }
    return JS_TRUE;
  }

  static
  JSBool
  display(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    if (csvgdoc&&csvgelt&&displayManager)
      displayManager->display(csvgdoc);
    else
      EJS_WARN("you must select a document via selectDocument first");
    return JS_TRUE;
  }

  static
  JSBool
  pick(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK(initHelper);
    svgl::Context* svglContext=initHelper->context;
    svgl::GLInfo* glinfo = initHelper->glinfo;
    EJS_CHECK(svglContext && glinfo);

    // get args
    int32 x,y;
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    if (!JS_ValueToECMAInt32(cx, argv[0], &x)) return JS_FALSE;
    if (!JS_ValueToECMAInt32(cx, argv[1], &y)) return JS_FALSE;
    
    // do work
    svgl::PickManager::PickResult p = initHelper->pickManager->pick(csvgelt, svglContext, glinfo, x,y,5,5);
    if(p.first==p.second) {
      // nothing hit
      *rval=JSVAL_FALSE;
      return JS_TRUE;
    }

    // convert pick result to JS array of arrays

    JSObject *atop=JS_NewArrayObject(cx, 0, NULL);
    if (!atop) return JS_FALSE;
    *rval=OBJECT_TO_JSVAL(atop);

    for(unsigned i=0; p.first!=p.second; ++p.first,++i) {
      JSObject *asub=JS_NewArrayObject(cx, 0, NULL);
      if (!asub) return JS_FALSE;
      jsval v=OBJECT_TO_JSVAL(asub);
      if (!JS_SetElement(cx, atop, i, &v)) return JS_FALSE;
      svgl::PickManager::HitIterator::value_type onehit = *p.first;
      for(unsigned j=0; onehit.first!=onehit.second; ++onehit.first,++j) {
	JSObject* njsobj=ejs_WrapElement(cx,obj,(*onehit.first));
	if (!njsobj) return JS_FALSE;
	jsval val=OBJECT_TO_JSVAL(njsobj);
	if (!JS_SetElement(cx, asub, j, &val)) return JS_FALSE;
      }
    };
    return JS_TRUE;
  }
  
  static
  JSBool
  startAnimation(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    EJS_CHECK(initHelper && initHelper->animinfo && initHelper->animinfo->animationManager);
    initHelper->animinfo->animationManager->start();
    return JS_TRUE;
  }

  static
  JSBool
  stopAnimation(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval*)
  {
    EJS_CHECK(initHelper && initHelper->animinfo && initHelper->animinfo->animationManager);
    initHelper->animinfo->animationManager->stop();
    return JS_TRUE;
  }
  
#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec svgl_static_methods[] = {
    FUNC (selectDocument, 1),
    FUNC (display, 0),
    FUNC (startAnimation, 0),
    FUNC (stopAnimation, 0),
    FUNC (pick, 2),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC


  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejssvgl_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    // todo: remove usage of global where possible

    if (!JS_DefineFunctions(cx, module, svgl_static_methods)) return JS_FALSE;
    JSObject* global=JS_GetGlobalObject(cx);
    try {
      initHelper = svgl::InitHelper::get(new TimeManager(cx,global,0.01), new RedisplayListener(cx,global));
      displayManager = initHelper->displayManager;
      // get window size (todo: we must get window resize events !)
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT,viewport);
      initHelper->glinfo->winWidth=viewport[2];
      initHelper->glinfo->winHeight=viewport[3];
    }
    catch (std::exception& e) {
      EJS_THROW_ERROR(cx,module,e.what());
    }
    catch (...) {
      EJS_THROW_ERROR(cx,module,"unknown exception");
    }

    // register DOM wrappers
    if (!ejsnode_onLoad(cx,module))        return JS_FALSE;
    if (!ejssvgdocument_onLoad(cx,module)) return JS_FALSE;
    if (!ejsnodelist_onLoad(cx,module))    return JS_FALSE;
    if (!ejselement_onLoad(cx,module))     return JS_FALSE;
    return JS_TRUE;
  }
  
  //! function called before module is unloaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejssvgl_LTX_onUnLoad()
  {
    if (displayManager){
      delete displayManager;
      displayManager=NULL;
    }
    return JS_TRUE;
  }

}
