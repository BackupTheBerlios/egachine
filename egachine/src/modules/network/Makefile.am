INCLUDES = @EJS_CFLAGS@ -I\$(top_srcdir) @SIGC_CFLAGS@

pkglib_LTLIBRARIES = ejsnet.la

ejsnet_la_SOURCES = \
	ejsnet.h ejsnet.cpp \
	netstreambuf.h \
	socket.h socket.cpp \
	netstreambufserver.h netstreambufserver.cpp \
	server.h \
	ejsserver.h ejsserver.cpp

ejsnet_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SIGC_LIBS@ @SOCKET_LIBS@

dist_pkgdata_DATA = net.js
EXTRA_DIST = export.sym
