# Makefile for Tux Paint default 'Magic' tool plugins

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2007-2007 by Bill Kendrick and others
# bill@newbreedsoftware.com
# http://www.tuxpaint.org/

# July 2007 - December 29, 2007

ifneq ($(SystemDrive),)
OS:=windows
else
SYSNAME:=$(shell uname -s)
ifeq ($(SYSNAME),Darwin)
OS:=osx
else ifeq ($(SYSNAME),BeOS)
OS:=beos
else ifeq ($(SYSNAME),Haiku)
OS:=beos
else
OS:=linux
endif
endif

windows_SO_TYPE:=dll
osx_SO_TYPE:=bundle
beos_SO_TYPE:=so
linux_SO_TYPE:=so
SO_TYPE:=$($(OS)_SO_TYPE)

SDL_CFLAGS:=$(shell sdl-config --cflags)
SDL_MIXER_LIB:=-lSDL_mixer
ARCH_LINKS:=-lintl -lpng12
SDL_LIBS:=-L/usr/local/lib -lmingw32 -lSDL -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)

windows_PLUGIN_LIBS:="$(SDL_LIBS) $(ARCH_LINKS)"
osx_PLUGIN_LIBS:=
beos_PLUGIN_LIBS:="$(SDL_LIBS) $(ARCH_LINKS) $(SDL_CFLAGS)"
linux_PLUGIN_LIBS:=
PLUGIN_LIBS:=$($(OS)_PLUGIN_LIBS)

#CFLAGS:=-g3 -O2 -fvisibility=hidden -fno-common -W -Wstrict-prototypes -Wmissing-prototypes -Wall $(SDL_CFLAGS) -I../src/
CFLAGS:=-g3 -O2 -fno-common -W -Wstrict-prototypes -Wmissing-prototypes -Wall $(SDL_CFLAGS) -I../src/
SHARED_FLAGS:=-shared -fpic

MAGIC_C:=$(wildcard src/*.c)
MAGIC_SO:=$(patsubst src/%.c,%.$(SO_TYPE),$(MAGIC_C))

.PHONY: buildmagic-all
buildmagic-all: $(MAGIC_SO)

$(MAGIC_SO): %.$(SO_TYPE): src/%.c  
	$(CC) $(CFLAGS) $(SHARED_FLAGS) -o $@ $< $(PLUGIN_LIBS)
# Probably should separate the various flags like the following, or at least
# split plug-in CFLAGS from normal CFLAGS.
#	$(CC) $(PLUG_CPPFLAGS) $(PLUG_CFLAGS) $(PLUG_LDFLAGS) -o $@ $< $(PLUG_LIBS)

.PHONY: buildmagic-clean
buildmagic-clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.$(SO_TYPE)
