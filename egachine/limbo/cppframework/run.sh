#!/bin/sh
set -e
CXX=${CXX:=c++}
#LDFLAGS=${LDFLAGS:=-lsmjs}
LDFLAGS="$LDFLAGS -lsmjs"
CXXFLAGS=${CXXFLAGS:=-DXP_UNIX -DJS_THREADSAFE -Wall -W -O9}
echo $CXX -o test $CPPFLAGS $CXXFLAGS test.cpp $LDFLAGS
$CXX -o test $CPPFLAGS $CXXFLAGS test.cpp $LDFLAGS
valgrind --leak-check=yes ./test <test.js
