SUBDIRS = etc modules

CXXFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" @CXXFLAGS@
INCLUDES = -I\$(top_srcdir) @LTDLINCL@

bin_PROGRAMS = ejs

ejs_SOURCES = ejsmoduleloader.h macros.h ejs.cpp ejsmoduleloader.cpp
ejs_LDADD = @LIBLTDL@
ejs_DEPENDENCIES = @LIBLTDL@
