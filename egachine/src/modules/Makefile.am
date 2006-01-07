# keep this in sync with src/Makefile.am and configure.ac

if BUILD_SVGL
MAYBE_SVGL = svgl
endif
if BUILD_ZLIB
MAYBE_ZLIB = zlib
endif
if BUILD_STREAM
MAYBE_STREAM = stream
endif
if BUILD_FILE
MAYBE_FILE = file
endif
if BUILD_NET
MAYBE_NET = network
endif
if BUILD_TIMER
MAYBE_TIMER = timer
endif
if BUILD_GL
MAYBE_GL = gl
endif
if BUILD_VIDEO
MAYBE_VIDEO = video
endif
if BUILD_IMAGE
MAYBE_IMAGE = image
endif
if BUILD_INPUT
MAYBE_INPUT = input
endif
if BUILD_AUDIO
MAYBE_AUDIO = audio
endif
if BUILD_TCC
MAYBE_TCC = tcc
endif
if BUILD_LTDL
MAYBE_LTDL = ltdl
endif

SUBDIRS = \
	example \
	$(MAYBE_ZLIB) \
	monitorable \
	$(MAYBE_STREAM) \
	$(MAYBE_FILE) \
	$(MAYBE_NET) \
	bytearray \
	egachine \
	$(MAYBE_TIMER) \
	$(MAYBE_GL) \
	$(MAYBE_VIDEO) \
	$(MAYBE_IMAGE) \
	$(MAYBE_INPUT) \
	$(MAYBE_AUDIO) \
	$(MAYBE_TCC) \
	$(MAYBE_LTDL) \
	$(MAYBE_SVGL) \
	util \
	jsmisc \
	jsolait

include_HEADERS=ejsmodule.h
