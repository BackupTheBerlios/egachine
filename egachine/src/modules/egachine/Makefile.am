INCLUDES = @EJS_CFLAGS@ -I\$(top_srcdir) @SIGC_CFLAGS@

pkglib_LTLIBRARIES = ejsegachine.la

ejsegachine_la_SOURCES = \
	egachine.cpp

ejsegachine_la_LDFLAGS = -module @MODULE_LDFLAGS@

dist_pkgdata_DATA = egachine.js
EXTRA_DIST = export.sym
