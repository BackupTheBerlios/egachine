svgl = svgl svgldom
if BUILD_SVGL
MAYBE_SVGL = $(svgl)
endif

tests = \
	audio \
	ejsexit \
	example \
	load \
	untrusted \
	zlib \
	monitorable \
	stream \
	file \
	net \
	server \
	egares \
	timer \
	scope \
	smbug240577 \
	opengl

check_SCRIPTS = \
	$(tests) \
	$(MAYBE_SVGL)

TESTS = $(check_SCRIPTS)

# TESTS_ENVIRONMENT = abs_top_srcdir="@abs_top_srcdir@"

EXTRA_DIST = \
	load.js \
	untrusted.js \
	common \
	$(tests) \
	$(svgl)

