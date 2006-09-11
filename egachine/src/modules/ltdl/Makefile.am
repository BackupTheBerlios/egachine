INCLUDES = @LTDLINCL@ @EJS_CFLAGS@

pkglib_LTLIBRARIES = ejsltdl.la

ejsltdl_la_SOURCES = ejsltdl.cpp
ejsltdl_la_LDFLAGS = -module @MODULE_LDFLAGS@ @LIBLTDL@

dist_pkgdata_DATA = ltdl.js
EXTRA_DIST = export.sym
