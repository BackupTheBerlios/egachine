INCLUDES = @EJS_CFLAGS@ -I\$(top_srcdir) @SIGC_CFLAGS@

pkglib_LTLIBRARIES = ejstimer.la

ejstimer_la_SOURCES = \
	timestamp.h timestamp.cpp ejstimer.cpp

ejstimer_la_LDFLAGS = -module @MODULE_LDFLAGS@
EXTRA_DIST = export.sym
