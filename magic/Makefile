# Makefile for magic plugins

SDL_CFLAGS=$(shell sdl-config --cflags)
CFLAGS=-g -Wall $(SDL_CFLAGS) -DDATA_PREFIX=\"$(DATA_PREFIX)\"
#-fPIC ?

all:	negative.so

clean:
	@echo
	@echo "Cleaning up the Magic plug-ins directory ($(PWD))"
	@-rm -f *.so
	@-rm -f obj/*


# Negative tool
# ------------------------------------------------------------------------

negative.so:	obj obj/negative.o obj/magic_helpers.o
	@echo "Linking Negative magic tool"
	@$(CC) -shared -o negative.so obj/negative.o obj/magic_helpers.o

obj/negative.o:	src/negative.c src/magic_helpers.h
	@echo "Compiling Negative magic tool"
	@$(CC) $(CFLAGS) src/negative.c -c -o obj/negative.o



obj/magic_helpers.o:	src/magic_helpers.c src/magic_helpers.h
	@echo "Compiling Magic tool helper routines"
	@$(CC) $(CFLAGS) src/magic_helpers.c -c -o obj/magic_helpers.o

obj:
	@-mkdir obj

