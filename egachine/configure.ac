dnl Process this file with autoconf to produce a configure script.

AC_INIT(ejs, 0.1.2, karme@berlios.de)
AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CANONICAL_TARGET
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
AC_CONFIG_SUBDIRS(libltdl)


dnl gnulib checks

gt_FUNC_MKDTEMP

dnl we only use the c++ compiler (=> we do want to do the tests with c++
dnl because we need CXXFLAGS right and not only CFLAGS)
AC_LANG(C++)

dnl check for spidermonkey

dnl Get misc stuff right
dnl todo: this is shit and should be done by spidermonkey itself
case "$host" in
	*-*-cygwin* | *-*-mingw32*)
		CPPFLAGS="$CPPFLAGS -DXP_WIN"
		;;
	*-*-linux*)
		CPPFLAGS="$CPPFLAGS -DXP_UNIX"
		;;
	*)
		AC_MSG_WARN([Unknown host type: $host assuming unix for spidermonkey])
		CPPFLAGS="$CPPFLAGS -DXP_UNIX"
		;;
esac

AC_CHECK_HEADER([jsapi.h], [], [
		AC_MSG_ERROR([Could not find required header 'jsapi.h'
			Perhaps you have to add some -I/path/tojsapiheader to your CPPFLAGS])
])

EJS_CFLAGS="-I\$(top_srcdir)/src/modules"

dnl check for spidermonkey library

AC_CHECK_LIB(js, JS_InitStandardClasses, SMJS="js", [
	AC_CHECK_LIB(smjs, JS_InitStandardClasses, SMJS="smjs", [
		AC_CHECK_LIB(mozjs, JS_InitStandardClasses, SMJS="mozjs", [
			AC_MSG_ERROR([Could not find required SpiderMonkey library])
		])
	])
])

dnl check wether spidermonkey library was compiled with JS_THREADSAFE
dnl if so we must add -DJS_THREADSAFE to our CFLAGS, too
AC_CHECK_LIB($SMJS, JS_BeginRequest, EJS_CFLAGS="$EJS_CFLAGS -DJS_THREADSAFE")
LIBS="-l$SMJS $LIBS"

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
	ZLIB_LIBS=""
	AC_CHECK_LIB(z, compress, [ZLIB_LIBS="-lz"], [
		AC_MSG_ERROR([Could not find required zlib library
Perhaps you have to add some -L/path/to/libz to your LDFLAGS
or disable the zlib module by passing --disable-zlib to configure])
	])
	AC_SUBST(ZLIB_LIBS)		
	dnl check wether compressBound is available
	AC_CHECK_LIB(z, compressBound,
		AC_DEFINE(HAVE_COMPRESSBOUND, 1,
              		[Define if your zlib has the compressBound function.]),
		[])
fi
AM_CONDITIONAL([BUILD_ZLIB], [test x$enable_zlib = xyes])


dnl stream module
AC_ARG_ENABLE([stream],
	AS_HELP_STRING([--enable-stream],
	[Build stream module [[default=yes]]]),
	[], enable_stream=yes)
dnl if test x$enable_stream = xyes; then
dnl fi
AM_CONDITIONAL([BUILD_STREAM], [test x$enable_stream = xyes])


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
AM_CONDITIONAL([BUILD_FILE], [test x$enable_file = xyes])


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

	dnl test if we need any libs for socket function
	dnl todo: this is quite ugly
	SOCKET_LIBS=""
	dnl added for SunOS
	dnl unfortunately we cannot use this to detect wsock32
	dnl todo: we should not add to LIBS only to SOCKET_LIBS
	dnl how to do this? (without duplicating AC_SEARCH_LIBS
	dnl or a diff hack)
	AC_SEARCH_LIBS(socket, [socket bsd], [found_socket=1])
	AC_SEARCH_LIBS(inet_pton, nsl)

	dnl if we have libwsock32 use it - this is not as nice as AC_SEARCH_LIBS
	dnl but it seems to be the only solution in the moment
	if test "x$found_socket" = "x"; then
		AC_MSG_CHECKING([for socket in -lwsock32])
		AC_LANG([C++])
		LIBS_BACKUP="$LIBS"
		SOCKET_LIBS="-lwsock32"
		LIBS="$LIBS $SOCKET_LIBS"
		AC_TRY_LINK([
			#if defined(__WIN32__) || defined(WIN32)
				#define __USE_W32_SOCKETS
				#include <windows.h>
			#endif
			],
			[
				#ifndef WIN32
					thisfunctiondoesnotexist();
				#endif
				int  handle = socket (PF_INET, SOCK_STREAM, 0);
			],
			[AC_MSG_RESULT([yes])],
			[
				AC_MSG_ERROR([Could not find required socket function])
			]
		)
	        LIBS="$LIBS_BACKUP"
	fi
	AC_SUBST(SOCKET_LIBS)
