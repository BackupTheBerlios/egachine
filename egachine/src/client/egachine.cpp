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
   \file client/egachine.cpp
   \brief egachine main
   \author Jens Thiele
*/

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>
#include <cstdlib>

#include "ecmascript.h"

#include "timer/time.h"
#include "timer/jstime.h"

#include "video/video.h"
#include "video/jsvideo.h"

#include "input/input.h"
#include "input/jsinput.h"

#include "network/network.h"
#include "network/jsnetwork.h"

#include "audio/audio.h"
#include "audio/jsaudio.h"

#include "common/jszlib.h"

#include "error.h"

void deinit()
{
  Audio::deinit();
  Network::deinit();
  Input::deinit();
  Video::deinit();
  Timer::deinit();

  JSZlib::deinit();
  JSAudio::deinit();
  JSTimer::deinit();
  JSInput::deinit();
  JSVideo::deinit();
  JSNetwork::deinit();

  ECMAScript::deinit();
}

void
Input::quitHandler()
{
  ::deinit();
  // hmm perhaps a bad idea to use exit
  exit(EXIT_SUCCESS);
}

void
Input::toggleFullscreenHandler()
{
  Video::toggleFullscreen();
}

void
Input::iconifyHandler()
{
  Video::iconify();
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
  Video::resize(sx,sy);
}

void
Input::charHandler(Unicode uc)
{
  // todo this should be a compile time check
  assert(sizeof(Unicode)==sizeof(jschar));
  jschar s[1];
  s[0]=uc;
  JSString* js=JS_NewUCStringCopyN(ECMAScript::cx, s, 1);
  if (!js) throw Input::CallbackError("js error");
  // todo
  JGACHINE_CHECK(JS_AddRoot(ECMAScript::cx,js));
  jsval args[1];
  args[0]=STRING_TO_JSVAL(js);
  JSBool res=ECMAScript::callFunction("Input","handleChar",1,args);
  // todo
  JGACHINE_CHECK(JS_RemoveRoot(ECMAScript::cx,js));
  if (!res)
    throw Input::CallbackError("js error");
}

void
Input::devStateHandler(const Input::DevState& d)
{
  std::ostringstream o;
  o << "Input.handleInput(new DevState("<<int(d.devno)<<","<<int(d.x)<<","<<int(d.y)<<","<<int(d.buttons)<<"));\n";
  // todo: i think we should use JS_CallFunction
  // note: we do not use ECMAScript::eval since this clears pending exceptions
  // i posted a question to the newsgroup if it is allowed at all to call eval within the same context again
  jsval rval;
  if (!JS_EvaluateScript(ECMAScript::cx, ECMAScript::glob,
			 o.str().c_str(), o.str().length(),
			 JGACHINE_FUNCTIONNAME,1,&rval))
    throw Input::CallbackError("js error");
}

#ifndef main
int main(int argc, char **argv)
#else
int cppmain(int argc,char *argv[]);
extern "C" {
  int SDL_main(int argc,char *argv[])
  {
    return cppmain(argc,argv);
  }
}
int cppmain(int argc,char *argv[])
#endif
{
  if (argc<2) {
    std::cerr << "Usage: egachine FILE [OPTION]...\n";
    return EXIT_FAILURE;
  }
  
  if (!ECMAScript::init()) {
    JGACHINE_ERROR("could not inititialize interpreter");
    ECMAScript::deinit();
    return EXIT_FAILURE;
  }

  ECMAScript::parseConfig("client.js");

  // now register our objects/functions
  if (!(JSTimer::init()
	&& JSVideo::init()
	&& JSInput::init()
	&& JSNetwork::init()
	&& JSAudio::init()
	&& JSZlib::init()
	)) {
    JGACHINE_ERROR("could not register functions/objects with interpreter");
    ECMAScript::deinit();
    return EXIT_FAILURE;
  }

#define GETCONF(param,default) int param=ECMAScript::evalInt32("this." #param "!= undefined ? this." #param ":" #default)
  GETCONF(width,0);
  GETCONF(height,0);
  GETCONF(fullscreen,1);
#undef GETCONF
  AudioConfig ac;
#define GETCONF(param,default) ac.param=ECMAScript::evalInt32("this." #param "!=undefined ? this." #param ":" #default)
  GETCONF(srate,44100);
  GETCONF(sbits,16);
  GETCONF(sbuffers,512);
  GETCONF(stereo,1);
#undef GETCONF


  Timer::init();
  // todo: catch exceptions
  Video::init(width,height,fullscreen);
  Audio::init(ac);
  Input::init();
  Network::init();
	
  // now load libraries
  ECMAScript::parseLib("common.js");
  ECMAScript::parseLib("egachine.js");
  
  // copy command line arguments to the interpreter
  ECMAScript::copyargv(argc,argv);
  
  // copy version information to the interpreter
  ECMAScript::setVersion("EGachine.version");

  int ret=EXIT_SUCCESS;
  std::ifstream in(argv[1]);
  if (in.good()) {
    if (!ECMAScript::eval(in,argv[1])) ret=EXIT_FAILURE;
  }else{
    ret=EXIT_FAILURE;
    JGACHINE_ERROR("Could not open file: \""<<argv[1]<<"\"");
  }

  deinit();
  return ret;
}
