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
   \file video/jsvideo.cpp
   \brief Javascript wrapper of video interface
   \author Jens Thiele
*/

#include "jsvideo.h"
#include "video.h"
#include "ecmascript.h"
#include <cassert>
#include "error.h"
#include "jsgl/jsgl.h"

extern "C" {
  ECMA_VOID_FUNC_VOID  (Video,swapBuffers);
  
  ECMA_BEGIN_FUNC(createTexture){
    ECMA_CHECK_NUM_ARGS(1);

    char* ctype;
    size_t len;
    ECMA_STRING_TO_CHARVEC(argv[0],ctype,len);

    jsdouble tid=Video::createTexture(len,ctype);
    // todo simply return number
    if (!JS_NewNumberValue(cx,tid,rval)) return JS_FALSE;
    return JS_TRUE;
  }

  ECMA_BEGIN_VOID_FUNC(drawTexture) 
  {
    if (argc<1) ECMA_ERROR("at least one argument required");
    if (argc>3) ECMA_ERROR("too much arguments (3 at max)");
    jsdouble tid;
    jsdouble w,h;
    w=h=1;
    if (!JS_ValueToNumber(cx,argv[0],&tid)) ECMA_ERROR("Argument 0 is not a number");
    if (argc>=2) JS_ValueToNumber(cx,argv[1],&w);
    if (argc>=3) JS_ValueToNumber(cx,argv[2],&h);
    Video::drawTexture(tid,w,h);
    return JS_TRUE;
  }

  ECMA_BEGIN_VOID_FUNC(drawText){
    if (!argc) ECMA_ERROR("at least one argument required");
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;
    if (!JS_AddRoot(cx,strtype)) return JS_FALSE;
    
    bool hcentered=false;
    bool vcentered=false;
    if ((argc>=2)&&(JSVAL_IS_BOOLEAN(argv[1])))
      hcentered=JSVAL_TO_BOOLEAN(argv[1]);
    if ((argc>=3)&&(JSVAL_IS_BOOLEAN(argv[2])))
      hcentered=JSVAL_TO_BOOLEAN(argv[2]);
    Video::drawText(ctype,hcentered,vcentered);

    JS_RemoveRoot(cx,strtype);
    return JS_TRUE;
  }

  ECMA_BEGIN_VOID_FUNC(setViewport){
    ECMA_CHECK_NUM_ARGS(1);

    if (!JSVAL_IS_OBJECT(argv[0])) ECMA_ERROR("object required as argument");
    JSObject *pobj=JSVAL_TO_OBJECT(argv[0]);
    if (!pobj) ECMA_ERROR("object required as argument");
    jsval v[4];
    jsdouble d[4];
    
#define GETPROP(x,y) do{\
if ((!JS_GetProperty(cx, pobj, x, &(v[y])))||(v[y]==JSVAL_VOID)) ECMA_ERROR("object property "x" missing"); \
if (!JS_ValueToNumber(cx, v[y], &(d[y]))) ECMA_ERROR("object property "x" is not a number"); \
}while(0)

    GETPROP("x",0);
    GETPROP("y",1);
    GETPROP("w",2);
    GETPROP("h",3);
#undef GETPROP

    Video::setViewport(Video::Rectangle(d[0],d[1],d[2],d[3]));

    return JS_TRUE;

  }
  ECMA_BEGIN_FUNC(getViewport){
    ECMA_CHECK_NUM_ARGS(0);
    Video::Rectangle r(Video::getViewport());
    JSObject * resobj=JS_NewObject(cx, 0, 0, 0);
    *rval=OBJECT_TO_JSVAL(resobj);
#define SETPROP(x) do{\
    if (!JS_NewNumberValue(cx,r.x,&argv[0])) ECMA_ERROR("should not happen"); \
    if (!JS_DefineProperty(cx, resobj, #x, argv[0],NULL, NULL, JSPROP_ENUMERATE)) ECMA_ERROR("should not happen");\
    }while(0)
      
    SETPROP(x);
    SETPROP(y);
    SETPROP(sx);
    SETPROP(sy);
    
#undef SETPROP
    
    return JS_TRUE;
  }

  ECMA_BEGIN_FUNC(project){
    ECMA_CHECK_NUM_ARGS(2);
    return JS_TRUE;

  }
}

#undef JSFUNC

using namespace ECMAScript;

static JSFunctionSpec static_methods[] = {
  ECMA_FUNCSPEC(swapBuffers,0),
  ECMA_FUNCSPEC(createTexture,1),
  ECMA_FUNCSPEC(drawTexture,1),
  ECMA_FUNCSPEC(drawText,3),
  ECMA_FUNCSPEC(setViewport,1),
  ECMA_FUNCSPEC_EXTRA(getViewport,0,1),
  ECMA_FUNCSPEC(project,2),
  ECMA_END_FUNCSPECS
};

static JSClass video_class = {
  "Video",0,
  JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,JS_PropertyStub,
  JS_EnumerateStub,JS_ResolveStub,JS_ConvertStub,JS_FinalizeStub,
  ECMA_END_CLASS_SPEC
};

bool
JSVideo::init()
{
  JSObject *obj = JS_DefineObject(cx, glob, "Video", &video_class, NULL,
				  JSPROP_ENUMERATE);
  if (!obj) return false;
  if (!JS_DefineFunctions(cx, obj, static_methods)) return false;
  return JSGL::init();
}

bool
JSVideo::deinit()
{
  return JSGL::deinit();
}
