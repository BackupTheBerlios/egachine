/*
 * Copyright (C) 2006 Jens Thiele <karme@berlios.de>
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
  \brief Javascript wrapper of SDL_Surface, SDL_Image
  \author Jens Thiele
*/

#include "ejsmodule.h"
#include <SDL.h>
#include <SDL_image.h>

extern "C" {

  static
  void
  image_finalize(JSContext *cx, JSObject *obj);

  static
  JSClass image_class = {
    "Image",
    JSCLASS_HAS_PRIVATE,
    JS_PropertyStub,  JS_PropertyStub, JS_PropertyStub, JS_PropertyStub,
    JS_EnumerateStub, JS_ResolveStub,  JS_ConvertStub,  image_finalize,
    JSCLASS_NO_OPTIONAL_MEMBERS
  };

#define GET_IMAGE_OBJ SDL_Surface* image=NULL;			\
    if (JS_GET_CLASS(cx, obj) != &image_class)			\
      EJS_THROW_ERROR(cx,obj,"incompatible object type");	\
    image=(SDL_Surface*)JS_GetPrivate(cx,obj);			\
    if (!image)							\
      EJS_THROW_ERROR(cx,obj,"no valid image object")

  static
  JSBool
  image_width(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
  {
    GET_IMAGE_OBJ;
    return JS_NewNumberValue(cx,image->w,rval);
  }
  
  static
  JSBool
  image_height(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
  {
    GET_IMAGE_OBJ;
    return JS_NewNumberValue(cx,image->h,rval);
  }

  static
  JSBool
  image_hasAlphaChannel(JSContext* cx, JSObject* obj, uintN argc, jsval*, jsval* rval)
  {
    GET_IMAGE_OBJ;
    *rval=BOOLEAN_TO_JSVAL(image->format->Amask!=0);
    return JS_TRUE;
  }

#undef GET_IMAGE_OBJ

#define FUNC(name, args) { #name,image_##name,args,0,0}

  static JSFunctionSpec image_methods[] = {
    FUNC(width,0),
    FUNC(height,0),
    FUNC(hasAlphaChannel,0),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

#define STRING_TO_CHARVEC(val, ctype, len) do {			\
    JSString *strtype=JS_ValueToString(cx, val);		\
    if (!strtype) return JS_FALSE;				\
    if (!(ctype=JS_GetStringBytes(strtype))) return JS_FALSE;	\
    len=JS_GetStringLength(strtype);				\
  }while(0)

  static
  JSBool
  image_cons
  (JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *rval)
  {
    if (!JS_IsConstructing(cx)) {
      // called as function f.e. x=Image() - we act like x=new Test()
      // todo do we really want this?
      if (!(obj=JS_NewObject(cx,&image_class,NULL,NULL))) return JS_FALSE;
      *rval=OBJECT_TO_JSVAL(obj);
    }

    // SDL functions to create Surfaces:
    //
    // SDL_Surface  *SDL_CreateRGBSurface
    // (Uint32  flags,  int width, int height, int depth, Uint32 Rmask, Uint32 Gmask,
    //  Uint32 Bmask, Uint32 Amask);
    //
    // SDL_Surface  *SDL_CreateRGBSurfaceFrom
    // (void  *pixels, int width, int height, int depth, int pitch, Uint32 Rmask,
    //  Uint32 Gmask, Uint32 Bmask, Uint32 Amask);
    //
    // SDL_Surface *SDL_ConvertSurface(SDL_Surface *src, SDL_PixelFormat *fmt, Uint32 flags);
    //
    // SDL_Surface *SDL_LoadBMP(const char *file);

    // SDL Image function to create Surfaces:
    // 
    // SDL_Surface * SDLCALL IMG_LoadTyped_RW(SDL_RWops *src, int freesrc, char *type);
    // SDL_Surface * SDLCALL IMG_Load(const char *file);
    // SDL_Surface * SDLCALL IMG_Load_RW(SDL_RWops *src, int freesrc);

    // We do not want to expose the complete SDL and SDL_Image API into JS to make completely
    // different implementations easy
    // => we only expose IMG_LOAD_RW in some way (Loading images from some kind of byte stream)

    // todo: for now we do load from a string (used as byte array)
    // in the future the images should be readable from a stream
    // a nice have would be to be able to load (rectangular) parts of images
    // f.e parts of very huge (>main memory) image files

    char* ctype;
    size_t len;
    STRING_TO_CHARVEC(argv[0],ctype,len);

    SDL_RWops* rw = SDL_RWFromMem((void *)ctype,len);
    SDL_Surface* image=IMG_Load_RW(rw,true);
    // todo: use SDL_GetError to get better error message
    if (!image) EJS_THROW_ERROR(cx,obj,"Failed to load image");

    return JS_SetPrivate(cx,obj,(void *)image);
  }

  static
  void
  image_finalize(JSContext *cx, JSObject *obj)
  {
    EJS_CHECK(JS_GET_CLASS(cx, obj) == &image_class);
    SDL_Surface* image=(SDL_Surface *)JS_GetPrivate(cx,obj);
    if (!image) return;
    SDL_FreeSurface(image);
  }

  JSBool
  ejsimage_LTX_onLoad(JSContext *cx, JSObject *module)
  {
    JSObject *image = JS_InitClass(cx, module,
				   NULL,
				   &image_class,
				   image_cons, 0,
				   NULL, image_methods,
				   NULL, NULL);
    if (!image) return JS_FALSE;
    return JS_TRUE;
  }
}
