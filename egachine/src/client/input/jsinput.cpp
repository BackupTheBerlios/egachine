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
   \file input/jsinput.cpp
   \brief Javascript wrapper of input layer
   \author Jens Thiele
*/

#include "jsinput.h"
#include "input.h"
#include "ecmascript.h"

extern "C" {
  ECMA_BEGIN_STATIC_VOID_FUNC_VOID(poll) 
  {
    try{
      Input::poll();
    }catch(const Input::CallbackError &error){
      // todo: perhaps use the error
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  ECMA_BEGIN_VOID_FUNC(charMode) 
  {
    ECMA_CHECK_NUM_ARGS(1);
    JSBool b;
    if (!JS_ValueToBoolean(ECMAScript::cx, argv[0],&b))
      ECMA_ERROR("argument 0 must be a boolean");
    Input::charInput(b==JS_TRUE);
    return JS_TRUE;
  }
}

static JSFunctionSpec static_methods[] = {
  ECMA_FUNCSPEC(poll,0),
  ECMA_FUNCSPEC(charMode,1),
  ECMA_END_FUNCSPECS
};

bool
JSInput::init()
{
  JSObject *obj = JS_DefineObject(ECMAScript::cx, ECMAScript::glob,
				  "Input", NULL, NULL,
				  JSPROP_ENUMERATE);
  if (!obj) return JS_FALSE;
  return JS_DefineFunctions(ECMAScript::cx, obj, static_methods);
}

bool
JSInput::deinit()
{
  return true;
}
