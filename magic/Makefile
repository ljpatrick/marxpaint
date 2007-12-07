# Makefile for Tux Paint default 'Magic' tool plugins

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2007-2007 by Bill Kendrick and others
# bill@newbreedsoftware.com
# http://www.tuxpaint.org/

# July 2007 - December 6, 2007


SO_TYPE=so

# Places to pick up Tux Paint Magic Plugin Dev header and SDL headers
# (can't assume plugin dev stuff has been installed yet, since we're
# part of Tux Paint base, so "install-plugin-dev" probably hasn't
# been run yet; hence "-I../src/" to find 'tp_magic_api.h')

TP_MAGIC_CFLAGS=$(shell if [ -x tp-magic-config ] ; then tp-magic-config --cflags ; else echo -I../src/ ; fi)

SDL_CFLAGS=$(shell sdl-config --cflags)
CFLAGS=-g -Wall $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)

SDL_LIBS=-L/usr/local/lib -lmingw32 -lSDL -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)
SDL_MIXER_LIB=-lSDL_mixer
ARCH_LINKS=-lintl -lpng12

all:	negative.$(SO_TYPE) \
	fade_darken.$(SO_TYPE) \
	mirror_flip.$(SO_TYPE) \
	rainbow.$(SO_TYPE) \
	blocks_chalk_drip.$(SO_TYPE) \
	fill.$(SO_TYPE) \
	blur.$(SO_TYPE) \
	grass.$(SO_TYPE) \
	tint.$(SO_TYPE) \
	smudge.$(SO_TYPE) \
	cartoon.$(SO_TYPE) \
	bricks.$(SO_TYPE) \
	kalidescope.$(SO_TYPE) \
	glasstile.$(SO_TYPE) \
	emboss.$(SO_TYPE) \
	metalpaint.$(SO_TYPE) \
	waves.$(SO_TYPE) \
	flower.$(SO_TYPE) \
	foam.$(SO_TYPE) \
	ripples.$(SO_TYPE) \
	light.$(SO_TYPE) \
	shift.$(SO_TYPE) \
	calligraphy.$(SO_TYPE) \
	distortion.$(SO_TYPE)
	@strip *.$(SO_TYPE)

install:
	cd .. ; make install-magic-plugins

clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.$(SO_TYPE)


# beos versions of the targets

beos:
	make \
		SO_TYPE=so \
		PLUGIN_LIBS="$(SDL_LIBS) $(ARCH_LINKS) $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)"


# win32 versions of the targets

cleanwin32:
	make clean \
		SO_TYPE=dll

win32:
	make \
		SO_TYPE=dll \
		PLUGIN_LIBS="$(SDL_LIBS) $(ARCH_LINKS)" \

# Shared objects:
# ---------------

negative.$(SO_TYPE):	src/negative.c
	@echo "Building Negative magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

fade_darken.$(SO_TYPE):	src/fade_darken.c
	@echo "Building Fade and Darken magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

mirror_flip.$(SO_TYPE):	src/mirror_flip.c
	@echo "Building Mirror and Flip magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

rainbow.$(SO_TYPE):	src/rainbow.c
	@echo "Building Rainbow magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

blocks_chalk_drip.$(SO_TYPE):	src/blocks_chalk_drip.c
	@echo "Building Blocks, Chalk and Drip magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

fill.$(SO_TYPE):	src/fill.c
	@echo "Building Fill magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

blur.$(SO_TYPE):	src/blur.c
	@echo "Building Blur magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

grass.$(SO_TYPE):	src/grass.c
	@echo "Building Grass magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

tint.$(SO_TYPE):	src/tint.c
	@echo "Building Tint magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

smudge.$(SO_TYPE):	src/smudge.c
	@echo "Building Smudge magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

cartoon.$(SO_TYPE):	src/cartoon.c
	@echo "Building Cartoon magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

bricks.$(SO_TYPE):	src/bricks.c
	@echo "Building Large Bricks and Small Bricks magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

kalidescope.$(SO_TYPE):	src/kalidescope.c
	@echo "Building Kaleidoscope magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

glasstile.$(SO_TYPE):	src/glasstile.c
	@echo "Building Glass Tile magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

emboss.$(SO_TYPE):	src/emboss.c
	@echo "Building Emboss magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

metalpaint.$(SO_TYPE):	src/metalpaint.c
	@echo "Building Metal Paint magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

waves.$(SO_TYPE):	src/waves.c
	@echo "Building Waves magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

flower.$(SO_TYPE):	src/flower.c
	@echo "Building Flower magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

foam.$(SO_TYPE):	src/foam.c
	@echo "Building Foam magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

ripples.$(SO_TYPE):	src/ripples.c
	@echo "Building Ripples magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

light.$(SO_TYPE):	src/light.c
	@echo "Building Light magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

shift.$(SO_TYPE):	src/shift.c
	@echo "Building Shift magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

calligraphy.$(SO_TYPE):	src/calligraphy.c
	@echo "Building Calligraphy magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

distortion.$(SO_TYPE):	src/distortion.c
	@echo "Building Distortion magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $< $(PLUGIN_LIBS)

