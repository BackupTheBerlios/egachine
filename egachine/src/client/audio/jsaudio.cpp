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

#include "jsaudio.h"
#include "audio.h"
#include "ecmascript.h"
#include <cassert>
#include "error.h"

extern "C" {
  ECMA_BEGIN_VOID_FUNC(playMusic) 
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;
    
    if((argc<1)||(argc>3)) ECMA_ERROR("Between 1 and 3 arguments required");


    char* ctype;
    size_t len;
    ECMA_STRING_TO_CHARVEC(argv[0],ctype,len);

    jsdouble volume=1;
    JSBool repeat=JS_FALSE;
    if (argc>=2)
      if (!JS_ValueToNumber(cx,argv[1],&volume)) ECMA_ERROR("Argument 1: number required");
    if (argc>=3)
      if (!JS_ValueToBoolean(cx, argv[2],&repeat)) ECMA_ERROR("Argument 2: boolean required");
    Audio::audio->playMusic(ctype,len,volume,repeat==JS_TRUE);
    return JS_TRUE;
  }

  ECMA_BEGIN_FUNC(loadSample) 
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;

    ECMA_CHECK_NUM_ARGS(1);

    char* ctype;
    size_t len;
    ECMA_STRING_TO_CHARVEC(argv[0],ctype,len);

    Audio::SID sid=Audio::audio->loadSample(ctype,len);
    return JS_NewNumberValue(cx,sid,rval);
  }
  ECMA_BEGIN_FUNC(playSample)
  {
    // sound disabled?
    if (!Audio::audio) return JS_TRUE;

    if((argc<1)||(argc>2)) ECMA_ERROR("Between 1 and 2 arguments required");
    jsdouble sid;
    if (!JS_ValueToNumber(cx,argv[1],&sid)) ECMA_ERROR("Argument 0: number required");
    JSBool rep=JS_FALSE;
    if (argc>=2)
      if (!JS_ValueToBoolean(cx,argv[1],&rep)) ECMA_ERROR("Argument 1: boolean required");
    Audio::CID cid=Audio::audio->playSample(sid,rep==JS_TRUE);
    return JS_NewNumberValue(cx,cid,rval);
  }
}

static JSFunctionSpec static_methods[] = {
  ECMA_FUNCSPEC(playMusic,3),
  ECMA_FUNCSPEC(loadSample,1),
  ECMA_FUNCSPEC(playSample,2),
  ECMA_END_FUNCSPECS
};

bool
JSAudio::init()
{
  JSObject *obj = JS_DefineObject(ECMAScript::cx, ECMAScript::glob, "Audio", NULL, NULL,
				  JSPROP_ENUMERATE);
  if (!obj) return false;
  if (!JS_DefineFunctions(ECMAScript::cx, obj, static_methods)) return false;
  return true;
}

bool
JSAudio::deinit()
{
  return true;
}
