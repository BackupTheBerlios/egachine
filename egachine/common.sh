# this file is sourced by: (all build scripts)
# ./make
# ./crossmake
# ./install
# ./clean

# debug build?
#DEBUG=1

# verbose compilation
VERBOSE=1

function run() {
    if [ "x$VERBOSE" != "x" ]; then echo "$1"; fi
    $1
}

if test ! -f $(pwd)/src/egachine.js; then
	echo "You must type ./make in the top level directory"
	exit 1
fi

TOPSRCDIR=${TOSRCDIR:=$(pwd)}
SRCDIR=${SRCDIR:=$TOPSRCDIR/src}
#SPIDERMONKEY_SRCDIR=$TOPSRCDIR/spidermonkey-1.5rc6/src
SPIDERMONKEY_SRCDIR=$TOPSRCDIR/spidermonkey/src
export LC_ALL=C

source VERSION
DEFINES="\
    -DPACKAGE_MAJOR_VERSION=$PACKAGE_MAJOR_VERSION \
    -DPACKAGE_MINOR_VERSION=$PACKAGE_MINOR_VERSION \
    -DPACKAGE_MICRO_VERSION=$PACKAGE_MICRO_VERSION \
    -DPACKAGE_VERSION=\"$PACKAGE_VERSION\""

CC=${CC:=cc}
CXX=${CXX:=c++}
CXXFLAGS=${CXXFLAGS:=-Wall -W -O2}
CXXFLAGS="$CXXFLAGS $DEFINES -D_REENTRANT"

if [ "x$EGACHINE_CROSS" = "x" ]; then
    PREFIX=${PREFIX:=/usr/local}
    BINDIR=${BINDIR:=$PREFIX/bin}
    LIBDIR=${LIBDIR:=$PREFIX/lib}
    DATADIR=${DATADIR:=$PREFIX/share}
    DOCDIR=${DOCDIR:=$PREFIX/share/doc}
    SYSCONFDIR=${SYSCONFDIR:=$PREFIX/etc}

    # todo non-linux platforms will use other dirs
    if [ "x$DEBUG" = "x" ]; then
	SM_BUILDDIR="Linux_All_OPT.OBJ"
	BUILD_OPT="BUILD_OPT=1"
    else
	SM_BUILDDIR="Linux_All_DBG.OBJ"
    fi
fi


PROGRAMS="egachine egaserver egares"
