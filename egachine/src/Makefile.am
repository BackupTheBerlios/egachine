SUBDIRS = etc modules examples

CXXFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" @CXXFLAGS@
INCLUDES = @LTDLINCL@ @EJS_CFLAGS@

bin_PROGRAMS = ejs

ejs_SOURCES = ejsmoduleloader.h ejs.cpp ejsmoduleloader.cpp

# keep this in sync with src/modules/Makefile.am and configure.ac

if BUILD_SVGL
DLOPEN_MAYBE_SVGL = -dlopen modules/svgl/ejssvgl.la
DEP_MAYBE_SVGL = modules/svgl/ejssvgl.la
endif
if BUILD_ZLIB
DLOPEN_MAYBE_ZLIB = -dlopen modules/zlib/ejszlib.la
DEP_MAYBE_ZLIB = modules/zlib/ejszlib.la
endif
if BUILD_STREAM
DLOPEN_MAYBE_STREAM = -dlopen modules/stream/ejsstream.la
DEP_MAYBE_STREAM = modules/stream/ejsstream.la
endif
if BUILD_FILE
DLOPEN_MAYBE_FILE = -dlopen modules/file/ejsfile.la
DEP_MAYBE_FILE = modules/file/ejsfile.la
endif
if BUILD_NET
DLOPEN_MAYBE_NET = -dlopen modules/network/ejsnet.la
DEP_MAYBE_NET = modules/network/ejsnet.la
endif
if BUILD_TIMER
DLOPEN_MAYBE_TIMER = -dlopen modules/timer/ejstimer.la
DEP_MAYBE_TIMER = modules/timer/ejstimer.la
endif
if BUILD_GL
DLOPEN_MAYBE_GL = -dlopen modules/gl/ejsgl.la
DEP_MAYBE_GL = modules/gl/ejsgl.la
endif
if BUILD_VIDEO
DLOPEN_MAYBE_VIDEO = -dlopen modules/video/ejsvideo.la
DEP_MAYBE_VIDEO = modules/video/ejsvideo.la
endif
if BUILD_INPUT
DLOPEN_MAYBE_INPUT = -dlopen modules/input/ejsinput.la
DEP_MAYBE_INPUT = modules/input/ejsinput.la
endif
if BUILD_AUDIO
DLOPEN_MAYBE_AUDIO = -dlopen modules/audio/ejsaudio.la
DEP_MAYBE_AUDIO = modules/audio/ejsaudio.la
endif
if BUILD_TCC
DLOPEN_MAYBE_TCC = -dlopen modules/tcc/ejstcc.la
DEP_MAYBE_TCC = modules/tcc/ejstcc.la
endif

ejs_LDADD = @LIBLTDL@ \
	-dlopen modules/example/ejsexample.la\
	-dlopen modules/monitorable/ejsmonitorable.la\
	-dlopen modules/bytearray/ejsbytearray.la\
	-dlopen modules/egachine/ejsegachine.la\
	-dlopen modules/util/ejsutil.la \
	$(DLOPEN_MAYBE_ZLIB) \
	$(DLOPEN_MAYBE_STREAM) \
	$(DLOPEN_MAYBE_FILE) \
	$(DLOPEN_MAYBE_NET) \
	$(DLOPEN_MAYBE_TIMER) \
	$(DLOPEN_MAYBE_GL) \
	$(DLOPEN_MAYBE_VIDEO) \
	$(DLOPEN_MAYBE_INPUT) \
	$(DLOPEN_MAYBE_AUDIO) \
	$(DLOPEN_MAYBE_TCC) \
	$(DLOPEN_MAYBE_SVGL) 

# note -export-symbols-regex only works since we use a hacked libtool
# and better would be called -import-symbols-regex
# -export-dynamic is not really required (but we could perhaps use it
# do print some stack-trace)
ejs_LDFLAGS = -export-dynamic -export-symbols-regex ejs.*_LTX_.*Load

ejs_DEPENDENCIES = @LIBLTDL@ \
	modules/example/ejsexample.la\
	modules/monitorable/ejsmonitorable.la\
	modules/bytearray/ejsbytearray.la\
	modules/egachine/ejsegachine.la\
	modules/util/ejsutil.la \
	$(DEP_MAYBE_ZLIB) \
	$(DEP_MAYBE_STREAM) \
	$(DEP_MAYBE_FILE) \
	$(DEP_MAYBE_NET) \
	$(DEP_MAYBE_TIMER) \
	$(DEP_MAYBE_GL) \
	$(DEP_MAYBE_VIDEO) \
	$(DEP_MAYBE_INPUT) \
	$(DEP_MAYBE_AUDIO) \
	$(DEP_MAYBE_TCC) \
	$(DEP_MAYBE_SVGL) 

EXTRA_DIST = egares.in egaserver.in egachine.in

all-local: egares egaserver egachine

editbuild = sed \
	-e 's,@bindir\@,@abs_top_builddir@/src,g' \
	-e 's,@PACKAGE_VERSION\@,@PACKAGE_VERSION@,g'

editinst = sed \
	-e 's,@bindir\@,$(bindir),g' \
	-e 's,@PACKAGE_VERSION\@,@PACKAGE_VERSION@,g'

egares: egares.build egares.install
	rm -f egares
	cp egares.build egares
	chmod +x egares

egares.build: Makefile $(srcdir)/egares.in
	rm -f egares.build
	$(editbuild) $(srcdir)/egares.in > egares.build

egares.install: Makefile $(srcdir)/egares.in
	rm -f egares.install
	$(editinst) $(srcdir)/egares.in > egares.install

egaserver: egaserver.build egaserver.install
	rm -f egaserver
	cp egaserver.build egaserver
	chmod +x egaserver

egaserver.build: Makefile $(srcdir)/egaserver.in
	rm -f egaserver.build
	$(editbuild) $(srcdir)/egaserver.in > egaserver.build

egaserver.install: Makefile $(srcdir)/egaserver.in
	rm -f egaserver.install
	$(editinst) $(srcdir)/egaserver.in > egaserver.install

egachine: egachine.build egachine.install
	rm -f egachine
	cp egachine.build egachine
	chmod +x egachine

egachine.build: Makefile $(srcdir)/egachine.in
	rm -f egachine.build
	$(editbuild) $(srcdir)/egachine.in > egachine.build

egachine.install: Makefile $(srcdir)/egachine.in
	rm -f egachine.install
	$(editinst) $(srcdir)/egachine.in > egachine.install

install-data-local: egares egaserver egachine
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	$(INSTALL_SCRIPT) egares.install $(DESTDIR)$(bindir)/egares
	$(INSTALL_SCRIPT) egaserver.install $(DESTDIR)$(bindir)/egaserver
	$(INSTALL_SCRIPT) egachine.install $(DESTDIR)$(bindir)/egachine

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/egares $(DESTDIR)$(bindir)/egaserver $(DESTDIR)$(bindir)/egachine

clean-local:
	rm -f egares egares.build egares.install
	rm -f egaserver egaserver.build egaserver.install
	rm -f egachine egachine.build egachine.install
