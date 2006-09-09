INCLUDES = @EJS_CFLAGS@ -I\$(top_srcdir)

pkglib_LTLIBRARIES = ejsfile.la

ejsfile_la_SOURCES = fdstream.hpp ejsfile.cpp mkdtemp.h mkdtemp.c
ejsfile_la_LDFLAGS = -module @MODULE_LDFLAGS@

dist_pkgdata_DATA = file.js
EXTRA_DIST = export.sym
