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
  stopMusic(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;
    Audio::audio->stopMusic();
    return JS_TRUE;
  }

  static
  JSBool
  playingMusic(JSContext *, JSObject *, uintN, jsval *, jsval *rval)
  {
    *rval=JSVAL_FALSE;
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;
    *rval=Audio::audio->playingMusic() ? JSVAL_TRUE : JSVAL_FALSE;
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
    FUNC(stopMusic,0),
    FUNC(playingMusic,0),
    FUNC(loadSample,1),
    FUNC(playSample,2),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsaudio_LTX_onLoad(JSContext* cx, JSObject* module)
  {
    AudioConfig ac;
    jsval val;

#define GET_STRING(prop) do{						\
      if (!ejs_evalExpression(cx,module,"ejs.config.Audio."#prop,&val))	\
	JS_ClearPendingException(cx);					\
      else{								\
	JSString* s=NULL;char *cstr=NULL;				\
	if ((!(s=JS_ValueToString(cx,val)))				\
	    || (!(cstr=JS_GetStringBytes(s)))) return JS_FALSE;		\
	ac.prop=cstr;							\
      }									\
    }while(0)

    GET_STRING(sdriver);
    GET_STRING(sdevice);
    
#define GET_INT32(prop) do{						\
      int32 i;								\
      if (!ejs_evalExpression(cx,module,"ejs.config.Audio."#prop,&val))	\
	JS_ClearPendingException(cx);					\
      else{								\
	if (!JS_ValueToECMAInt32(cx,val,&i)) return JS_FALSE;		\
	ac.prop=i;							\
      }									\
    }while(0)
    
    GET_INT32(srate);
    GET_INT32(sbits);
    GET_INT32(sbuffers);

    if (!ejs_evalExpression(cx,module,"ejs.config.Audio.stereo",&val))
      JS_ClearPendingException(cx);
    else{
      JSBool jsb;
      if (!JS_ValueToBoolean(cx, val, &jsb)) return JS_FALSE;
      ac.stereo=jsb;
    }

    Audio::init(ac);

    if (!JS_DefineFunctions(cx, module, static_methods)) return JS_FALSE;
    return JS_TRUE;
  }

  JSBool
  ejsaudio_LTX_onUnLoad()
  {
    Audio::deinit();
    return JS_TRUE;
  }
}
