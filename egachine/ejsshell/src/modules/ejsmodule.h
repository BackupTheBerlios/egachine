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
  \file modules/ejsmodule.h
  \brief Module API for the extensible Javascript shell
  \author Jens Thiele

  \todo the API should be a pure C API (no C++)
*/

#ifndef EJSMODULE_H
#define EJSMODULE_H

#include <string>
#include <jsapi.h>
#include <sstream>

//! throw an JavaScript Error exception
/*!
  \todo improve this
*/
#define EJS_THROW_ERROR(cx,obj,msg) do{				\
    return ejs_throw_error(cx,obj,msg);				\
  }while(0)

#define EJS_CHECK_NUM_ARGS(cx,obj,expect,got) do{	\
    if(expect!=got) {					\
      std::ostringstream o;				\
      o << "Wrong number of arguments: "		\
	<< "expected: " << expect << " got: " << got;	\
      EJS_THROW_ERROR(cx,obj,o.str().c_str());		\
    }							\
  }while(0)

#define EJS_END_FUNCTIONSPEC {0,0,0,0,0}

#define EJS_END_CLASS_SPEC 0,0,0,0,0,0,0,0


//! not part of the public api
inline
JSBool
ejs_throw_error(JSContext* cx, JSObject* obj, const char* msg)
{
  std::string script("throw new Error('");			
  std::string op;						
  for (unsigned i=0;msg[i];++i) {				
    switch(msg[i]){						
    case '\\':						
      op+="\\\\";						
      break;							
    case '\'':						
      op+="\\'";						
      break;							
    default:							
      op+=msg[i];						
    }								
  }								
  script+=op;							
  script+="');";						
  jsval dummy;						
  return JS_EvaluateScript(cx, obj,				
			   script.c_str(), script.length(),	
			   NULL, 0, &dummy);			
}

#endif
