#ifndef DOPE_NETWORK_H
#define DOPE_NETWORK_H

/*
 * Copyright (C) 2002 Jens Thiele <karme@berlios.de>
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
   \file dope/network.h
   \brief socket wrapper
   \author Jens Thiele
*/


// standard c++ includes
#include <cstdio>
#include <cstdlib>
#include <string>
#include <iostream>
#include <streambuf>
#include <list>
#include <algorithm>
#include <cerrno>
#include <ios>
#include <map>
#include <stdexcept>
#include "error.h"
#include "dopeexcept.h"
#include <cassert>

// sigc++
#include <sigc++/signal_system.h>
// boost
#include "smartptr.h"

// other dope includes
#include "socket.h"


#if defined(__WIN32__) || defined(WIN32)

// need fd_set

#define __USE_W32_SOCKETS
// define NOMINMAX - otherwise min and max are defined as macros !
#ifndef NOMINMAX
#define NOMINMAX
#else
#define NET_NOMINMAX_WAS_DEFINED
#endif

#include <windows.h>
#ifndef NET_NOMINMAX_WAS_DEFINED
#undef NOMINMAX
#else
#undef NET_NOMINMAX_WAS_DEFINED
#endif

#endif


template <typename _CharT, typename _Traits = std::char_traits<_CharT> >
class BasicNetStreamBuf : public std::basic_streambuf<_CharT, _Traits>
{
protected:
  Socket sock;

public:
  typedef _CharT 					char_type;
  typedef _Traits 					traits_type;
  typedef typename traits_type::int_type 		int_type;

  BasicNetStreamBuf(const InternetAddress &in_address) 
    : sock(in_address,true)
  {
    init();
  }
  
  BasicNetStreamBuf(const Socket &_sock) 
    : sock(_sock)
  {
    init();
  }

  //! test for input
  /*!
    \param timeout from man 2 select: upper bound on the amount of time elapsed
    before select returns. It may be zero, causing  select  to
    return  immediately.  If  timeout  is  NULL  (no timeout),
    select can block indefinitely.

    \return true if data is available otherwise false
  */
  bool select(const Timer::TimeStamp* timeout=NULL)
  {
    if (in_avail()>0)
      return true;
    return sock.inAvail(timeout);
  }

  //! return corresponding file handle / descriptor
  int get_handle(){
    return sock.get_handle();
  }

  //! set socket to blocking or non-blocking mode - default is blocking
  void setBlocking(bool block)
  {
    sock.setBlocking(block);
  }

  bool setTcpNoDelay(bool on=true)
  {
    return sock.setTcpNoDelay(on);
  }
  
  ~BasicNetStreamBuf()
  {
    freeBuf();
    if (pbase())
      delete [] pbase();
  }

  
protected:
  int_type underflow()
  {
    freeBuf();
    int bsize=1024;
    char_type * newBuf=new char_type[bsize];
    setg(newBuf,newBuf,newBuf+bsize);
    int res;
    int toread=sizeof(char_type)*bsize;
    res=sock.read((void *)gptr(),toread);
    if (res>0) {
      if (res!=toread) {
	JGACHINE_CHECK(toread>res);
	setg(eback(),gptr(),gptr()+res);
      }
      int_type r=traits_type::to_int_type(*gptr());
      setg(eback(),gptr()+1,egptr());
      return r;
    }
    // else
    freeBuf();
    return traits_type::eof();
  }

  /*
    std::streamsize 
    xsgetn(char_type* s, std::streamsize n)
    {
    std::streamsize __ret = 0;
    while (__ret < n)
    {
    size_t __buf_len = in_avail();
    if (__buf_len > 0)
    {
    // consume chars in buffer
    size_t __remaining = n - __ret;
    size_t __len = std::min(__buf_len, __remaining);
    traits_type::copy(s, gptr(), __len);
    __ret += __len;
    s += __len;
    setg(eback(),gptr()+__len,egptr());
    _M_in_cur_move(__len);
    }
    if (__ret < n)
    {
    // read from net
    size_t remaining = n - __ret;
    int got=read(sock.get_handle(),(void *)s,remaining);
    if (got>0) {
    return __ret+got;
    }
    return __ret;
    }
    }
    return __ret;
    }*/
  
  //! 
  /*! 
    \todo - why did i do this ?
  */
  int_type uflow()
  {
    int_type c = underflow();
    return c;
  }
  
  //! write to socket 
  /*!
    \note it seems i==eof means only to flush/sync
   */
  int_type overflow(int_type i = traits_type::eof())
  {
    JGACHINE_CHECK(pbase());
    int towrite=((char *)pptr())-((char *)pbase());
    int written=0;
    while (written<towrite){
      int res=sock.write((void *)((char *)pbase()+written),towrite-written);
      if (res<0)
	// error occured - todo should we bump the written characters ?
	return traits_type::eof();
      written+=res;
    }
    pbump(-towrite/sizeof(char_type));
    if (i!=traits_type::eof()) 
      sputc(i);
    return traits_type::not_eof(i);
  }

  /*!
    \return EOF on error
  */
  int sync()
  {
    return overflow();
  }
  
  //! put back one character
  /*!
    two possible cases:
    there was no space left to put back characters
    the character simply must be stored

    \todo what should I do if I am called with eof ?
  */
  int_type pbackfail(int_type c  = traits_type::eof())
  { 
    // inspired from gcc std c++ lib streambuf.tcc
    bool haveSpace = gptr() && eback() < gptr();
    if (!haveSpace)
      {
	// acquire new space - if current pos != end pos we have to copy the chars
        assert(egptr()-eback()>=0);
	int_type oldSize=egptr()-eback();
	int_type pbSize=1000;
        int_type newSize=oldSize+pbSize;
	char_type* newBuf=new char_type[newSize];
	assert(eback()==gptr());
	traits_type::copy(newBuf+pbSize,gptr(),oldSize);
	freeBuf();
	setg(newBuf,newBuf+pbSize,newBuf+newSize);
      }
    //    sputbackc(traits_type::to_char_type(c)); calls pbackfail ? - i thought it only calls pbackfail if no space is available ?
    // todo there must be a better way to do this
    setg(eback(),gptr()-1,egptr());
    *gptr()=traits_type::to_char_type(c);
    return c;
  }

  void init()
  {
    _M_mode = std::ios_base::in | std::ios_base::out; // sockets are always read/write ?
    _M_buf_unified = false; // we want to use different buffers for input/output
    setg(NULL,NULL,NULL);
    // todo should be at least the MTU of the underlying interface
    // or otherwise a fixed size member array (=> no new and delete)
    int bsize=1500; 
    char_type *buf=new char_type[bsize];
    setp(buf,buf+bsize);
  }

  void freeBuf()
  {
    if (eback())
      {
	delete [] eback();
	setg(NULL,NULL,NULL);
      }
  }
};

typedef BasicNetStreamBuf<char> NetStreamBuf;


#endif
