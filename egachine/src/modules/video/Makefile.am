# top_srcdir is required for boost
INCLUDES = @EJS_CFLAGS@ @SDL_CFLAGS@ -I\$(top_srcdir)

pkglib_LTLIBRARIES = ejsvideo.la

ejsvideo_la_SOURCES = \
	ejsvideo.cpp \
	fontdata.h \
	glfont.cpp \
	glfont.h \
	sdlgl.cpp \
	sdlgl.h \
	texture.cpp \
	texture.h \
	video.cpp \
	video.h

ejsvideo_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SDL_LIBS@ @GL_LIBS@

dist_pkgdata_DATA = video.js
EXTRA_DIST = export.sym
