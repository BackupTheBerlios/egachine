INCLUDES = @EJS_CFLAGS@

pkglib_LTLIBRARIES = ejstcc.la

ejstcc_la_SOURCES = ejstcc.cpp
ejstcc_la_LDFLAGS = -module @MODULE_LDFLAGS@ @TCC_LIBS@

dist_pkgdata_DATA = tcc.js
EXTRA_DIST = export.sym
