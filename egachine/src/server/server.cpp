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
   \file server/server.cpp
   \brief egaserver
   \author Jens Thiele
*/

#include <cstring>
#include <string>
#include <iostream>
#include <fstream>
#include <cassert>
#include <sstream>

#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h> // timeval

#include "ecmascript.h"
#include "error.h"

#include "timer/time.h"
#include "timer/jstime.h"

#include "network/network.h"
#include "network/jsnetwork.h"


class NetStreamBufServer {
public:
  typedef int ID;
  typedef SigC::Signal2<void, ID, JGACHINE_SMARTPTR<NetStreamBuf> > NewConnectionSignal;
  typedef SigC::Signal2<void, ID, JGACHINE_SMARTPTR<NetStreamBuf> > DataAvailableSignal;
  typedef SigC::Signal2<void, ID, JGACHINE_SMARTPTR<NetStreamBuf> > ConnectionClosedSignal;

  NetStreamBufServer(const Port &_port) : server_socket(_port)
  {}

  bool init();
  //! test for input
  /*!
    \param timeout from man 2 select: upper bound on the amount of time elapsed
    before select returns. It may be zero, causing  select  to
    return  immediately.  If  timeout  is  NULL  (no timeout),
    select can block indefinitely.

    \return true if data is available otherwise false
  */
  bool select(const Timer::TimeStamp* timeout=NULL);

  NewConnectionSignal newConnection;
  DataAvailableSignal dataAvailable;
  ConnectionClosedSignal connectionClosed;

  void closeStream(ID id){
    std::map<ID, JGACHINE_SMARTPTR<NetStreamBuf> >::iterator it(client_sockets.find(id));
    if (it==client_sockets.end())
      throw std::invalid_argument(__PRETTY_FUNCTION__);
    FD_CLR (it->first, &active_fd_set);
    connectionClosed.emit(it->first,it->second);
    client_sockets.erase(it);
  }
private:
  void closeStream(JGACHINE_SMARTPTR<NetStreamBuf> strPtr){
    std::map<ID, JGACHINE_SMARTPTR<NetStreamBuf> >::iterator it(client_sockets.begin());
    while (it!=client_sockets.end())
      {
	if (it->second.get()==strPtr.get())
	  {
	    closeStream(it->first);
	    return;
	  }
	++it;
      }
    if (it!=client_sockets.end())
      {
	FD_CLR (it->first, &active_fd_set);
	client_sockets.erase(it);
      }
  }

  std::map<ID, JGACHINE_SMARTPTR<NetStreamBuf> > client_sockets;
  Socket server_socket;
  fd_set active_fd_set;
};


#if defined(JGACHINE_RESTORE_CONNECT)
#define connect JGACHINE_RESTORE_CONNECT
#endif

// ---------------------------------------------------------------------------
// 'lib-netstream++' is a library that helps you to build networked 
//  applications with c++ or to be a base to build further abstract
//  coomunication libraries
//
//  Mainly it is a wrapper around a subset of libc system calls related
//  to sockets (see also info libc sockets) 
//  TODO:
//  * only tested with linux
//  Current restrictions:
//  * only IP4
//  * no internationilzation support
//  * only SOCK_STREAM sockets are supported (no plans to change this)
//  * only internet namespace socket adresses are supported (no named sockets)
//
//  Copyright (C) Jens Thiele <karme@unforgettable.com>
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Library General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Library General Public License for more details.
//
//  You should have received a copy of the GNU Library General Public
//  License along with this library; if not, write to the Free
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// ---------------------------------------------------------------------------



bool NetStreamBufServer::init(){
  FD_ZERO(&active_fd_set);
  if (!server_socket.init())
    return false;
  FD_SET(server_socket.getHandle(),&active_fd_set);
  return true;
}

