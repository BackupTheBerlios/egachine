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

// sigc++
#include <sigc++/bind.h>

#include "ejsmodule.h" // for EJS_ macros
#include "netstreambufserver.h"

#if defined(__WIN32__) || defined(WIN32)
#define WINDOOF
#endif

#ifndef WINDOOF
#include <signal.h>

void sigPipeHandler(int x){
  //  EJS_WARN("Ignore SIGPIPE signal");
}

#endif



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

static
bool globalInitCalled=false;

static
void
globalInit()
{
  if (globalInitCalled) return;
#ifndef WINDOOF
  signal(SIGPIPE,sigPipeHandler);
#endif
}



bool NetStreamBufServer::init(){
  globalInit();
  FD_ZERO(&active_fd_set);
  if (!serverSocket.init())
    return false;
  FD_SET(serverSocket.getHandle(),&active_fd_set);
  return true;
}

bool NetStreamBufServer::select(const Timeout *timeout){
  bool ret=false;
  fd_set read_fd_set = active_fd_set;
  timeval ctimeout;
  if (timeout) {
    if (*timeout>=0){
      // todo what about negative values and unsigned timeval structs
      ctimeout.tv_sec=(*timeout)/1000000LL;
      ctimeout.tv_usec=(*timeout)%1000000LL;
    }else{
      EJS_WARN("negative timeout value");
      ctimeout.tv_sec=ctimeout.tv_usec=0;
    }
  }
  // Find the largest file descriptor
  // todo - this should not be done on each select
  int maxfd;
  if (clientSockets.empty())
    maxfd=serverSocket.getHandle();
  else {
    // clientSockets is a map => sorted 
    maxfd=clientSockets.rbegin()->first;
    maxfd=std::max(maxfd,serverSocket.getHandle());
    // does the std require a map to be sorted ascending ?
    assert(maxfd>=clientSockets.begin()->first);
  }
  
  while (::select (++maxfd, &read_fd_set, NULL, NULL, (timeout) ? (&ctimeout) : NULL) < 0)
    {
      if (errno!=EINTR) {
	if (timeout)
	  EJS_ERROR("(timeout:"<< (*timeout));
	EJS_FATAL("select failed");
      }
    }
  // which sockets have input pending ?
  for (int i = 0; i < maxfd; ++i)
    if (FD_ISSET (i, &read_fd_set))
      {
	if (i == serverSocket.getHandle())
	  {
	    // Connection request on original socket.
	    // Accepted connection
	    Socket* newSocketPtr(serverSocket.accept());
	    EJS_CHECK(newSocketPtr);
	    int nhandle=newSocketPtr->getHandle();
	    // add to active_fd_set
	    FD_SET (nhandle, &active_fd_set);
	    
	    NetStreamBuf* nbufptr=new NetStreamBuf(*newSocketPtr);
	    nbufptr->onDelete.connect(SigC::bind(SigC::slot(*this,&NetStreamBufServer::closeStream),nhandle));
	    // we copied this socket object and don't want to close the handle
	    newSocketPtr->setCloseOnDelete(false);
	    delete newSocketPtr;
	    
	    clientSockets[nhandle]=nbufptr;
	    // note: this might call code removing a client socket
	    newConnection.emit(nhandle,nbufptr);
	  }
	else
	  {
	    // perhaps we aren't interested in this fd anymore
	    ClientSockets::iterator it(clientSockets.find(i));
	    if (it!=clientSockets.end()) {
	      try {
		ret=true;
		dataAvailable.emit(i,it->second);
		//	    }catch(const ReadError &e) {
		//	      EJS_WARN("Warning read error occured: "<<e.what()<<" => close stream");
		//	      closeStream(i);
	      }catch(const std::range_error &e) {
		EJS_WARN(e.what());
	      }
	    }
	  }
      }
  return ret;
}

