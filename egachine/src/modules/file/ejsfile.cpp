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
   \file file/ejsfile.cpp
   \brief Wrapper around std::filebuf
   \author Jens Thiele
*/

#include <fstream>
#include "ejsmodule.h"


// this code is also in ejsnet.cpp
JSBool
newStreamObject(JSContext* cx, JSObject* obj, std::streambuf* stream, jsval* rval)
{
  // todo: this is quite ugly
  // create javascript Stream object and set stream pointer
  // the difficulty is that we do not have any native reference
  // => we must use the interpreter to indirectly create a native Stream
  // wrapper object and set the private data accordingly
  // this is only safe if we know that the created javascript object 
  // is of the correct class (stream_class) otherwise this would be dangerous

  std::string script("new Stream()");
  jsval streamval;
  if (!JS_EvaluateScript(cx, obj, 
			 script.c_str(), script.length(),
			 __PRETTY_FUNCTION__ , 1,
			 &streamval))
    return JS_FALSE;
  // todo: is this enough to root this object?
  *rval=streamval;
  if (!JSVAL_IS_OBJECT(streamval))
    EJS_THROW_ERROR(cx,obj,"failed to create Stream object");
  JSObject* jsstream=JSVAL_TO_OBJECT(streamval);
  // make sure this object has a private slot and it is NULL
  JSClass* oclass;
  if ((!(oclass=JS_GET_CLASS(cx,jsstream)))
      || (!(oclass->flags & JSCLASS_HAS_PRIVATE))
      || (JS_GetPrivate(cx,jsstream))
      || (std::string("Stream")!=oclass->name))
    EJS_THROW_ERROR(cx,obj,"you have messed with the Stream object");
  
  if (!JS_SetPrivate(cx,jsstream,(void *)stream))
    return JS_FALSE;
  // tell the stream object wether to delete this streambuf
  JS_SetReservedSlot(cx,jsstream,0,JSVAL_TRUE);
  
  return JS_TRUE;
}

extern "C" {

  //! wrapper function to open a file
  /*!
    the javascript function returns a Stream object
  */
  static
  JSBool
  ejsfile_open(JSContext* cx, JSObject* obj, uintN argc, jsval* argv, jsval* rval) 
  {
    EJS_CHECK_NUM_ARGS(cx,obj,2,argc);
    EJS_CHECK_TRUSTED(cx,obj);
    
    // todo: we have to root the result string
    JSString *strtype=JS_ValueToString(cx, argv[0]);
    if (!strtype) return JS_FALSE;
    // todo: we loose unicode information here
    char* ctype=JS_GetStringBytes(strtype);
    if (!ctype) return JS_FALSE;

    int32 mode;
    if (!JS_ValueToInt32(cx,argv[1],&mode)) return JS_FALSE;

    // ugly
    // hmm how to initialize this to an empty set? (protable)
    // it seems the standard only allows to use |
    // but then & should work, too ;-)
    std::ios_base::openmode cppmode=std::ios_base::in & std::ios_base::out;

    // keep those in sync with file.js
    if (mode& 1) cppmode|=std::ios_base::in;
    if (mode& 2) cppmode|=std::ios_base::out;
    if (mode& 4) cppmode|=std::ios_base::trunc;
    if (mode& 8) cppmode|=std::ios_base::app;
    if (mode&16) cppmode|=std::ios_base::binary;
    if (mode&32) cppmode|=std::ios_base::ate;
    
    // open the file
    std::filebuf* stream=new std::filebuf();
    if (!stream)
	EJS_THROW_ERROR(cx,obj,"Failed to create native object");      
    stream->open(ctype,cppmode);
    if (!newStreamObject(cx, obj, stream, rval))
      return JS_FALSE;
    return JS_TRUE;
  }

#define FUNC(name, args) { #name,ejsfile_##name,args,0,0}

  static JSFunctionSpec net_static_methods[] = {
    FUNC(open,2),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsfile_LTX_onLoad(JSContext *cx, JSObject *global)
  {
    JSObject *obj = JS_DefineObject(cx, global,
				    "File", NULL, NULL, JSPROP_ENUMERATE);
    if (!obj) return JS_FALSE;
    if (!JS_DefineFunctions(cx, obj, net_static_methods)) return JS_FALSE;
    return JS_TRUE;
  }

  JSBool
  ejsfile_LTX_onUnLoad()
  {
    return JS_TRUE;
  }

}