bool NetStreamBufServer::select(const Timer::TimeStamp *timeout){
  bool ret=false;
  fd_set read_fd_set = active_fd_set;
  timeval ctimeout;
  if (timeout) {
    // todo what about negative values and unsigned timeval structs
    ctimeout.tv_sec=(*timeout)/1000000LL;
    ctimeout.tv_usec=(*timeout)%1000000LL;
  }
  // Find the largest file descriptor
  // todo - this should not be done on each select
  int maxfd;
  if (client_sockets.empty())
    maxfd=server_socket.getHandle();
  else {
    // client_sockets is a map => sorted 
    maxfd=client_sockets.rbegin()->first;
    // does the std require a map to be sorted ascending ?
    assert(maxfd>=client_sockets.begin()->first);
  }
  
  while (::select (++maxfd, &read_fd_set, NULL, NULL, (timeout) ? (&ctimeout) : NULL) < 0)
    {
      if (errno!=EINTR)
	JGACHINE_FATAL("select failed");
    }
  // which sockets have input pending ?
  for (int i = 0; i < maxfd; ++i)
    if (FD_ISSET (i, &read_fd_set))
      {
	if (i == server_socket.getHandle())
	  {
	    /* Connection request on original socket. */
	    JGACHINE_SMARTPTR<Socket> newSocketPtr(server_socket.accept());
	    if (newSocketPtr.get()) {
	      int nhandle=newSocketPtr->getHandle();
	      // Accepted connection
	      // add to active_fd_set
	      FD_SET (nhandle, &active_fd_set);
	      // we want to copy this socket object and don't want to close the handle when leaving this scope
	      newSocketPtr->setCloseOnDelete(false);
	      JGACHINE_SMARTPTR<NetStreamBuf> nbufptr(new NetStreamBuf(*newSocketPtr));
	      client_sockets[nhandle]=nbufptr;
	      newConnection.emit(nhandle,nbufptr);
	    }
	  }
	else
	  {
	    assert(client_sockets.find(i)!=client_sockets.end());
	    try {
	      ret=true;
	      dataAvailable.emit(i,client_sockets[i]);
	    }catch(const ReadError &e) {
	      JGACHINE_WARN("Warning read error occured: "<<e.what()<<" => close stream");
	      closeStream(i);
	    }catch(const std::range_error &e) {
	      JGACHINE_WARN(e.what());
	    }
	  }
      }
  return ret;
}


JSObject*
getNetObject()
{
  jsval oval;
  JGACHINE_CHECK(JS_GetProperty(ECMAScript::cx, ECMAScript::glob,"Net",&oval));
  JGACHINE_CHECK(JSVAL_IS_OBJECT(oval));
  JSObject *obj;
  JGACHINE_CHECK((obj=JSVAL_TO_OBJECT(oval)));
  return obj;
}

void
handleNewConnection(NetStreamBufServer::ID id, JGACHINE_SMARTPTR<NetStreamBuf> streamPtr)
{
  assert(streamPtr.get());
  JGACHINE_INFO("New connection ("<<id<<")");
  jsval rval;
  jsval args[2];
  JGACHINE_CHECK(JS_NewNumberValue(ECMAScript::cx,id,&args[0]));

  JSNetwork::newStreamObject(streamPtr.get(),&args[1]);

  ECMAScript::callFunction("Net","handleNewConnection",2,args);
}

void
handleDataAvailable(NetStreamBufServer::ID id, JGACHINE_SMARTPTR<NetStreamBuf> streamPtr)
{
  assert(streamPtr.get());
  jsval rval;
  jsval args[1];

  JGACHINE_CHECK(JS_NewNumberValue(ECMAScript::cx,id,&args[0]));

  ECMAScript::callFunction("Net","handleDataAvailable",1,args);
}

void
handleConnectionClosed(NetStreamBufServer::ID id, JGACHINE_SMARTPTR<NetStreamBuf> streamPtr)
{
  assert(streamPtr.get());
  JGACHINE_INFO("Connection ("<<id<<") closed");
  jsval rval;
  jsval args[1];

  JGACHINE_CHECK(JS_NewNumberValue(ECMAScript::cx,id,&args[0]));

  ECMAScript::callFunction("Net","handleConnectionClosed", 1, args);
}


