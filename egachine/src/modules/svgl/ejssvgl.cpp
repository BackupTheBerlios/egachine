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

#include <w3c/svg/SVGDocument.hpp>
#include <w3c/svg/SVGSVGElement.hpp>
#include <w3c/svg/SVGRectElement.hpp>

#include "ejsmodule.h"

// TODO: this is probably not portable
#include "GL/gl.h"
#include "GL/glu.h"

svg::SVGDocument*
createSampleDoc()
{
  svg::SVGDocument * doc = new svg::SVGDocument();
  svg::SVGSVGElement * thesvgelt = new svg::SVGSVGElement(doc);
  thesvgelt->setWidth(450);
  thesvgelt->setHeight(450);

  doc->appendChild(thesvgelt);

  svg::SVGRectElement * rect = new svg::SVGRectElement(doc);

  double w=100,h=100;
  double x = 100;
  double y = 100;

  rect->setX(w/2);
  rect->setY(h/2);
  rect->setWidth(w);
  rect->setHeight(h);
  //rect->setFill(0,0,1);
  rect->setFill("blue");
  rect->setStroke(0,0,0);
  rect->setStrokeWidth(4);
  rect->setOpacity(.75);
      
  // order independant
  //rect->scale(.5);
  //rect->rotate(20);
  //rect->translate(x,y);

  thesvgelt->appendChild(rect);
  //  glutReshapeWindow(450, 450);
  return doc;
}

static svg::SVGDocument * thesvgdoc=NULL;
static svgl::DisplayManager * displayManager=NULL;

extern "C" {


  static JSBool test(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval)
  {
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    displayManager->display(thesvgdoc);
  }


#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec svgl_static_methods[] = {
    FUNC (test, 0),
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


    JSObject *svglobj = JS_DefineObject(cx, global, "svgl", NULL, NULL,
					JSPROP_ENUMERATE);
    if (!svglobj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, svglobj, svgl_static_methods)) return JS_FALSE;

    try {
      svgl::InitHelper * initHelper = svgl::InitHelper::get();
      displayManager = initHelper->displayManager;
      thesvgdoc = createSampleDoc();
    }
    catch (std::exception& e) {
      EJS_THROW_ERROR(cx,global,e.what());
    }
    catch (...) {
      EJS_THROW_ERROR(cx,global,"unknown exception");
    }
    return JS_TRUE;
  }
  
  //! function called before module is unloaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejssvgl_LTX_onUnLoad()
  {
    if (thesvgdoc) {
      delete thesvgdoc;
      thesvgdoc=NULL;
    }
    if (displayManager){
      delete displayManager;
      displayManager=NULL;
    }
    return JS_TRUE;
  }

}
