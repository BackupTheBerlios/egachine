EXTRA_DIST = onstartup.js.in

# create script at compile time
# the autoconf manual suggests this

# Note: we in fact create one at compile time and at install time
# to allow running ejs from within the build directory without
# make install

editbuild = sed \
	-e 's,@scriptPath\@,".",g' \
	-e 's,@libraryPath\@,"src/modules/example",g'

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

all-local: onstartup.js

clean-local:
	rm -f onstartup.js onstartup.build onstartup.install
