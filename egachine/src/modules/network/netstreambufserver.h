#ifndef NETSTREAMBUFSERVER_H
#define NETSTREAMBUFSERVER_H

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
   \brief NetStreamBufServer Class
   \author Jens Thiele
*/

#include "netstreambuf.h"
// sigc++
#include <sigc++/signal_system.h>

//! Factory for NetStreamBuf objects (created if a client connects)
class NetStreamBufServer : public SigC::Object
{
public:
  typedef int ID;
  typedef SigC::Signal2<void, ID, NetStreamBuf* > NewConnectionSignal;
  typedef SigC::Signal2<void, ID, NetStreamBuf* > DataAvailableSignal;
  typedef SigC::Signal2<void, ID, NetStreamBuf* > ConnectionClosedSignal;

  NetStreamBufServer(const Port &_port) : serverSocket(_port)
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
  bool select(const Timeout* timeout=NULL);

  NewConnectionSignal newConnection;
  DataAvailableSignal dataAvailable;

private:
  void
  closeStream(ID id){
    ClientSockets::iterator it(clientSockets.find(id));
    EJS_CHECK(it!=clientSockets.end());
    FD_CLR (it->first, &active_fd_set);
    clientSockets.erase(it);
    EJS_INFO(id);
  }

  typedef std::map<ID, NetStreamBuf* > ClientSockets;
  ClientSockets clientSockets;
  Socket serverSocket;
  fd_set active_fd_set;
};

#endif
