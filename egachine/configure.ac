dnl Process this file with autoconf to produce a configure script.
dnl Please note this file was autogenerated by bootstrap

AC_INIT(egachine, 0.1.0, karme@berlios.de)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([src/ejs.cpp])

dnl check for progs
AC_PROG_CXX

dnl libtool and libltdl
dnl by default we want our modules to be compiled as shared libraries
AC_DISABLE_STATIC
dnl AC_LIBLTDL_CONVENIENCE
AC_LIBLTDL_INSTALLABLE
AC_SUBST(LTDLINCL)
AC_SUBST(LIBLTDL)
AC_LIBTOOL_DLOPEN
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL
dnl AC_CONFIG_SUBDIRS(libltdl)

dnl we only use the c++ compiler (=> we do want to do the tests with c++
dnl because we need CXXFLAGS right and not only CFLAGS)
AC_LANG(C++)

dnl check for spidermonkey

dnl Get misc stuff right
dnl todo: this is shit and should be done by spidermonkey itself
case "$host" in
	*-*-mingw32*)
		CPPFLAGS="$CPPFLAGS -DXP_WIN"
		;;
	*-*-linux*)
		CPPFLAGS="$CPPFLAGS -DXP_UNIX"
		;;
	*)
		AC_MSG_WARN([Unknown host type: $host assuming unix for spidermonkey])
		;;
esac
dnl todo:
dnl how to detect if the installed spidermonkey library was compiled with JS_THREADSAFE?
dnl probably if it uses libnspr
dnl why do we have to know? because jsapi.h has some "#ifdef JS_THREADSAFE" ...
dnl for example JS_GET_CLASS
dnl if we detect this the wrong way this will result in stack corruption
dnl CPPFLAGS="$CPPFLAGS -DJS_THREADSAFE"

AC_CHECK_HEADER([jsapi.h], [], [
	AC_MSG_ERROR([Could not find required header 'jsapi.h'
Perhaps you have to add some -I/path/tojsapiheader to your CPPFLAGS])
])

AC_SEARCH_LIBS(JS_InitStandardClasses, [js smjs mozjs], , [
	AC_MSG_ERROR([Could not find required spidermonkey Javascript library])
])


AC_SUBST(EJS_CFLAGS, ["-I\$(top_srcdir)/src/modules"])

dnl optional modules

dnl the zlib module
AC_ARG_ENABLE([zlib],
	AS_HELP_STRING([--enable-zlib],
	[Build zlib module [[default=yes]]]),
	[], enable_zlib=yes)
if test x$enable_zlib = xyes; then
	AC_CHECK_HEADER([zlib.h], [], [
		AC_MSG_ERROR([Could not find required header 'zlib.h'
Perhaps you have to add some -I/path/to/zlibheader to your CPPFLAGS
or disable the zlib module by passing --disable-zlib to configure])
	])
	AC_SEARCH_LIBS(compress, [z], , [
		AC_MSG_ERROR([Could not find required zlib library
Perhaps you have to add some -L/path/to/libz to your LDFLAGS
or disable the zlib module by passing --disable-zlib to configure])
	])
fi


dnl stream module
AC_ARG_ENABLE([stream],
	AS_HELP_STRING([--enable-stream],
	[Build stream module [[default=yes]]]),
	[], enable_stream=yes)
dnl if test x$enable_stream = xyes; then
dnl fi


dnl the file module
AC_ARG_ENABLE([file],
	AS_HELP_STRING([--enable-file],
	[Build file module [[default=yes]]]),
	[], enable_file=yes)
if test x$enable_file = xyes; then
	dnl file module requires stream module
	if test x$enable_stream != xyes; then
		AC_MSG_ERROR([file module requires stream module])
	fi
fi


dnl the net module
AC_ARG_ENABLE([net],
	AS_HELP_STRING([--enable-net],
	[Build net (network) module [[default=yes]]]),
	[], enable_net=yes)
if test x$enable_net = xyes; then
	dnl net module requires stream module
	if test x$enable_stream != xyes; then
		AC_MSG_ERROR([net module requires stream module])
	fi
	dnl networking module requires sigc 1.0.x
	AM_PATH_SIGC(1.0.0)
fi

