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

#include "ecmascript.h"

#include "timer/time.h"
#include "timer/jstime.h"

#include "video/video.h"
#include "video/jsvideo.h"

#include "input/input.h"
#include "input/jsinput.h"

#include "network/network.h"
#include "network/jsnetwork.h"

#include "error.h"

void deinit()
{
  JGACHINE_MSG("Info:"," deinit");
  
  Network::deinit();
  Input::deinit();
  JGACHINE_MSG("Info:"," pre Video::deinit");
  Video::deinit();
  JGACHINE_MSG("Info:"," post Video::deinit");
  Timer::deinit();

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
  exit(0);
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
  JGACHINE_CHECK(js);
  JGACHINE_CHECK(JS_AddRoot(ECMAScript::cx,js));
  jsval args[1];
  jsval rval;
  args[0]=STRING_TO_JSVAL(js);
  if (!JS_CallFunctionName(ECMAScript::cx, ECMAScript::glob, "handleChar", 1, args, &rval)) {
    std::cerr << "error while calling handleChar\n";
  }
  JGACHINE_CHECK(JS_RemoveRoot(ECMAScript::cx,js));
}

void
Input::devStateHandler(const Input::DevState& d)
{
  std::ostringstream o;
  o << "handleInput(new DevState("<<int(d.devno)<<","<<int(d.x)<<","<<int(d.y)<<","<<int(d.buttons)<<"));\n";
  std::istringstream i(o.str());
  ECMAScript::eval(i,JGACHINE_FUNCTIONNAME);
}


static
int32
getJSIntValue(const char* name, int32 defaultv)
{
  jsval rval;
  int32 res=defaultv;
  if (JS_EvaluateScript(ECMAScript::cx, ECMAScript::glob, name, strlen(name),JGACHINE_FUNCTIONNAME, 1, &rval)
      &&JS_ValueToInt32(ECMAScript::cx, rval, &res))
    return res;
  return defaultv;
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
  int ret=0;
  if (ECMAScript::init()) {
    // parse config files - this is done before we register our functions
    // since they may use config variables !!

    // todo - path serparator and $HOME
    char* sysconf="/etc/egachine/client.js";
    char* userconf="client/config.js";

    std::ifstream sysin(sysconf);
    if (sysin.good()) {
      ECMAScript::eval(sysin,sysconf);
    }
    std::ifstream userin(userconf);
    if (userin.good()) {
      ECMAScript::eval(userin,userconf);
    }

    // now register our objects/functions

    if (JSTimer::init()
	&& JSVideo::init()
	&& JSInput::init()
	&& JSNetwork::init()) 
      {
	Timer::init();
	Video::init(getJSIntValue("width",0),getJSIntValue("height",0),getJSIntValue("fullscreen",1));
	Input::init();
	Network::init();
	
	{
	  // now load library
	  char* lib="egachine.js";
	  std::ifstream in(lib);
	  JGACHINE_CHECK(in.good());
	  ret|=ECMAScript::eval(in,lib);
	}

	// copy command line arguments to the interpreter
	ECMAScript::copyargv(argc,argv);
	
	// copy version information to the interpreter
	ECMAScript::setVersion("EGachine.version");

	if (argc<2)
	  ret|=ECMAScript::eval(std::cin,"stdin");
	else{
	  std::ifstream in(argv[1]);
	  JGACHINE_CHECK(in.good());
	  ret|=ECMAScript::eval(in,argv[1]);
	}
      }else{
	ret|=4;
	std::cerr << "error registering functions/objects with interpreter\n";
      }
  }else{
    std::cerr << "error initializing interpreter\n";
    ret|=1;
  }

  deinit();
  return ret;
}
