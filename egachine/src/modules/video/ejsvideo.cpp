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
  \brief Javascript wrapper of video interface
  \author Jens Thiele
*/

#include "video.h"
#include "ejsmodule.h"

#define STRING_TO_CHARVEC(val, ctype, len) do {			\
    JSString *strtype=JS_ValueToString(cx, val);		\
    if (!strtype) return JS_FALSE;				\
    if (!(ctype=JS_GetStringBytes(strtype))) return JS_FALSE;	\
    len=JS_GetStringLength(strtype);				\
  }while(0)

extern "C" {
  static
  JSBool
  swapBuffers(JSContext *, JSObject *, uintN, jsval *, jsval *)
  {
    Video::swapBuffers();
    return JS_TRUE;
  }
  
  static
  JSBool
  createTexture(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);
    
    jsdouble tid=Video::createTexture(len,ctype);
    return JS_NewNumberValue(cx,tid,rval);
  }
  
  static
  JSBool
  drawTexture(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    if ((argc<1)||(argc>3)) EJS_THROW_ERROR(cx,obj,"between one and three arguments required");
    jsdouble tid;
    jsdouble w,h;
    w=h=1;
    if (!JS_ValueToNumber(cx,argv[0],&tid)) return JS_FALSE;
    if (argc>=2)
      if (!JS_ValueToNumber(cx,argv[1],&w)) return JS_FALSE;
    if (argc>=3)
      if (!JS_ValueToNumber(cx,argv[2],&h)) return JS_FALSE;
    Video::drawTexture(tid,w,h);
    return JS_TRUE;
  }
  
  static
  JSBool
  drawText(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    if (!argc) EJS_THROW_ERROR(cx,obj,"at least one argument required");
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
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

#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(swapBuffers,0),
    FUNC(createTexture,1),
    FUNC(drawTexture,1),
    FUNC(drawText,3),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsvideo_LTX_onLoad(JSContext* cx, JSObject* global)
  {
    try{
      Video::init();
    }
    catch(const Video::FatalError &error) {
      EJS_THROW_ERROR(cx,global,error.what());
    }
    
    JSObject *obj = JS_DefineObject(cx, global,
				    "Video", NULL, NULL,JSPROP_ENUMERATE);
    if (!obj) return JS_FALSE;
    return JS_DefineFunctions(cx, obj, static_methods);
  }

  JSBool
  ejsvideo_LTX_onUnLoad()
  {
    Video::deinit();
    return JS_TRUE;
  }
}
