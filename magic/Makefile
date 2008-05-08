# Makefile for Tux Paint default 'Magic' tool plugins

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2007-2007 by Bill Kendrick and others
# bill@newbreedsoftware.com
# http://www.tuxpaint.org/

# July 2007 - December 29, 2007


SO_TYPE:=so

# Places to pick up Tux Paint Magic Plugin Dev header and SDL headers
# (can't assume plugin dev stuff has been installed yet, since we're
# part of Tux Paint base, so "install-plugin-dev" probably hasn't
# been run yet; hence "-I../src/" to find 'tp_magic_api.h')

TP_MAGIC_CFLAGS:=$(shell if [ -x tp-magic-config ] ; then tp-magic-config --cflags ; else echo -I../src/ ; fi)

SDL_CFLAGS:=$(shell sdl-config --cflags)
#CFLAGS:=-g3 -O2 -fvisibility=hidden -fno-common -W -Wstrict-prototypes -Wmissing-prototypes -Wall $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)
CFLAGS:=-g3 -O2 -fno-common -W -Wstrict-prototypes -Wmissing-prototypes -Wall $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)
SHARED_FLAGS:=-shared -fpic

SDL_MIXER_LIB:=-lSDL_mixer
ARCH_LINKS:=-lintl -lpng12
SDL_LIBS:=-L/usr/local/lib -lmingw32 -lSDL -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)


MAGIC_C:=$(wildcard src/*.c)
MAGIC_SO:=$(patsubst src/%.c,%.$(SO_TYPE),$(MAGIC_C))

.PHONY: all
all: $(MAGIC_SO)

$(MAGIC_SO): %.$(SO_TYPE): src/%.c  
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $@ $< $(PLUGIN_LIBS)
# Probably should separate the various flags like the following, or at least
# split plug-in CFLAGS from normal CFLAGS.
#	$(CC) $(PLUG_CPPFLAGS) $(PLUG_CFLAGS) $(PLUG_LDFLAGS) -o $@ $< $(PLUG_LIBS)


.PHONY: install
install:
	cd .. ; make install-magic-plugins

.PHONY: clean
clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.$(SO_TYPE)


# beos versions of the targets
.PHONY: beos
beos:
	make \
		SO_TYPE:=so \
		PLUGIN_LIBS:="$(SDL_LIBS) $(ARCH_LINKS) $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)"


# win32 versions of the targets
.PHONY: cleanwin32
cleanwin32:
	make clean \
		SO_TYPE:=dll

.PHONY: win32
win32:
	make \
		SO_TYPE:=dll \
		PLUGIN_LIBS:="$(SDL_LIBS) $(ARCH_LINKS)" \

