#include "sdlgl.h"

#include <iostream>
#include <iomanip>
#include "error.h"

int
printGLErrors()
{
  int res=0;
  int e;
  while ((e=glGetError())) {
    JGACHINE_MSG("GL Error: ", e);
    std::ios::fmtflags old=std::cerr.flags();
    std::cerr << "GL Error: " << e << "=0x" << std::hex << e << std::endl;
    std::cerr.flags(old);
    ++res;
    if (res>5) {
      JGACHINE_WARN("stop printing errors because there are too much (parhaps some other problem)");
      return res;
    }
  }
  return res;
}



