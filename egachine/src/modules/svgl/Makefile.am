INCLUDES = @EJS_CFLAGS@ @SVGL_CFLAGS@

pkglib_LTLIBRARIES = ejssvgl.la

ejssvgl_la_SOURCES = \
	ejssvgl.cpp \
	ejssvgdocument.h \
	ejssvgdocument.cpp \
	ejsnode.h \
	ejsnode.cpp \
	ejselement.h \
	ejselement.cpp

ejssvgl_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SVGL_LIBS@

dist_pkgdata_DATA = svgl.js
EXTRA_DIST = export.sym