// server socket
JGACHINE_SMARTPTR<NetStreamBufServer> incoming;

void
poll()
{
  JGACHINE_CHECK(incoming.get());
  Timer::TimeStamp zero(0);
  incoming->select(&zero);
}

extern "C" {
  ECMA_VOID_FUNC_VOID( ,poll);
  ECMA_BEGIN_VOID_FUNC(closeConnection)
  {
    ECMA_CHECK_NUM_ARGS(1);
    if (!JSVAL_IS_INT(argv[0])) return JS_FALSE;
    int id=JSVAL_TO_INT(argv[0]);
    JGACHINE_CHECK(incoming);
    incoming->closeStream(id);
    return JS_TRUE;
  }
}

static JSFunctionSpec static_methods[] = {
  ECMA_FUNCSPEC(poll,0),
  ECMA_FUNCSPEC(closeConnection,1),
  ECMA_END_FUNCSPECS
};


static
void
deinit()
{
  if (incoming.get()) {
    incoming=JGACHINE_SMARTPTR<NetStreamBufServer>();
  }

  Network::deinit();
  Timer::deinit();
  ECMAScript::deinit();
}

int
main(int argc, char **argv)
{
  if (!ECMAScript::init()) {
    JGACHINE_ERROR("could not inititialize interpreter");
    ECMAScript::deinit();
    return EXIT_FAILURE;
  }

  ECMAScript::parseConfig("server.js");

  // now register our objects/functions
  if (!(JSTimer::init()&& JSNetwork::init())) {
    JGACHINE_ERROR("could not register functions/objects with interpreter");
    ECMAScript::deinit();
    return EXIT_FAILURE;
  }

  Timer::init();
  Network::init();

  // copy command line arguments to the interpreter
  ECMAScript::copyargv(argc,argv);
	
  // create server socket
  int port=ECMAScript::evalInt32("this.listenPort != undefined ? this.listenPort : 47000");

  //  NetStreamBufServer* dummy=new NetStreamBufServer(port);
  //  incoming=JGACHINE_SMARTPTR<NetStreamBufServer>(dummy);

  incoming=JGACHINE_SMARTPTR<NetStreamBufServer>(new NetStreamBufServer(port));
  JGACHINE_CHECK(incoming);

  try {
    JGACHINE_CHECK(incoming->init());
  }catch(const SocketError &e){
    JGACHINE_ERROR("Could not listen on port: "<<port<<": "<<e.what());
    ::deinit();
    return EXIT_FAILURE;
  }

  incoming->newConnection.connect(SigC::slot(handleNewConnection));
  incoming->dataAvailable.connect(SigC::slot(handleDataAvailable));
  incoming->connectionClosed.connect(SigC::slot(handleConnectionClosed));

  JSObject *obj=getNetObject();
  JGACHINE_CHECK(JS_DefineFunctions(ECMAScript::cx, obj, static_methods));
	
  // load libs
  ECMAScript::parseLib("egachine.js");
  ECMAScript::parseLib("server.js");
	
  // copy command line arguments to the interpreter
  ECMAScript::copyargv(argc,argv);

  // copy version information to the interpreter
  ECMAScript::setVersion("EGachine.version");

  int ret=EXIT_SUCCESS;
  
  if (argc<2) {
    if (!ECMAScript::eval(std::cin,"stdin")) ret=EXIT_FAILURE;
  }else{
    std::ifstream in(argv[1]);
    if (in.good()) {
      if (!ECMAScript::eval(in,argv[1])) ret=EXIT_FAILURE;
    }else{
      ret=EXIT_FAILURE;
      JGACHINE_ERROR("Could not open file: \""<<argv[1]<<"\"");
    }
  }

  ::deinit();
  return ret;
}
