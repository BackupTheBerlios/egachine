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
   \file common/jszlib.cpp
   \brief zlib wrapper
   \author Jens Thiele
*/


#include <zlib.h>
#include "jszlib.h"
#include "ecmascript.h"
#include "error.h"

#define RETSTR(dest,destLen) \
    if (!dest) return JS_FALSE; \
    JSString *s=JS_NewString(cx, (char *)dest, destLen); \
    if (!s) { \
      JS_free(cx,dest); \
      return JS_FALSE; \
    } \
    *rval=STRING_TO_JSVAL(s); \
    return JS_TRUE;

extern "C" {
  ECMA_BEGIN_FUNC(jszlib_compress) 
  {
    ECMA_CHECK_NUM_ARGS(1);
    
    char* ctype;
    size_t len;
    ECMA_STRING_TO_CHARVEC(argv[0],ctype,len);
    if (!len) ECMA_ERROR("nothing to compress");
    
    uLong destLen=compressBound(len);

    Byte* dest=(Byte *)JS_malloc(cx, destLen);
    if (!dest) return JS_FALSE;

    if (compress(dest, &destLen, (Byte*)ctype, len)!=Z_OK) {
      JS_free(cx,dest);
      ECMA_ERROR("compression failed");
    }

    JGACHINE_CHECK(destLen);
    dest=(Byte *)JS_realloc(cx,dest,destLen);

    RETSTR(dest,destLen);
  }
  ECMA_BEGIN_FUNC(jszlib_decompress) 
  {
    ECMA_CHECK_NUM_ARGS(2);

    char* ctype;
    size_t len;
    ECMA_STRING_TO_CHARVEC(argv[0],ctype,len);
    if (!len) ECMA_ERROR("nothing to decompress");

    int32 d;
    if (!JS_ValueToInt32(cx,argv[1],&d)) return JS_FALSE;
    if (d<0) ECMA_ERROR("Argument 0 must be a postive number");
    uLong destLen=d;

    Byte* dest=(Byte *)JS_malloc(cx, destLen);
    if (!dest) return JS_FALSE;

    int r;
    if ((r=uncompress(dest,  &destLen, (Byte*)ctype, len))!=Z_OK) {
      JS_free(cx,dest);
      ECMA_FERROR("decompression failed: %s", (r==Z_DATA_ERROR ? "DATA_ERROR" : (r==Z_BUF_ERROR ? "wrong size" : "out of memory")));
    }
    dest=(Byte *)JS_realloc(cx,dest,destLen);
    RETSTR(dest,destLen);
  }
}

#define JSFUNC(prefix, name, args) { #name,prefix##name,args,0,0}
static JSFunctionSpec static_methods[] = {
  JSFUNC(jszlib_, compress,1),
  JSFUNC(jszlib_, decompress,2),
  ECMA_END_FUNCSPECS
};


bool 
JSZlib::init()
{
  JSObject *o = JS_DefineObject(ECMAScript::cx, ECMAScript::glob, "Zlib", NULL, NULL,
				JSPROP_ENUMERATE);
  if (!o) return false;
  if (!JS_DefineFunctions(ECMAScript::cx, o, static_methods)) return false;

  return true;
}

bool
JSZlib::deinit()
{
  return true;
}

