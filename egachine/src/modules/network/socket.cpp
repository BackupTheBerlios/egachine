#include "socket.h"

#include <unistd.h> // close
#include <cstdio>
#include <cstdlib>
#include <cassert>
#include <sstream>

#include "ejsmodule.h"

#ifndef SOCKET_NO_EXCEPTIONS
#define JGACHINE_SERROR(msg) throw SocketError(msg)
#else
#error "todo: this will not work out of the box anymore"
#define JGACHINE_SERROR(msg) do{EJS_WARN(msg);return false;}while(0)
#endif

#if defined(__WIN32__) || defined(WIN32)
#define WINDOOF
#endif

#ifdef WINDOOF

#define __USE_W32_SOCKETS
#include <windows.h>

// define to setsockopt value type:
// win: int setsockopt(SOCKET,int,int,const char*,int);
#define JGACHINE_SOCKOPTVALT char*

#ifndef EWOULDBLOCK
#define EWOULDBLOCK WSAEWOULDBLOCK
#endif

#ifndef socklen_t
#define socklen_t int
#endif

#else /* ifdef WINDOOF */

// not win => we assume unix like (posix)
// "unix" includes
// for htons
#include <netinet/in.h>
/*
  attention on SunOS this makes a #define connect __xnet_connect
  if _XPG4_2 is defined which is defined (at least with gcc >=3)
  s.a.:
  http://gcc.gnu.org/cgi-bin/gnatsweb.pl?cmd=view%20audit-trail&database=gcc&pr=5440
*/
#include <sys/socket.h>
#if defined(connect)
// this assumes that if connect is defined it is defined to be __xnet_connect !!
#define JGACHINE_RESTORE_CONNECT __xnet_connect
#undef connect

#endif

#include <arpa/inet.h>   // inet_pton
#include <netdb.h>       // hostent
#include <fcntl.h>       // fcntl
#include <sys/types.h>   // setsockopt
#include <sys/time.h>    // timeval


#ifdef HAVE_TCP_NODELAY
#include <netinet/tcp.h> // TCP_NODELAY
#endif

#define closesocket	::close
#define JGACHINE_SOCKOPTVALT void*

#endif /* WIN32 */



//! convert any type to a string which has operator<<
template<typename X>
std::string anyToString(X z)
{
  std::ostringstream o;
  o.setf(std::ios::fixed);
  o.precision(50);
  o.unsetf(std::ios::showpoint);
  o << z;
  return o.str();
};

class SocketAddress
{
public:
  SocketAddress(unsigned long int haddr, unsigned short int port)
  {
    in_addr tmp;
    tmp.s_addr=haddr;
    addr.sin_family=AF_INET;
    addr.sin_addr=tmp;
    addr.sin_port=htons(port);
  }
  
  sockaddr *getSockAddr() 
  {return (sockaddr *)&addr;}

  int getSize()
  {return sizeof(addr);}

  std::string toString() const
  {
    std::string res;
    for (int i=0;i<4;++i) {
      res+=anyToString(unsigned(((unsigned char *)(&(addr.sin_addr)))[i]));
      res+=(i!=3) ? "." : ":";
    }
    res+=anyToString(unsigned(ntohs(addr.sin_port)));
    return res;
  }
protected:
  sockaddr_in addr;
};


Socket::Socket(const InternetAddress &_iaddr, bool runInit) 
  : server(false),
    closeOnDelete(true),
    handle(-1),
    iaddr(_iaddr)
{
  if (!globalInit()) JGACHINE_SERROR("globalInit");
  if (runInit) init();
}

Socket::Socket(Port _port, bool runInit)
  : server(true),
    closeOnDelete(true),
    handle(-1),
    iaddr(HostAddress(),_port)
{
  if (!globalInit()) JGACHINE_SERROR("globalInit");
  if (runInit) init();
}


Socket::Socket(unsigned long int _haddr, Port _port, int _handle, bool runInit) 
  : closeOnDelete(true),handle(_handle),haddr(_haddr)
{
  if (!globalInit()) JGACHINE_SERROR("globalInit");
  if (runInit) init();
  // todo "reverse" lookup addr
  iaddr.second=ntohs(_port);
}


Socket::Socket(const Socket&o) 
  : server(o.server), closeOnDelete(o.closeOnDelete), handle(o.handle), 
    iaddr(o.iaddr), haddr(o.haddr)
{
  globalInit();
}

Socket::Socket()
  : server(true), closeOnDelete(false), handle(-1)
{
  globalInit();
}

// this is no class member because of c++ static member initialization problems
static int initCount=0;

bool
Socket::globalInit()
{
  if (initCount++) return true;
#ifdef WINDOOF
  /* Start up the windows networking */
  WORD version_wanted = MAKEWORD(1,1);
  WSADATA wsaData;
  
  if ( WSAStartup(version_wanted, &wsaData) != 0 ) JGACHINE_SERROR("Couldn't initialize Winsock 1.1");
#endif
  return true;
}

bool
Socket::globalDeinit()
{
  assert(initCount);
  if (--initCount) return true;
#ifdef WINDOOF
  /* Clean up windows networking */
  if ( WSACleanup() == SOCKET_ERROR ) {
    if ( WSAGetLastError() == WSAEINPROGRESS ) {
      /* todo was undeclared
	 WSACancelBlockingCall();
      */
      WSACleanup();
    }
  }
#endif
  return true;
}

