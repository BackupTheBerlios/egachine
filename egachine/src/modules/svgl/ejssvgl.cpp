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
#include <svgl/GLInfo.hpp>
#include <svgl/getattr.hpp>
#include <svgl/Context.hpp>

#include <w3c/svg/SVGDocument.hpp>
#include <w3c/svg/SVGSVGElement.hpp>

#include <ejsmodule.h>
#include "ejssvgdocument.h"
#include "ejsnode.h"
#include "ejselement.h"
#include "ejstext.h"


// TODO: this is probably not portable
#include "GL/gl.h"
#include "GL/glu.h"

static svgl::DisplayManager * displayManager=NULL;
static svgl::InitHelper * initHelper=NULL;

extern "C" {


  static
  JSBool
  display(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    svg::SVGDocument* svgdoc=NULL;
    if (!JSVAL_IS_OBJECT(argv[0]))
      EJS_THROW_ERROR(cx,obj,"object required as first argument");
    if (!ejssvgdocument_GetNative(cx,JSVAL_TO_OBJECT(argv[0]),svgdoc))
      return JS_FALSE;

    // get svg size
    const svg::SVGSVGElement * thesvgelt = svgdoc->GET_SIMPLE_VAL(RootElement);
    if (thesvgelt) {
      svg::SVGLength widthl = thesvgelt->GETVAL(Width);
      svg::SVGLength heightl = thesvgelt->GETVAL(Height);
      const svg::SVGRect& viewbox = thesvgelt->GETVAL(ViewBox);
      float width, height;
      svgl::Context * svglContext=initHelper->context;
      width = widthl.computeValue(svglContext->dpi, viewbox.getWidth(), svglContext->fontSize, svglContext->fontXHeight);
      height = heightl.computeValue(svglContext->dpi, viewbox.getHeight(), svglContext->fontSize, svglContext->fontXHeight);
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
    // TODO: remove this
    // svgdoc->updateStyle();

    displayManager->display(svgdoc);
    return JS_TRUE;
  }


#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec svgl_static_methods[] = {
    FUNC (display, 1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC


  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejssvgl_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    // TODO: svgl needs opengl with:
    /*
      GLUT_RGBA | GLUT_DOUBLE | GLUT_STENCIL;
      initDisplayMode |= GLUT_MULTISAMPLE;
      initDisplayMode |= GLUT_ALPHA;
    */

    // test

    JSObject *svglobj = JS_DefineObject(cx, global, "svgl", NULL, NULL,
					JSPROP_ENUMERATE);
    if (!svglobj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, svglobj, svgl_static_methods)) return JS_FALSE;

    try {
      initHelper = svgl::InitHelper::get();
      displayManager = initHelper->displayManager;
      // get window size (todo: we must get window resize events !)
      int viewport[4];
      glGetIntegerv(GL_VIEWPORT,viewport);
      initHelper->glinfo->winWidth=viewport[2];
      initHelper->glinfo->winHeight=viewport[3];
    }
    catch (std::exception& e) {
      EJS_THROW_ERROR(cx,global,e.what());
    }
    catch (...) {
      EJS_THROW_ERROR(cx,global,"unknown exception");
    }

    // register DOM wrappers
    if (!ejssvgdocument_onLoad(cx,global)) return JS_FALSE;
    if (!ejsnode_onLoad(cx,global))        return JS_FALSE;
    if (!ejselement_onLoad(cx,global))     return JS_FALSE;
    if (!ejstext_onLoad(cx,global))        return JS_FALSE;
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
