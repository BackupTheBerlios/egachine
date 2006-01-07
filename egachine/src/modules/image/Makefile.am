INCLUDES = @EJS_CFLAGS@ @SDL_CFLAGS@

pkglib_LTLIBRARIES = ejsimage.la

ejsimage_la_SOURCES = \
	ejsimage.cpp 

ejsimage_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SDL_LIBS@ @GL_LIBS@

EXTRA_DIST = export.sym
