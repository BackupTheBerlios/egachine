SUBDIRS = etc modules examples

CXXFLAGS = -DSYSCONFDIR=\"$(sysconfdir)\" @CXXFLAGS@
INCLUDES = @LTDLINCL@ @EJS_CFLAGS@

bin_PROGRAMS = ejs

ejs_SOURCES = ejsmoduleloader.h ejs.cpp ejsmoduleloader.cpp
ejs_LDADD = @LIBLTDL@ \
	-dlopen modules/example/ejsexample.la\
	-dlopen modules/zlib/ejszlib.la\
	-dlopen modules/monitorable/ejsmonitorable.la\
	-dlopen modules/stream/ejsstream.la\
	-dlopen modules/file/ejsfile.la\
	-dlopen modules/network/ejsnet.la\
	-dlopen modules/bytearray/ejsbytearray.la\
	-dlopen modules/egachine/ejsegachine.la\
	-dlopen modules/timer/ejstimer.la\
	-dlopen modules/gl/ejsgl.la\
	-dlopen modules/video/ejsvideo.la\
	-dlopen modules/input/ejsinput.la\
	-dlopen modules/audio/ejsaudio.la

# note -export-symbols-regex only works since we use a hacked libtool
# and better would be called -import-symbols-regex
# -export-dynamic is not really required (but we could perhaps use it
# do print some stack-trace)
ejs_LDFLAGS = -export-dynamic -export-symbols-regex ejs.*_LTX_.*Load

ejs_DEPENDENCIES = @LIBLTDL@ 

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
