# Makefile for magic plugins

# Places to pick up Tux Paint Magic Plugin Dev header and SDL headers
# (can't assume plugin dev stuff has been installed yet, since we're
# part of Tux Paint base, so "install-plugin-dev" probably hasn't
# been run yet; hence "-I../src/" to find 'tp_magic_api.h')

SO_TYPE=so

TP_MAGIC_CFLAGS=$(shell if [ -x tp-magic-config ] ; then tp-magic-config --cflags ; else echo -I../src/ ; fi)

SDL_CFLAGS=$(shell sdl-config --cflags)
CFLAGS=-g -Wall $(SDL_CFLAGS) $(TP_MAGIC_CFLAGS)



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
	sparkles.$(SO_TYPE)

install:
	cd .. ; make install-magic-plugins

clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.$(SO_TYPE)


# Shared objects:
# ---------------

negative.$(SO_TYPE):	src/negative.c
	@echo "Building Negative magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

fade_darken.$(SO_TYPE):	src/fade_darken.c
	@echo "Building Fade and Darken magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $<

mirror_flip.$(SO_TYPE):	src/mirror_flip.c
	@echo "Building Mirror and Flip magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $<

rainbow.$(SO_TYPE):	src/rainbow.c
	@echo "Building Rainbow magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

blocks_chalk_drip.$(SO_TYPE):	src/blocks_chalk_drip.c
	@echo "Building Blocks, Chalk and Drip magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $<

fill.$(SO_TYPE):	src/fill.c
	@echo "Building Fill magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

blur.$(SO_TYPE):	src/blur.c
	@echo "Building Blur magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

grass.$(SO_TYPE):	src/grass.c
	@echo "Building Grass magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

tint.$(SO_TYPE):	src/tint.c
	@echo "Building Tint magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

smudge.$(SO_TYPE):	src/smudge.c
	@echo "Building Smudge magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

cartoon.$(SO_TYPE):	src/cartoon.c
	@echo "Building Smudge magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

bricks.$(SO_TYPE):	src/bricks.c
	@echo "Building Large Bricks and Small Bricks magic tools"
	@$(CC) $(CFLAGS) -shared -o $@ $<

kalidescope.$(SO_TYPE):	src/kalidescope.c
	@echo "Building Kalidescope magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

glasstile.$(SO_TYPE):	src/glasstile.c
	@echo "Building Glass Tile magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

emboss.$(SO_TYPE):	src/emboss.c
	@echo "Building Emboss magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

metalpaint.$(SO_TYPE):	src/metalpaint.c
	@echo "Building Metal Paint magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

waves.$(SO_TYPE):	src/waves.c
	@echo "Building Waves magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

flower.$(SO_TYPE):	src/flower.c
	@echo "Building Flower magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

foam.$(SO_TYPE):	src/foam.c
	@echo "Building Foam magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

ripples.$(SO_TYPE):	src/ripples.c
	@echo "Building Ripples magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

sparkles.$(SO_TYPE):	src/sparkles.c
	@echo "Building Sparkles magic tool"
	@$(CC) $(CFLAGS) -shared -o $@ $<

