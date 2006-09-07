INCLUDES = @EJS_CFLAGS@

pkglib_LTLIBRARIES = ejsposix.la

ejsposix_la_SOURCES = ejsposix.cpp
ejsposix_la_LDFLAGS = -module @MODULE_LDFLAGS@

dist_pkgdata_DATA = posix.js
EXTRA_DIST = export.sym
