#!/bin/bash
set -e
cc -g -O0 -DXP_UNIX -o bug -I../spidermonkey-1.5rc6/src -I../spidermonkey-1.5rc6/src/Linux_All_DBG.OBJ bug.c -L../spidermonkey-1.5rc6/src/Linux_All_DBG.OBJ/ -lsmjs -lm -lefence
gdb ./bug -batch -x <(for a in r bt "up 6" "print a" "print *a" "print pool" "print p" "print size" "print incr" "print ap" "print *ap" "print b" "print boff" "print aoff" "print extra" "print hdrsz" up l; do echo echo $a\\n; echo $a; done) </dev/null
