INCLUDES = @EJS_CFLAGS@ @SVGL_CFLAGS@

pkglib_LTLIBRARIES = ejssvgl.la

ejssvgl_la_SOURCES = \
	ejssvgl.cpp \
	ejssvgdocument.h \
	ejssvgdocument.cpp \
	nodefdefs.h \
	nodefuncs.h \
	ejsnode.h \
	ejsnode.cpp \
	ejsnodelist.h \
	ejsnodelist.cpp \
	ejselement.h \
	ejselement.cpp \
	ejstext.h \
	ejstext.cpp \
	ejsallelements.h \
	strutils.h

ejssvgl_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SVGL_LIBS@

dist_pkgdata_DATA = svgl.js
EXTRA_DIST = export.sym
