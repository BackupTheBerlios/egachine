INCLUDES = @EJS_CFLAGS@

pkglib_LTLIBRARIES = ejsgl.la

ejsgl_la_SOURCES = \
	jsgl_fspecs.h \
	jsgl_funcs.h \
	jsgl_glgetnumargs.h \
	jsglu_fspecs.h \
	jsglu_funcs.h \
	ejsgl.cpp

ejsgl_la_LDFLAGS = -module @MODULE_LDFLAGS@ @GL_LIBS@

dist_pkgdata_DATA = gl.js
EXTRA_DIST = export.sym
