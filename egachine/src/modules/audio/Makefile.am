INCLUDES = @EJS_CFLAGS@ @SDL_CFLAGS@

pkglib_LTLIBRARIES = ejsaudio.la

ejsaudio_la_SOURCES = \
	audioconfig.h \
	audio.cpp \
	audio.h \
	audiosdlmixer.cpp \
	audiosdlmixer.h \
	ejsaudio.cpp

# todo: why do we have to specify SDL_LIBS ? this should be tracked automatically by libtool!
ejsaudio_la_LDFLAGS = -module @MODULE_LDFLAGS@ @SDLMIXER_LIBS@ @SDL_LIBS@

EXTRA_DIST = export.sym
