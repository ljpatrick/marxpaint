# Makefile for magic plugins

# Places to pick up Tux Paint Magic Plugin Dev header and SDL headers
# (can't assume plugin dev stuff has been installed yet, since we're
# part of Tux Paint base, so "install-plugin-dev" probably hasn't
# been run yet; hence "-I../src/" to find 'tp_magic_api.h')

SO_TYPE=so

TP_MAGIC_CFLAGS=$(shell sdl-config --cflags) \
		-I../src/ \
		$(shell tp-magic-config --cflags)
CFLAGS=-g -Wall $(TP_MAGIC_CFLAGS)


all:	negative.$(SO_TYPE) \
	fade_darken.$(SO_TYPE) \
	mirror_flip.$(SO_TYPE) \

clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.$(SO_TYPE)


# Shared objects:
# ---------------

negative.$(SO_TYPE):	src/negative.c
	@echo "Building Negative magic tool"
	@$(CC) $(CFLAGS) -shared -o negative.$(SO_TYPE) src/negative.c

fade_darken.$(SO_TYPE):	src/fade_darken.c
	@echo "Building Fade and Darken magic tools"
	@$(CC) $(CFLAGS) -shared -o fade_darken.$(SO_TYPE) src/fade_darken.c

mirror_flip.$(SO_TYPE):	src/mirror_flip.c
	@echo "Building Mirror and Flip magic tools"
	@$(CC) $(CFLAGS) -shared -o mirror_flip.$(SO_TYPE) src/mirror_flip.c

