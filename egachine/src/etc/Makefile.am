EXTRA_DIST = onstartup.js.in

# create script at compile time
# the autoconf manual suggests this

# Note: we in fact create one at compile time and at install time
# to allow running ejs from within the build directory without
# make install

editbuild = sed \
	-e 's,@scriptPath\@,\
	"@abs_top_srcdir@/src/modules/example"\,\
	"@abs_top_srcdir@/src/modules/zlib"\,\
	"@abs_top_srcdir@/src/modules/monitorable"\,\
	"@abs_top_srcdir@/src/modules/stream"\,\
	"@abs_top_srcdir@/src/modules/file"\,\
	"@abs_top_srcdir@/src/modules/network"\,\
	"@abs_top_srcdir@/src/modules/bytearray"\,\
	"@abs_top_srcdir@/src/modules/egachine"\,\
	"@abs_top_srcdir@/src/modules/base64"\,\
	"@abs_top_srcdir@/src/modules/timer"\,\
	"@abs_top_srcdir@/src/modules/gl"\,\
	"@abs_top_srcdir@/src/modules/video"\,\
	"@abs_top_srcdir@/src/modules/input"\,\
	"@abs_top_srcdir@/src/modules/audio"\,\
	"@abs_top_srcdir@/src/modules/svgl"\,\
	"@abs_top_srcdir@/src/modules/util"\,\
	"@abs_top_builddir@/src/modules/graphviz"\,\
	,g' \
	-e 's,@libraryPath\@,\
	"@abs_top_builddir@/src/modules/example"\,\
	"@abs_top_builddir@/src/modules/zlib"\,\
	"@abs_top_builddir@/src/modules/monitorable"\,\
	"@abs_top_builddir@/src/modules/stream"\,\
	"@abs_top_builddir@/src/modules/file"\,\
	"@abs_top_builddir@/src/modules/network"\,\
	"@abs_top_builddir@/src/modules/bytearray"\,\
	"@abs_top_builddir@/src/modules/egachine"\,\
	"@abs_top_builddir@/src/modules/timer"\,\
	"@abs_top_builddir@/src/modules/gl"\,\
	"@abs_top_builddir@/src/modules/video"\,\
	"@abs_top_builddir@/src/modules/input"\,\
	"@abs_top_builddir@/src/modules/audio"\,\
	"@abs_top_builddir@/src/modules/svgl"\,\
	"@abs_top_builddir@/src/modules/util"\,\
	,g'

editinst = sed \
	-e 's,@scriptPath\@,"$(pkgdatadir)",g' \
	-e 's,@libraryPath\@,"$(pkglibdir)",g'

onstartup.build: Makefile $(srcdir)/onstartup.js.in
	rm -f onstartup.build
	$(editbuild) $(srcdir)/onstartup.js.in >onstartup.build

onstartup.install: Makefile $(srcdir)/onstartup.js.in
	rm -f onstartup.install
	$(editinst) $(srcdir)/onstartup.js.in >onstartup.install

onstartup.js: onstartup.build onstartup.install
	rm -f onstartup.js
	cp onstartup.build onstartup.js

install-data-local: onstartup.js
	$(mkinstalldirs) $(DESTDIR)$(sysconfdir)/$(PACKAGE)
	$(INSTALL_DATA) onstartup.install $(DESTDIR)$(sysconfdir)/$(PACKAGE)/onstartup.js

uninstall-local:
	rm -f $(DESTDIR)$(sysconfdir)/$(PACKAGE)/onstartup.js

all-local: onstartup.js

clean-local:
	rm -f onstartup.js onstartup.build onstartup.install