bool
Socket::init() {
  /* Create the socket. */
  handle = socket (PF_INET, SOCK_STREAM, 0);
  if (handle== -1) JGACHINE_SERROR("create");

  if (!server) return connect();

  reuse();
  if (!bind()) JGACHINE_SERROR("bind");
  if (!listen()) JGACHINE_SERROR("listen");
  return true;
}

void
Socket::reuse()
{
  int reuser=1;
  int len=sizeof(reuser);
  if (setsockopt(handle,SOL_SOCKET,SO_REUSEADDR,(JGACHINE_SOCKOPTVALT) &reuser,len)<0) 
    EJS_WARN("reuse failed");
};

bool
Socket::bind()
{
  // host resolution
  assert(server);
  haddr=INADDR_ANY;
  SocketAddress saddr(haddr,iaddr.second);
  if (::bind(handle, saddr.getSockAddr(), saddr.getSize()))
    {
      close();
      JGACHINE_SERROR("Can't bind socket");
    }
  return true;
}

bool
Socket::listen(){
  if (::listen (handle, 1) < 0)
    {
      close();
      JGACHINE_SERROR("can't listen on socket");
    }
  return true;
}

bool
Socket::connect(){
  hostent *he_ptr=gethostbyname(iaddr.first.c_str());
  if (!he_ptr) JGACHINE_SERROR("Hostname not valid");
  EJS_CHECK(he_ptr->h_length==sizeof(haddr));
  std::memcpy((char *)&haddr,(char *)he_ptr->h_addr,he_ptr->h_length);

  SocketAddress saddr(haddr,iaddr.second);
  std::string hstring(saddr.toString());
  //   EJS_INFO("Connecting to: "<<hstring);

#ifndef JGACHINE_RESTORECONNECT  
  if (::connect(handle, saddr.getSockAddr(), saddr.getSize())== -1)
#else
  if (JGACHINE_RESTORECONNECT(handle, saddr.getSockAddr(), saddr.getSize())== -1)
#endif
    {
      close();
      JGACHINE_SERROR(std::string("Can't connect to ")+hstring);
    }
  return true;
}

Socket::~Socket()
{
  if (closeOnDelete) close();
  if (!globalDeinit()) JGACHINE_SERROR("globalDeinit");
}

bool
Socket::inAvail(const Timeout* timeout)
{
  fd_set read_fd_set;
  FD_ZERO(&read_fd_set);
  int fd=getHandle();
  FD_SET(fd,&read_fd_set);
  struct timeval ctimeout;
  if (timeout) {
    ctimeout.tv_sec=(*timeout)/1000000LL;
    ctimeout.tv_usec=(*timeout)%1000000LL;
  }
  int av=0;
  while ((av=::select (fd+1, &read_fd_set, NULL, NULL, (timeout) ? (&ctimeout) : NULL))<0) {
    if (errno!=EINTR) 
      EJS_FATAL("select failed");
  }
  return av>0;
}

//! set socket to blocking or non-blocking mode - default is blocking
void 
Socket::setBlocking(bool block)
{
#ifndef WINDOOF
  int desc=handle;
  // taken from info libc (gnu libc)
  int oldflags = fcntl (desc, F_GETFL, 0);
  // If reading the flags failed, return error indication now. 
  EJS_CHECK(oldflags!=-1);
  // Set just the flag we want to set. 
  if (!block) oldflags |= O_NONBLOCK;
  else oldflags &= ~O_NONBLOCK;
  // Store modified flag word in the descriptor. 
  EJS_CHECK(fcntl (desc, F_SETFL, oldflags)==0);
#else
  EJS_WARN("not yet implemented on WIN32");
#endif
}

bool
Socket::setTcpNoDelay(bool on)
{
  int flag = on;
  // (void *) cause of solaris
#ifdef HAVE_TCP_NODELAY
  return !setsockopt(getHandle(), IPPROTO_TCP, TCP_NODELAY, (JGACHINE_SOCKOPTVALT) &flag, sizeof(int));
#else
  return false;
#endif  
}

int
Socket::read(void *buf, size_t count)
{
  return recv(getHandle(),(char *)buf,count,0);
}

int
Socket::write(const void *buf, size_t count)
{
  return send(getHandle(),(char *)buf,count,0);
}

Socket *
Socket::accept()
{
  int newHandle;
  struct sockaddr_in clientname;
  socklen_t struct_size = sizeof (clientname);

  newHandle = ::accept (getHandle(),(struct sockaddr *) &clientname,&struct_size);
  if (newHandle == -1) {
    if ((errno!=EINTR)&&(errno!=EWOULDBLOCK)) 
      JGACHINE_SERROR("can't accept connection");
    // recoverable error but no new connection
    return NULL;
  }
  return new Socket(clientname.sin_addr.s_addr,clientname.sin_port,newHandle);
}

void
Socket::close()
{
  if (handle==-1) return;
  if (closesocket(handle)<0) EJS_WARN("close failed");
  handle=-1;
}
