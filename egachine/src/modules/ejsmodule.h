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
#include <iostream>

//! throw an JavaScript Error exception
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

#define EJS_CHECK_CLASS(cx,obj,class) do{				\
    if (JS_GET_CLASS(cx, obj) != &class) {				\
      std::ostringstream o;						\
      o << "Wrong object class: "					\
	<< "expected: " << class.name					\
	<< " got: " << JS_GET_CLASS(cx, obj)->name;			\
      EJS_THROW_ERROR(cx, obj, o.str().c_str());			\
    }									\
  }while(0)

#define EJS_CHECK_CLASS4(cx,obj,class,argv) do{				\
    if (!JS_InstanceOf(cx,obj,&class,argv)) return JS_FALSE;		\
  }while(0)

#define EJS_END_FUNCTIONSPEC {0,0,0,0,0}

#define EJS_END_CLASS_SPEC 0,0,0,0,0,0,0,0

//! check that we are in trusted mode
#define EJS_CHECK_TRUSTED(cx,obj) do{			\
    if (!ejs_check_trusted(cx,obj)) return JS_FALSE;	\
  }while(0)

// end of public API -------------------------------------------------

//! evaluate in current scope: "(function(){return "+expr+";})()"
inline
JSBool
ejs_evalExpression(JSContext* cx, JSObject* obj, const char* expr, jsval* rval)
{
  std::string fbody("return ");
  fbody+=expr;
  fbody+=";";
  JSFunction *func;
  JSObject* fobj;
  if (!(func=JS_CompileFunction(cx, obj, NULL,
				0, NULL,
				fbody.c_str(), fbody.length(),	
				NULL, 0))) return JS_FALSE;
  // root function
  if (! (((fobj = JS_GetFunctionObject(func))) && (JS_AddNamedRoot(cx, &fobj, "fobj"))) )
    return JS_FALSE;
  JSBool ret=JS_TRUE;
  if (!JS_CallFunction(cx, obj, func, 0, NULL, rval))
    ret=JS_FALSE;
  if (!JS_RemoveRoot(cx, &fobj))
    ret=JS_FALSE;
  return ret;
}

//! not part of the public api
inline
JSBool
ejs_throw_error(JSContext* cx, JSObject* obj, const char* msg)
{
  JSString* jsstr;
  // if we get errors during error reporting we report those
  if ( ((jsstr=JS_NewStringCopyZ(cx, msg)))
       && (JS_AddNamedRoot(cx,&jsstr,"jsstr")) ) {
    jsval dummy;						
    // We can't use JS_EvaluateScript since the stack would be wrong
    JSFunction *func;
    JSObject* fobj;
    const char* fbody="throw new Error(msg);";
    const char* argnames[]={"msg"};
    if ((func=JS_CompileFunction(cx, obj, NULL,
				 1, argnames,
				 fbody, strlen(fbody),	
				 NULL, 0))) {
      // root function
      if ( ((fobj = JS_GetFunctionObject(func)))
	   && (JS_AddNamedRoot(cx, &fobj, "fobj")) ) {
	jsval args[]={STRING_TO_JSVAL(jsstr)};
	JS_CallFunction(cx, obj, func, 1, args, &dummy);
	JS_RemoveRoot(cx, &fobj);
      }
    }
    JS_RemoveRoot(cx,&jsstr);
  }
  return JS_FALSE;
}

/*
  \def EJS_STR(s)
  \brief EJS_STR converts the macro argument to a string constant
*/
#define EJS_STR(s) #s

/*
  \def EJS_STR(s)
  \brief EJS_STR converts the macro argument to a string constant
  \note this works also if the argument is a macro
*/
#define EJS_XSTR(s) EJS_STR(s)

#define EJS_ERRORSTR "errno="<<jg_errno<<":"<<strerror(jg_errno)
#define EJS_FUNCTIONNAME __PRETTY_FUNCTION__
#define EJS_HERE __FILE__ ":" << __LINE__ << ":" << EJS_FUNCTIONNAME
#define EJS_MSG(ejsLevelP, ejsMsgP, printerrno) do{int jg_errno=errno;errno=0;std::cerr << ejsLevelP << EJS_HERE << ":\n    " << ejsMsgP << "\n";if(printerrno&&jg_errno) std::cerr << "    (" << EJS_ERRORSTR << ")\n";}while(0)
#define EJS_WARN(msg) EJS_MSG("WARNING: ",msg, 1)
#define EJS_INFO(msg) EJS_MSG("INFO: ",msg, 0)
#define EJS_ERROR(msg) EJS_MSG("ERROR: ",msg, 1)

