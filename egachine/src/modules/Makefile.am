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
MAYBE_TIMER = gl
endif
if BUILD_VIDEO
MAYBE_TIMER = video
endif
if BUILD_INPUT
MAYBE_TIMER = input
endif
if BUILD_AUDIO
MAYBE_TIMER = audio
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
	base64 \
	$(MAYBE_TIMER) \
	$(MAYBE_GL) \
	$(MAYBE_VIDEO) \
	$(MAYBE_INPUT) \
	$(MAYBE_AUDIO) \
	$(MAYBE_SVGL)

include_HEADERS=ejsmodule.h
