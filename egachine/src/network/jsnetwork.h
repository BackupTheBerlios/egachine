#ifndef EGACHINE_JSNETWORK_H
#define EGACHINE_JSNETWORK_H

#include "ecmascript.h"
#include "network.h"

namespace JSNetwork
{
  bool init();
  bool deinit();

  JSBool newStreamObject(NetStreamBuf* s, jsval* rval);
}

#endif
