#!/bin/ash
CXX=${CXX:=c++}
LDFLAGS=${LDFLAGS:=-L/home/jens/develop/egachine/spidermonkey/src/Linux_All_OPT.OBJ -lsmjs}
CXXFLAGS=${CXXFLAGS:=-I/home/jens/develop/egachine/spidermonkey/src -I/home/jens/develop/egachine/spidermonkey/src/Linux_All_OPT.OBJ -DXP_UNIX -Wall -W}
echo $CXX -o errtest $CXXFLAGS errtest.cpp $LDFLAGS
$CXX -o errtest $CXXFLAGS errtest.cpp $LDFLAGS
./errtest <<EOF
function foo()
{
	bar();
}
function foo2()
{
	print(1,2,3);
}

try{
	foo();
}catch(error){
	print(error);
	print(error.stack);
}
try{
	foo2();
	print("should not be reached");
}catch(error){
	print("should be reached");
	print(error);
	print(error.stack);
}
EOF
