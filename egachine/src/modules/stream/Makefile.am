# top_srcdir is required for boost
INCLUDES = @EJS_CFLAGS@ -I\$(top_srcdir)

pkglib_LTLIBRARIES = ejsstream.la

ejsstream_la_SOURCES = ejsstream.cpp
ejsstream_la_LDFLAGS = -module @MODULE_LDFLAGS@
EXTRA_DIST = export.sym
