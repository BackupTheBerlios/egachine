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

#include <cassert>
#include "input.h"
#include "ejsmodule.h"

//! exception we throw within our callbacks
struct CallbackError
{};


//! store data when calling poll
struct PollData
{
  PollData() : cx(NULL), obj(NULL)
  {}

  JSBool
  enterPoll(JSContext* _cx, JSObject* _obj)
  {
    EJS_CHECK(_cx&&_obj);
    if (cx||obj) EJS_THROW_ERROR(cx,obj,"only one active Input.poll allowed");
    cx=_cx;
    obj=_obj;
    return JS_TRUE;
  }
  
  void
  leavePoll(JSContext* _cx, JSObject* _obj)
  {
    EJS_CHECK((cx==_cx)&&(obj==_obj));
    EJS_CHECK(cx&&obj);
    cx=NULL;
    obj=NULL;
  }
  
  JSContext* cx;
  JSObject* obj;
};

static
PollData pollData;

//! keep pollData correct in case of exceptions (finally)
/*!
  The "resource acquisition is initialization" technique
  see also:
  http://www.research.att.com/~bs/bs_faq2.html#finally
*/
struct Poll
{
  Poll(JSContext* _cx, JSObject* _obj) : cx(_cx), obj(_obj)
  {
    if ((_good=pollData.enterPoll(cx,obj)))
      Input::poll();
  }
  JSBool good() const
  {
    return _good;
  }
  ~Poll()
  {
    pollData.leavePoll(cx,obj);
  }
private:
  JSBool _good;
  JSContext* cx;
  JSObject* obj;
};


extern "C" {
  //! fetch input events
  /*!
    we are called back from the input layer
   */
  static
  JSBool
  poll(JSContext *cx, JSObject *obj, uintN, jsval *, jsval *)
  {
    try{
      Poll poll(cx,obj);
      if (!poll.good()) return JS_FALSE;
    }catch(const CallbackError &error){
      return JS_FALSE;
    }
    return JS_TRUE;
  }

  static
  JSBool
  charMode(JSContext *cx, JSObject *obj, uintN argc, jsval *argv, jsval *)
  {
    EJS_CHECK_NUM_ARGS(cx,obj,1,argc);
    JSBool b;
    if (!JS_ValueToBoolean(cx, argv[0],&b))
      return JS_FALSE;
    Input::charInput(b==JS_TRUE);
    return JS_TRUE;
  }

#define FUNC(name,numargs) { #name,name,numargs,0,0}

  static JSFunctionSpec static_methods[] = {
    FUNC(poll,0),
    FUNC(charMode,1),
    EJS_END_FUNCTIONSPEC
  };

#undef FUNC

  JSBool
  ejsinput_LTX_onLoad(JSContext* cx, JSObject* global)
  {
    Input::init();
    JSObject *obj = JS_DefineObject(cx, global,
				    "Input", NULL, NULL,
				    JSPROP_ENUMERATE);
    if (!obj) return JS_FALSE;
    return JS_DefineFunctions(cx, obj, static_methods);
  }

  JSBool
  ejsinput_LTX_onUnLoad()
  {
    Input::deinit();
    return JS_TRUE;
  }
}



//! use this to root objects in callbacks
struct CbRoot
{
  CbRoot(JSContext* _cx, void* _obj) : cx(_cx), obj(_obj)
  {
    EJS_CHECK(cx&&obj);
    if (!JS_AddRoot(cx,obj)) {
      cx=NULL;
      obj=NULL;
      throw CallbackError();
    }
  }
  ~CbRoot()
  {
    if (cx&&obj)
      JS_RemoveRoot(cx,obj);
  }
protected:
  JSContext* cx;
  void* obj;
};

void
Input::quitHandler()
{
  EJS_CHECK(pollData.cx&&pollData.obj);
  JSContext* cx=pollData.cx;
  JSObject* obj=pollData.obj;
  jsval dummy;
  if (!JS_CallFunctionName(cx, obj, "handleQuit", 0, NULL, &dummy))
    throw CallbackError();
}

void
Input::toggleFullscreenHandler()
{
  EJS_CHECK(pollData.cx&&pollData.obj);
  JSContext* cx=pollData.cx;
  JSObject* obj=pollData.obj;
  jsval dummy;
  if (!JS_CallFunctionName(cx, obj, "toggleFullscreen", 0, NULL, &dummy))
    throw CallbackError();
}

void
Input::iconifyHandler()
{
  EJS_CHECK(pollData.cx&&pollData.obj);
  JSContext* cx=pollData.cx;
  JSObject* obj=pollData.obj;
  jsval dummy;
  if (!JS_CallFunctionName(cx, obj, "iconify", 0, NULL, &dummy))
    throw CallbackError();
}

//! input layer calls this if we should resize (f.e. the input layer got a message from a window manager)
void
Input::resizeHandler(int sx, int sy)
{
  // resize neccessary? prevent possible endless loop where video subsystem generates input event
  // hmm still shit 
  // 
  // UPDATE: i implemented a workaround in sdl/input.cpp (NEED_RESIZE_HACK)
  // 
  // the (sdlopengl) video subsystem generates resize requests - this is a real problem with wm's like sawfish....
  // corresponding discussion on SDL ML: http://www.libsdl.org/pipermail/sdl/2003-July/thread.html#55073
  // Idea: perhaps sawfish does also react on application requested resizes and tries to resize
  // to the application requested size in small steps, too - hmm (this would explain the alternating resizes)
  // 1. test with different wm's
  // 2. take a look at the sawfish source
  // 3. how did I do it in TUD? / read the Xlib manual
  // 4. read the SDL source
  //  if ((sx==org::jgachine::JGachine::width)&&(sy==org::jgachine::JGachine::height))
  //    return;

  // tell the video layer to actually resize
  //  Video::resize(sx,sy);
}

void
Input::charHandler(Unicode uc)
{
  // todo this should be a compile time check
  assert(sizeof(Unicode)==sizeof(jschar));
  EJS_CHECK(pollData.cx&&pollData.obj);
  JSContext* cx=pollData.cx;
  JSObject* obj=pollData.obj;

  jschar s[1];
  s[0]=uc;
  JSString* js=JS_NewUCStringCopyN(cx, s, 1);
  if (!js) throw CallbackError();
  CbRoot root(cx,js);
  
  jsval args[1];
  args[0]=STRING_TO_JSVAL(js);

  jsval dummy;
  if (!JS_CallFunctionName(cx, obj, "handleChar", 1, args, &dummy))
    throw CallbackError();
}

void
Input::devStateHandler(const Input::DevState& d)
{
  EJS_CHECK(pollData.cx&&pollData.obj);
  std::ostringstream o;
  o << "handleInput(new DevState("<<int(d.devno)<<","<<int(d.x)<<","<<int(d.y)<<","<<int(d.buttons)<<"));\n";
  // todo: i think we should use JS_CallFunction
  jsval rval;
  if (!JS_EvaluateScript(pollData.cx, pollData.obj,
			 o.str().c_str(), o.str().length(),
			 EJS_FUNCTIONNAME,1,&rval))
    throw CallbackError();
}