fi
AM_CONDITIONAL([BUILD_NET], [test x$enable_net = xyes])

dnl the timer module
AC_ARG_ENABLE([timer],
	AS_HELP_STRING([--enable-timer],
	[Build timer module [[default=yes]]]),
	[], enable_timer=yes)
if test x$enable_timer = xyes; then
	TIMER_LIBS=""
	case "$host" in
		*-*-cygwin* | *-*-mingw32*)
			dnl use winmm if available todo: this is shit
			AC_MSG_CHECKING([for timeGetTime in -lwinmm])
			AC_LANG([C++])
			LIBS_BACKUP="$LIBS"
			TIMER_LIBS="-lwinmm"
			LIBS="$LIBS $TIMER_LIBS"
			AC_TRY_LINK([
				#include <windows.h>
				],
				[
					unsigned long n=timeGetTime();
				],
				[AC_MSG_RESULT([yes])],
				[
					AC_MSG_ERROR([Could not find required timeGetTime function])
				]
			)
		        LIBS="$LIBS_BACKUP"
		;;
	esac
	AC_SUBST(TIMER_LIBS)
fi
AM_CONDITIONAL([BUILD_TIMER], [test x$enable_timer = xyes])

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
	dnl TODO: gl module depends upon video module
fi
AM_CONDITIONAL([BUILD_GL], [test x$enable_gl = xyes])

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
AM_CONDITIONAL([BUILD_VIDEO], [test x$enable_video = xyes])

dnl the image module
AC_ARG_ENABLE([image],
	AS_HELP_STRING([--enable-image],
	[Build image module [[default=yes]]]),
	[], enable_image=yes)
if test x$enable_image = xyes; then
	dnl Note: the current image module uses SDL/SDL_image
	if test x$enable_video != xyes; then
		AC_MSG_ERROR([image module requires video module])
	fi
	dnl todo: remove SDL_image dependency from video module
	dnl todo check for SDL_image
	dnl SDL_LIBS="-lSDL_image $SDL_LIBS"
fi
AM_CONDITIONAL([BUILD_IMAGE], [test x$enable_image = xyes])

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
AM_CONDITIONAL([BUILD_INPUT], [test x$enable_input = xyes])

dnl the audio module
AC_ARG_ENABLE([audio],
	AS_HELP_STRING([--enable-audio],
	[Build audio module [[default=yes]]]),
	[], enable_audio=yes)
if test x$enable_audio = xyes; then
	dnl Note: the current audio module uses SDL_mixer / SDL
	dnl we need Mix_LoadWAV_RW (at the moment only supported by cvs version of SDL_mixer)
	dnl update: i think the newest release will suffice now
	dnl todo: improve this check and check if the header file is also correct
	dnl the problem is that newer SDL_mixer version might depend upon mikmod


	AC_LANG_PUSH(C)
	OLD_LIBS="$LIBS"
	OLD_CPPFLAGS="$CPPFLAGS"
	SDLMIXER_CFLAGS=""
	SDLMIXER_LIBS="-lSDL_mixer"
	CPPFLAGS="$SDL_CFLAGS $CPPFLAGS $SDLMIXER_CFLAGS"
	AC_CHECK_LIB(SDL_mixer, Mix_LoadMUS_RW, [SDLMIXER_LIBS="-lSDL_mixer"],
            	[
			dnl Newer versions of SDL_mixer might be linked against libmikmod
			dnl hmm cant call AC_CHECK_LIB again since result is cached
			dnl is there a simple way to remove a cheched result?
			AM_PATH_LIBMIKMOD([],[
					SDLMIXER_CFLAGS="$SDLMIXER_CFLAGS $LIBMIKMOD_CFLAGS"
					SDLMIXER_LIBS="-lSDL_mixer $LIBMIKMOD_LIBS"
					AC_MSG_RESULT([yes - using external mikmod])
				],
        	    		[
					AC_MSG_ERROR([Could not find required SDL_mixer library
Perhaps you have to add some -L/path/to/libz to your LDFLAGS
or your SDL_mixer library is missing Mix_LoadWAV_RW - you need a version newer than 1.2.5
or disable the audio module by passing --disable-audio to configure])
				])
		],
		["$SDL_LIBS"])
	CPPFLAGS="$OLD_CPPFLAGS"
	LIBS="$OLD_LIBS"
	AC_SUBST(SDLMIXER_CFLAGS)
	AC_SUBST(SDLMIXER_LIBS)
	AC_LANG_POP(C)