dnl the gl module (opengl)
AC_ARG_ENABLE([gl],
	AS_HELP_STRING([--enable-gl],
	[Build gl (opengl) module [[default=yes]]]),
	[], enable_gl=yes)
if test x$enable_gl = xyes; then
	dnl check for opengl - todo: improve this
	case "$host" in
	    *-*-cygwin* | *-*-mingw32*)
        	GL_LIBS="-lglu32 -lopengl32"
        	;;
    	*)
		GL_LIBS="-lGLU -lGL"
        	;;
	esac
	AC_SUBST(GL_LIBS)
fi

dnl the video module
AC_ARG_ENABLE([video],
	AS_HELP_STRING([--enable-video],
	[Build video module [[default=yes]]]),
	[], enable_video=yes)
if test x$enable_video = xyes; then
	dnl video module requires gl module
	if test x$enable_gl != xyes; then
		AC_MSG_ERROR([video module requires gl module])
	fi
	dnl the current video module uses SDL
	dnl check for SDL
	dnl check only works with c-compiler (only CFLAGS are set)
	AC_LANG_PUSH(C)
	AM_PATH_SDL(1.2.0, , [AC_MSG_ERROR([SDL required for video module])])

	dnl Check for SDL OpenGL support
	AC_MSG_CHECKING(for OpenGL support in SDL)
	TMP="$CFLAGS"
	CFLAGS="$CFLAGS $SDL_CFLAGS"
	have_opengl=no
	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include "SDL_opengl.h"
		]])],
		[have_opengl=yes],
		[AC_MSG_ERROR([not available])])
	CFLAGS="$TMP"
	AC_MSG_RESULT($have_opengl)
	AC_LANG_POP(C)
	dnl todo check for SDL_image
	SDL_LIBS="-lSDL_image $SDL_LIBS"
fi

dnl the input module
AC_ARG_ENABLE([input],
	AS_HELP_STRING([--enable-input],
	[Build input module [[default=yes]]]),
	[], enable_input=yes)
if test x$enable_input = xyes; then
	dnl Note: the current input module uses SDL and depends
	dnl on the (current) video module using SDL
	dnl input module requires video module
	if test x$enable_video != xyes; then
		AC_MSG_ERROR([input module requires video module])
	fi
fi

dnl the audio module
AC_ARG_ENABLE([audio],
	AS_HELP_STRING([--enable-audio],
	[Build audio module [[default=yes]]]),
	[], enable_audio=yes)
if test x$enable_audio = xyes; then
	dnl Note: the current audio module uses SDL_mixer / SDL
	dnl we need Mix_LoadWAV_RW (at the moment only supported by cvs version of SDL_mixer)
	dnl todo: improve this check and check if the header file is also correct
	TMP="$LDFLAGS"
	AC_CHECK_LIB(SDL_mixer, Mix_LoadWAV_RW, [SDLMIXER_LIBS="-lSDL_mixer"], [
		AC_MSG_ERROR([Could not find required SDL_mixer library
Perhaps you have to add some -L/path/to/libz to your LDFLAGS
or your SDL_mixer library is missing Mix_LoadWAV_RW - you need a version newer than 1.2.5
or disable the audio module by passing --disable-audio to configure])
], [$SDL_LIBS])
	LDFLAGS="$TMP"
	AC_SUBST(SDLMIXER_LIBS)
fi

dnl todo: thread-safety
LDFLAGS="$LDFLAGS -thread-safe -pthread"
MODULE_LDFLAGS="-avoid-version -no-undefined -export-symbols \$(srcdir)/export.sym"
AC_SUBST(MODULE_LDFLAGS)

dnl output files
AC_OUTPUT([
	Makefile
	tests/Makefile
	src/Makefile
	src/etc/Makefile
	src/examples/Makefile
	src/modules/Makefile
	src/modules/example/Makefile
	src/modules/zlib/Makefile
	src/modules/monitorable/Makefile
	src/modules/stream/Makefile
	src/modules/file/Makefile
	src/modules/network/Makefile
	src/modules/bytearray/Makefile
	src/modules/egachine/Makefile
	src/modules/base64/Makefile
	src/modules/timer/Makefile
	src/modules/gl/Makefile
	src/modules/video/Makefile
	src/modules/input/Makefile
	src/modules/audio/Makefile
	boost/Makefile
])
