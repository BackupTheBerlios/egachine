svgl = svgl svgldom
zlib = zlib
stream = stream
file = file
net = net server
audio = audio
timer = timer
gl = opengl
tcc = tcc
ltdl = ltdl
posix = posix
io = io

if BUILD_SVGL
MAYBE_SVGL = $(svgl)
endif
if BUILD_ZLIB
MAYBE_ZLIB = $(zlib)
endif
if BUILD_STREAM
MAYBE_STREAM = $(stream)
endif
if BUILD_FILE
MAYBE_FILE = $(file)
endif
if BUILD_NET
MAYBE_NET = $(net)
endif
if BUILD_AUDIO
MAYBE_AUDIO = $(audio)
endif
if BUILD_TIMER
MAYBE_TIMER = $(timer)
endif
if BUILD_GL
MAYBE_GL = $(gl)
endif
if BUILD_TCC
MAYBE_TCC = $(tcc)
endif
if BUILD_LTDL
MAYBE_LTDL = $(ltdl)
endif
if BUILD_POSIX
MAYBE_POSIX = $(posix)
endif
if BUILD_IO
MAYBE_IO = $(io)
endif

tests = \
	ejsexit \
	example \
	load \
	untrusted \
	monitorable \
	egares \
	scope \
	smbug240577 \
	jsolait \
	util \
	jsstl \
	tmp \
	cc \
	namespace

check_SCRIPTS = \
	$(tests) \
	$(MAYBE_ZLIB) \
	$(MAYBE_STREAM) \
	$(MAYBE_FILE) \
	$(MAYBE_NET) \
	$(MAYBE_AUDIO) \
	$(MAYBE_TIMER) \
	$(MAYBE_GL) \
	$(MAYBE_SVGL) \
	$(MAYBE_TCC) \
	$(MAYBE_LTDL) \
	$(MAYBE_POSIX) \
	$(MAYBE_IO)

TESTS = $(check_SCRIPTS)

# TESTS_ENVIRONMENT = abs_top_srcdir="@abs_top_srcdir@"

EXTRA_DIST = \
	load.js \
	untrusted.js \
	common \
	$(tests) \
	$(svgl) $(zlib) $(stream) $(file) $(net) $(audio) $(timer) $(gl)

