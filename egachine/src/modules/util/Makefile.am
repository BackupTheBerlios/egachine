INCLUDES = @EJS_CFLAGS@ -DCC="$(CC)" -DCXX="$(CXX)" -DDEFS="$(DEFS)" -DEJS_CFLAGS="@EJS_CFLAGS@" -DCPPFLAGS="$(CPPFLAGS)" -DAM_CPPFLAGS="$(AM_CPPFLAGS)" -DMODULE_LDFLAGS="@MODULE_LDFLAGS@"

pkglib_LTLIBRARIES = ejsutil.la

ejsutil_la_SOURCES = ejsutil.cpp
ejsutil_la_LDFLAGS = -module @MODULE_LDFLAGS@

dist_pkgdata_DATA = util.js
EXTRA_DIST = export.sym