/*
  \def EJS_CHECK(expr)
  \brief EJS_CHECK is like assert but takes also effect when NDEBUG is defined
  
  it seems there is no assert which is not disabled by a \#define NDEBUG 
*/
#define EJS_CHECK(expr) (static_cast<void> ((expr) ? 0 : ejs_fatal(__FILE__, __LINE__, EJS_FUNCTIONNAME, "assertion failed: " EJS_STR(expr))))

/*!
  \def EJS_FATAL(msg)
  \brief fatal error - abort with a short message
*/
#define EJS_FATAL(msg) do{ejs_fatal(__FILE__, __LINE__, EJS_FUNCTIONNAME, msg);}while(0)

inline int ejs_fatal(const char *file,int line,const char *func, const char *msg) 
{
  int jg_errno=errno;
  errno=0;
  std::cerr << "FATAL: " << file << ":" << line << ":" << func << ": " << msg << "(" << EJS_ERRORSTR <<")\n";
  std::terminate();
  return 0;
}

#define EJS_THROW_MESSED_ERROR(cx,obj) EJS_THROW_ERROR(cx,obj,"you have messed with the ejs object")

inline
JSBool ejs_get_ejs_object(JSContext* cx, JSObject* obj, JSObject* &ejsout)
{
  // get ejs object

  // todo: this is quite ugly
  // the difficulty is that we do not have any native reference
  // => we must use the interpreter to indirectly get the ejs object
  // and get the private data (in the reserved slot)
  // this is only safe if we know that the javascript object 
  // is of the correct class (ejs_class) otherwise this would be dangerous
  // another problem is that there must only be one ejs object
  JSObject *global;
  if (!(global=JS_GetGlobalObject(cx))) return JS_FALSE;
  jsval ejsval;
  if (!JS_GetProperty(cx, global, "ejs", &ejsval)) return JS_FALSE;
  JSObject* ejs=NULL;
  JSClass* oclass=NULL;
  if ((!JSVAL_IS_OBJECT(ejsval))
      || (!(ejs=JSVAL_TO_OBJECT(ejsval)))
      || (!(oclass=JS_GET_CLASS(cx,ejs)))
      || (!(JSCLASS_RESERVED_SLOTS(oclass)))
      || (strcmp("ejs",oclass->name))) // especially ugly
    EJS_THROW_MESSED_ERROR(cx,obj);
  ejsout=ejs;
  return JS_TRUE;
}

//! enter untrusted mode: if we are already in untrusted mode this is an error
#define EJS_ENTER_UNTRUSTED(cx,obj) do{			\
    EJS_CHECK_TRUSTED(cx,obj);				\
    if (!ejs_set_untrusted(cx,obj,JSVAL_TRUE)) return JS_FALSE;	\
  }while(0)

//! set hidden untrusted field
inline
JSBool
ejs_set_untrusted(JSContext *cx, JSObject *obj,jsval untrusted)
{
  JSObject *ejs;
  EJS_CHECK(JSVAL_IS_BOOLEAN(untrusted));
  if (!ejs_get_ejs_object(cx,obj,ejs)) return JS_FALSE;
  return JS_SetReservedSlot(cx,ejs,0,untrusted);
}

//! get hidden untrusted field
inline
JSBool
ejs_get_untrusted(JSContext *cx, JSObject *obj,jsval &untrusted)
{
  JSObject *ejs;
  if (!ejs_get_ejs_object(cx,obj,ejs)) return JS_FALSE;
  if (!JS_GetReservedSlot(cx,ejs,0,&untrusted)) return JS_FALSE;
  if (!JSVAL_IS_BOOLEAN(untrusted)) EJS_THROW_MESSED_ERROR(cx,obj);
  return JS_TRUE;
}

inline
JSBool ejs_check_trusted(JSContext* cx, JSObject* obj)
{
  jsval untrusted;
  if (!ejs_get_untrusted(cx,obj,untrusted)) return JS_FALSE;
  if (untrusted==JSVAL_TRUE) EJS_THROW_ERROR(cx,obj,"Security exception");
  return JS_TRUE;
}

#endif
