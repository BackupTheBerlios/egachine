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
   \file sdl/input.cpp
   \brief SDL input layer/interface implementation
   \author Jens Thiele
*/

#include <SDL.h>
#include <cassert>
#include <vector>
#include <iostream>

#include "../input.h"
#include "error.h"

// #define NEED_RESIZE_HACK 

#ifdef NEED_RESIZE_HACK

#include <list>

typedef std::list<SDL_Event> EventQueue;
EventQueue* eventQueue=NULL;

static SDL_ResizeEvent expected;
static bool expectResize=false;

static
std::ostream &operator<<(std::ostream& o, const SDL_ResizeEvent &e)
{
  o << e.w << "/" << e.h;
  return o;
}

#endif

// all input devices
static std::vector<Input::DevState> *devState=NULL;
// map joystick number to input device number
static std::vector<int> *joyDevMap=NULL;
// map keyboard number to input device number
int keyDevMap[2]={0,1};
// keyboard definition
typedef SDLKey Keys[6];
Keys keys[2]={
  {SDLK_KP8, SDLK_KP2, SDLK_KP4, SDLK_KP6, SDLK_KP0, SDLK_KP_PERIOD},
  {SDLK_w, SDLK_s, SDLK_a, SDLK_d, SDLK_1, SDLK_2}
};

//! are we in character input mode?
/*!
  there are 2 input modes:
  character input mode (for entering text)
  normal mode (keyboard is used as joypad like device)
*/
static bool charMode=false;


void
Input::init()
{
#ifdef NEED_RESIZE_HACK
  assert(!eventQueue);
  eventQueue=new EventQueue();
#endif

  assert(!devState);
  devState=new std::vector<Input::DevState>();
  assert(!joyDevMap);
  int numj=SDL_NumJoysticks();
  joyDevMap=new std::vector<int>(numj);
  
  for (int i=0;i<numj;++i) {
    (*joyDevMap)[i]=-1;
    SDL_Joystick* joy=SDL_JoystickOpen(i);
    if (!joy) continue;
    if (SDL_JoystickNumAxes(joy)<2) {
      SDL_JoystickClose(joy);
      continue;
    }
    // found usable joy
    DevState s;
    s.devno=devState->size();
    devState->push_back(s);
    (*joyDevMap)[i]=s.devno;
  }
  // add 2 keyboard devices
  DevState s;
  s.devno=keyDevMap[0]=devState->size();
  devState->push_back(s);
  s.devno=keyDevMap[1]=devState->size();
  devState->push_back(s);
}

void
Input::charInput(bool active)
{
  if (active==::charMode) return;
  charMode=active;
  SDL_EnableUNICODE(charMode);
}

unsigned
Input::attachedDevices()
{
  if (!devState) return 0;
  return devState->size();
}

static
void toggleGrab()
{
  SDL_GrabMode mode;

  //  printf("Ctrl-G: toggling input grab!\n");
  mode = SDL_WM_GrabInput(SDL_GRAB_QUERY);
  if ((mode!=SDL_GRAB_OFF)&&(mode!=SDL_GRAB_ON))
    return;
  /*
    if ( mode == SDL_GRAB_ON ) {
    printf("Grab was on\n");
    } else {
    printf("Grab was off\n");
    }
  */
  mode = SDL_WM_GrabInput((mode==SDL_GRAB_ON)?SDL_GRAB_OFF:SDL_GRAB_ON);
  /*
    if ( mode == SDL_GRAB_ON ) {
    printf("Grab is now on\n");
    } else {
    printf("Grab is now off\n");
    }
  */
}


static
bool
handleSpecialKey(const SDL_KeyboardEvent &e)
{
  bool pressed=e.state==SDL_PRESSED;
  if (!pressed) return false;
  switch (e.keysym.sym){
  case SDLK_g:
    if (e.keysym.mod & KMOD_CTRL) {
      toggleGrab();
      return true;
    }
    break;
  case SDLK_z:    
    if (e.keysym.mod & KMOD_CTRL) {
      Input::iconifyHandler();
      return true;
    }
    break;
  case SDLK_ESCAPE:
    Input::quitHandler();
    return true;
  case SDLK_RETURN:
    if (e.keysym.mod & KMOD_ALT){
      Input::toggleFullscreenHandler();
      return true;
    }
    break;
  default:
    return false;
  }
  return false;
}

//! handle key in character mode
static
bool
handleKey(const SDL_KeyboardEvent &e)
{
  if (!charMode) return false;
  bool pressed=e.state==SDL_PRESSED;
  if (!pressed) return false;
  Input::charHandler(e.keysym.unicode);
  return true;
}


//! \return true if it was a device key (the key was handled)  
static
bool
handleDevKey(const SDL_KeyboardEvent &e, int keyDev)
{
  // map keyboard dev no to real dev no
  assert((keyDev>=0)&&(keyDev<2));
  int dev=keyDevMap[keyDev];
  assert(devState);
  assert((dev>=0)&&(dev<(int)devState->size()));
  
  Input::DevState &m_state((*devState)[dev]);
  Input::DevState old(m_state);
  bool pressed=e.state==SDL_PRESSED;
  SDLKey k(e.keysym.sym);
  if (k==keys[keyDev][0]) {
    // up
    if (pressed) m_state.y=1;
    else if (m_state.y==1) m_state.y=0;
  }else if (k==keys[keyDev][1]) {
    // down
    if (pressed) m_state.y=-1;
    else if (m_state.y==-1) m_state.y=0;
  }else if (k==keys[keyDev][2]) {
    // left 
    if (pressed) m_state.x=-1;
    else if (m_state.x==-1) m_state.x=0;
  }else if (k==keys[keyDev][3]) {
    // right
    if (pressed) m_state.x=1;
    else if (m_state.x==1) m_state.x=0;
  }else if (k==keys[keyDev][4]) {
    // button 1
    if (pressed) m_state.buttons|=1;
    else m_state.buttons&=~1;
  }else if (k==keys[keyDev][5]) {
    // button 1
    if (pressed) m_state.buttons|=2;
    else m_state.buttons&=~2;
  }
  if (m_state!=old) {
    Input::devStateHandler(m_state);
    return true;
  }
  return false;
}

