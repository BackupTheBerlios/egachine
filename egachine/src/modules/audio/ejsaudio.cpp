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
  \file audio/jsaudio.cpp
  \brief javascript wrapper of audio layer
  \author Jens Thiele
*/

#include <cassert>
#include "ejsmodule.h"
#include "audio.h"

#define STRING_TO_CHARVEC(val, ctype, len) do {			\
    JSString *strtype=JS_ValueToString(cx, val);		\
    if (!strtype) return JS_FALSE;				\
    if (!(ctype=JS_GetStringBytes(strtype))) return JS_FALSE;	\
    len=JS_GetStringLength(strtype);				\
  }while(0)


extern "C" {

  static
  JSBool
  playMusic(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;
    if((argc<1)||(argc>3)) EJS_THROW_ERROR(cx,obj,"Between 1 and 3 arguments required");

    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);

    jsdouble volume=1;
    JSBool repeat=JS_FALSE;
    if (argc>=2)
      if (!JS_ValueToNumber(cx,argv[1],&volume)) return JS_FALSE;
    if (argc>=3)
      if (!JS_ValueToBoolean(cx, argv[2],&repeat)) return JS_FALSE;
    Audio::audio->playMusic(ctype,len,volume,repeat==JS_TRUE);
    return JS_TRUE;
  }

  static
  JSBool
  loadSample(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;

    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);

    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);

    Audio::SID sid=Audio::audio->loadSample(ctype,len);
    return JS_NewNumberValue(cx,sid,rval);
  }

  static
  JSBool
  playSample(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;

    if((argc<1)||(argc>2)) EJS_THROW_ERROR(cx,obj,"Between 1 and 2 arguments required");
    jsdouble sid;
    if (!JS_ValueToNumber(cx,argv[0],&sid)) return JS_FALSE;
    jsdouble repeat;
    if (argc>=2)
      if (!JS_ValueToNumber(cx,argv[1],&repeat)) return JS_FALSE;
    Audio::CID cid=Audio::audio->playSample(sid,repeat);
    return JS_NewNumberValue(cx,cid,rval);
  }

#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(playMusic,3),
    FUNC(loadSample,1),
    FUNC(playSample,2),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsaudio_LTX_onLoad(JSContext* cx, JSObject* global)
  {
    // todo: get from javascript / config file
    AudioConfig ac;
    Audio::init(ac);

    JSObject *obj = JS_DefineObject(cx, global, "Audio", NULL, NULL,
				    JSPROP_ENUMERATE);
    if (!obj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, obj, static_methods)) return JS_FALSE;
    return JS_TRUE;
  }

  JSBool
  ejsaudio_LTX_onUnLoad()
  {
    Audio::deinit();
    return JS_TRUE;
  }
}
