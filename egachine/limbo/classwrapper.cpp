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
  \brief Javascript wrapper of SDL_Surface, SDL_Image
  \author Jens Thiele
*/

#include <SDL.h>
#include "ejsmodule.h"

extern "C" {

  static
  JSBool
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

#undef GET_IMAGE_OBJ

#define FUNC(name, args) { #name,image_##name,args,0,0}

  static JSFunctionSpec stream_methods[] = {
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  static
  JSBool
  image_cons
  (JSContext *, JSObject *, uintN, jsval *, jsval *)
  {
    return JS_TRUE;
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
