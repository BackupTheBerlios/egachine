#include "timestamp.h"
#include <errno.h> // errno
#include <sys/time.h> // timeval
#include <unistd.h>

#if defined(__WIN32__) || defined(WIN32)

// need timeval
#define __USE_W32_SOCKETS
#include <windows.h>
#define WINDOOF 1

#else

// we assume non-win platforms all have gettimeofday
#define HAVE_GETTIMEOFDAY 1
// unix/posix
static Timer::TimeStamp start=0;

#endif

static int initialized=0;

void
Timer::init()
{
  if (initialized++)
    return;

#ifdef WINDOOF
  timeBeginPeriod(1);
#else

  // unix/posix

#if defined(HAVE_GETTIMEOFDAY)
#else
#error "bug"
#endif

  start=Timer::getTimeStamp();
#endif
}

void
Timer::deinit()
{
  if (initialized<=0) return;
  if (--initialized)
    return;
#ifdef WINDOOF
  timeEndPeriod(1);
#endif
}

Timer::TimeStamp
Timer::getTimeStamp()
{
#if defined(HAVE_GETTIMEOFDAY)
  timeval s;
  gettimeofday(&s,NULL);
  return Timer::TimeStamp(s.tv_sec)*Timer::TimeStamp(1000000)+Timer::TimeStamp(s.tv_usec)-start;
#else
#if defined WINDOOF
  unsigned long n=timeGetTime();
  return Timer::TimeStamp(n)*1000LL;
#else
#error "bug"
#endif
#endif

}

void
Timer::uSleep(Timer::TimeStamp usec)
{
  if (usec<=0) return;
#ifndef WINDOOF
  fd_set z;
  FD_ZERO(&z);
  timeval s;
  s.tv_sec=usec/1000000LL;
  s.tv_usec=usec%1000000LL;
  // todo: 
  // could be interrupted by a signal !!
  // on linux we could use the timeout paramter if select was interrupted by a signal
  // but this would be non-std
  int ret;
  if (((ret=select(1,&z,&z,&z,&s))==-1)&&(errno!=EINTR)) {
    // todo: we are in trouble
    return;
  }
#else
  unsigned msec=usec/1000;
  Sleep(msec);
#endif  
}