static
signed char
joyScale(Sint16 v) 
{
  Sint16 clearance=20000;
  if (v>clearance) return 1;
  if (v<-clearance) return -1;
  return 0;
}

static
void
handleJoyMotion(const SDL_JoyAxisEvent &event)
{
  int joy=event.which;
  assert(joyDevMap);
  if ((joy<0)||(joy>=(int)joyDevMap->size())) return;
  
  int dev=(*joyDevMap)[joy];
  assert(devState);
  if ((dev<0)||(dev>=(int)devState->size())) return;
  
  Input::DevState &m_state((*devState)[dev]);
  Input::DevState old(m_state);

  switch (event.axis) {
  case 0: 
    m_state.x=joyScale(event.value);
    break;
  case 1:
    m_state.y=joyScale(-event.value);
    break;
  }

  if (m_state!=old)
    Input::devStateHandler(m_state);
}

static
void
handleJoyButton(const SDL_JoyButtonEvent &e)
{
  int joy=e.which;
  assert(joyDevMap);
  if ((joy<0)||(joy>=(int)joyDevMap->size())) return;
  
  int dev=(*joyDevMap)[joy];
  assert(devState);
  if ((dev<0)||(dev>=(int)devState->size())) return;
  
  Input::DevState &m_state((*devState)[dev]);
  Input::DevState old(m_state);

  if (e.state==SDL_PRESSED)
    m_state.buttons|=1<<e.button;
  else
    m_state.buttons&=~(1<<e.button);

  if (m_state!=old)
    Input::devStateHandler(m_state);
}

inline
static
void handleResize(const SDL_ResizeEvent &e)
{
  Input::resizeHandler(e.w,e.h);
}

//! handle one sdl event
/*!
  return true if the event should be removed from the queue false otherwise
*/
static
bool
handleEvent(const SDL_Event &event)
{
    switch (event.type) {
    case SDL_QUIT:
      Input::quitHandler();
      return true;
    case SDL_KEYDOWN:
    case SDL_KEYUP:
      if (!charMode) {
	if (!handleDevKey(event.key,0))
	  if (!handleDevKey(event.key,1))
	    handleSpecialKey(event.key);
      }else{
	if (!handleSpecialKey(event.key)) {
	  handleKey(event.key);
	}
      }
      return true;
    case SDL_MOUSEMOTION:
      //      mouseMotion.emit(event.motion);
      return true;
    case SDL_MOUSEBUTTONDOWN:
    case SDL_MOUSEBUTTONUP:
      //      mouseButton.emit(event.button);
      return true;
    case SDL_JOYAXISMOTION:
      handleJoyMotion(event.jaxis);
      return true;
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
      handleJoyButton(event.jbutton);
      return true;
    case SDL_VIDEORESIZE:
#ifndef NEED_RESIZE_HACK
      handleResize(event.resize);
      return true;
#else
      std::cerr << "got resize: "<<event.resize<<" and ";
      if (expectResize) {
	// we assume if we get the right size - this is the event we (unfortunately) produced
	if ((expected.w==event.resize.w)&&(expected.h==event.resize.h)) {
	  std::cerr << "this was the expected resize event\n";
	  expectResize=false;
	  return true;
	}else{
	  std::cerr << "this was NOT the expected resize event\n";      
	  if (eventQueue->size()>2) {
	    std::cerr << "since there are many events pending i stop waiting for this event - i assume it was lost\n";
	    expectResize=false;
	    return true;
	  }
	  return false;
	}
      }else{
	std::cerr << "we do not expect a resize event\n";
	handleResize(event.resize);
	expectResize=true;
	expected=event.resize;
	return true;
      }
#endif
    default:
      JGACHINE_INFO("Got unknown event => we drop it ("<<unsigned(event.type)<<")");
      return true;
    }
}


void 
Input::poll()
{
  SDL_Event event;

#ifdef NEED_RESIZE_HACK
  assert(eventQueue);
  EventQueue::iterator it(eventQueue->begin());
  while (it!=eventQueue->end()) {
    if (handleEvent(*it))
      it=eventQueue->erase(it);
    else
      ++it;
  }
  while ( SDL_PollEvent(&event) ) {
    if (!handleEvent(event)) {
      eventQueue->push_back(event);
    }
  }

#else
  
  while ( SDL_PollEvent(&event) ) {
    handleEvent(event);
  }
#endif

}

void
Input::deinit()
{
#ifdef NEED_RESIZE_HACK
  if (eventQueue){
    delete eventQueue;
    eventQueue=NULL;
  }
#endif

  if (devState) {
    delete devState;
    devState=NULL;
  }
  if (joyDevMap) {
    delete joyDevMap;
    joyDevMap=NULL;
  }
  // todo: close joysticks
}