fi
AM_CONDITIONAL([BUILD_AUDIO], [test x$enable_audio = xyes])

dnl the svgl module
AC_ARG_ENABLE([svgl],
	AS_HELP_STRING([--enable-svgl],
	[Build svgl module [[default=no]]]),
	[], enable_svgl=no)
if test x$enable_svgl = xyes; then
	dnl TODO: do we need the glft check?
	dnl PKG_CHECK_MODULES(GLFT, glft)
	PKG_CHECK_MODULES(SVGL, svgl)
	SVGL_CFLAGS="$GLFT_CFLAGS $SVGL_CFLAGS"
	dnl TODO: svgl module depends upon video module
fi
AM_CONDITIONAL([BUILD_SVGL], [test x$enable_svgl = xyes])

dnl the tcc module
AC_ARG_ENABLE([tcc],
	AS_HELP_STRING([--enable-tcc],
	[Build tcc module [[default=no]]]),
	[], enable_tcc=no)
if test x$enable_tcc = xyes; then
	AC_CHECK_HEADER([libtcc.h], [ true ], [
		AC_MSG_ERROR([Could not find required header 'libtcc.h'
			Perhaps you have to add some -I/path/toheader to your CPPFLAGS])
	])
	TCC_LIBS=""
	AC_CHECK_LIB(tcc, tcc_new, [TCC_LIBS="-ltcc"], [
		AC_MSG_ERROR([Could not find required tcc library])
	])
	AC_SUBST(TCC_LIBS)
fi
AM_CONDITIONAL([BUILD_TCC], [test x$enable_tcc = xyes])

dnl the ltdl module
AC_ARG_ENABLE([ltdl],
	AS_HELP_STRING([--enable-ltdl],
	[Build ltdl module [[default=yes]]]),
	[], enable_ltdl=yes)
AM_CONDITIONAL([BUILD_LTDL], [test x$enable_ltdl = xyes])

dnl the posix module
AC_ARG_ENABLE([posix],
	AS_HELP_STRING([--enable-posix],
	[Build posix module [[default=yes]]]),
	[], enable_posix=yes)
AM_CONDITIONAL([BUILD_POSIX], [test x$enable_posix = xyes])

dnl the io module
AC_ARG_ENABLE([io],
	AS_HELP_STRING([--enable-io],
	[Build posix module [[default=yes]]]),
	[], enable_io=yes)
AM_CONDITIONAL([BUILD_IO], [test x$enable_io = xyes])

MODULE_LDFLAGS="-avoid-version -no-undefined -export-symbols \$(srcdir)/export.sym"
AC_SUBST(EJS_CFLAGS)
AC_SUBST(MODULE_LDFLAGS)

dnl output files
AC_CONFIG_FILES([
	Makefile
	tests/Makefile
	debian/Makefile
	config/Makefile
	config/m4/Makefile
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
	src/modules/timer/Makefile
	src/modules/gl/Makefile
	src/modules/video/Makefile
	src/modules/video/boost/Makefile
	src/modules/image/Makefile
	src/modules/input/Makefile
	src/modules/audio/Makefile
	src/modules/svgl/Makefile
	src/modules/util/Makefile
	src/modules/jsolait/Makefile
	src/modules/jsmisc/Makefile
	src/modules/tcc/Makefile
	src/modules/ltdl/Makefile
	src/modules/posix/Makefile
	src/modules/io/Makefile
])

AC_OUTPUT
