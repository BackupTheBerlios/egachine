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
  \brief zlib wrapper
  \author Jens Thiele

  Wrapper around zlib compress and uncompress functions.

  \todo
  At  the moment the  input and  output strings  are treated  as 8-Bit
  character strings (In fact they are  16-Bit).  Only the lower 8-Bits
  are  used (the upper  8-Bit  are thrown  away).  This will hopefully
  change in the future if I find some time to write a ByteArray Object
  for Javascript.   Another possibility would  be to  try to tell zlib
  that   to  use 16-Bit  chars  -  because   otherwise compression and
  uncompression might result  in a odd   number of bytes  which causes
  trouble with 16-Bit characters.
*/


#include <zlib.h>
#include <ejsmodule.h>
#include <cassert>

#define RETSTR(dest,destLen,rval) do{				\
    if (!dest) return JS_FALSE;					\
    JSString *s=JS_NewString(cx, (char *)dest, destLen);	\
    if (!s) {							\
      JS_free(cx,dest);						\
      return JS_FALSE;						\
    }								\
    *rval=STRING_TO_JSVAL(s);					\
    return JS_TRUE;						\
  }while(0)

//! convert Javascript string to C character array
/*!
  \todo Javascript strings are 2 byte characters
*/
#define STRING_TO_CHARVEC(val, ctype, len) do {			\
    JSString *strtype=JS_ValueToString(cx, val);		\
    if (!strtype) return JS_FALSE;				\
    if (!(ctype=JS_GetStringBytes(strtype))) return JS_FALSE;	\
    len=JS_GetStringLength(strtype);				\
  }while(0)

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_COMPRESSBOUND
#warning did not find compressBound in zlib
  //! old libz does not provide compressBound
  /*!
    [...] size of the destination buffer, which must be at least 0.1% larger than
    sourceLen plus 12 bytes. [...]
  */
  static
  uLong
  compressBound (uLong sourceLen)
  {
    return sourceLen + (sourceLen >> 6) + 12;
  }
#endif


  static
  JSBool
  ejszlib_compress (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    
    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);
    // todo: perhaps simply return an empty string
    if (!len) EJS_THROW_ERROR(cx,obj,"nothing to compress");
    
    uLong destLen=compressBound(len);
    
    Byte* dest=(Byte *)JS_malloc(cx, destLen);
    if (!dest) return JS_FALSE;

    if (compress(dest, &destLen, (Byte*)ctype, len)!=Z_OK) {
      JS_free(cx,dest);
      EJS_THROW_ERROR(cx,obj,"compression failed");
    }

    assert(destLen>0);
    dest=(Byte *)JS_realloc(cx,dest,destLen);

    RETSTR(dest,destLen,rval);
  }

  static
  JSBool
  ejszlib_uncompress (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);

    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);
    // todo: perhaps simply return an empty string
    if (!len) EJS_THROW_ERROR(cx,obj,"nothing to uncompress");

    int32 d;
    if (!JS_ValueToInt32(cx,argv[1],&d)) return JS_FALSE;
    if (d<0) EJS_THROW_ERROR(cx,obj,"Argument 1 must be a postive number");
    uLong destLen=d;

    Byte* dest=(Byte *)JS_malloc(cx, destLen);
    if (!dest) return JS_FALSE;

    int r;
    if ((r=uncompress(dest,  &destLen, (Byte*)ctype, len))!=Z_OK) {
      JS_free(cx,dest);
      std::string error="uncompression failed: ";
      switch(r) {
      case Z_DATA_ERROR:
	error+="DATA_ERROR";
	break;
      case Z_BUF_ERROR:
	error+="wrong size";
	break;
      case Z_MEM_ERROR:
	error+="out of memory";
	break;
      default:
	error+="unknown error";
      };

      EJS_THROW_ERROR(cx,obj,error.c_str());
    }
    assert(destLen>0);
    dest=(Byte *)JS_realloc(cx,dest,destLen);
    RETSTR(dest,destLen,rval);
  }

#define JSFUNC(name, args) { #name,ejszlib_##name,args,0,0}
  static JSFunctionSpec static_methods[] = {
    JSFUNC(compress,1),
    JSFUNC(uncompress,2),
    EJS_END_FUNCTIONSPEC
  };
#undef JSFUNC

  //! function called after module is loaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejszlib_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    return JS_DefineFunctions(cx, module, static_methods);
  }
  
  //! function called before module is unloaded
  /*!
    \return JS_TRUE on success
  */
  JSBool
  ejszlib_LTX_onUnLoad()
  {
    return JS_TRUE;
  }

#ifdef __cplusplus
}
#endif
