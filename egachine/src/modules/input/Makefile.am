INCLUDES = @EJS_CFLAGS@ @SDL_CFLAGS@

pkglib_LTLIBRARIES = ejsinput.la

ejsinput_la_SOURCES = \
	input.cpp \
	input.h \
	ejsinput.cpp

ejsinput_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SDL_LIBS@

dist_pkgdata_DATA = input.js
EXTRA_DIST = export.sym
