/* what's this?
 test when precision is lost
 53Bits are okay
 this conforms to http://docs.sun.com/source/806-3568/ncg_goldberg.html
*/

#include <iostream>

typedef unsigned long long uint64;
uint64 dconv(uint64 i)
{
  double d=i;
  uint64 r=d;
  return r;
}


int main()
{
  std::cout.setf(std::ios::fixed);
  uint64 i=~0ULL>>11;
  std::cout << i << std::endl;
  std::cout << dconv(i) << std::endl;
}
