#ifndef EGACHINE_TIME_H
#define EGACHINE_TIME_H

//! Timer subsystem Interface
/*!
  unit for the TimeStamp is usec: 10^-6 sec
*/

//todo - one central definition for 64bit int types
//perhaps autoconf again
typedef long long int64_t;

namespace Timer
{
  typedef int64_t TimeStamp;

  void init();
  void deinit();
  
  TimeStamp getTimeStamp();
  void uSleep(TimeStamp usec);
}

#endif
