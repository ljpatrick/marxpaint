# Makefile for tuxpaint
# $Id$

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2002-2006 by Bill Kendrick and others
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/tuxpaint/

# June 14, 2002 - September 22, 2006


# The version number, for release:

VER_VERSION=0.9.16rc2
VER_DATE=`date +"%Y-%m-%d"`


# Where to install things:

PREFIX=/usr/local


# Root directory to place files when creating packages.

PKG_ROOT=


# Program:

BIN_PREFIX=$(PKG_ROOT)$(PREFIX)/bin
EXE_EXT=


# Data:

DATA_PREFIX=$(PKG_ROOT)$(PREFIX)/share/tuxpaint


# Docs and man page:

DOC_PREFIX=$(PKG_ROOT)$(PREFIX)/share/doc/tuxpaint
MAN_PREFIX=$(PKG_ROOT)$(PREFIX)/share/man


# 'System-wide' Config file:

ifeq ($(PREFIX),/usr)
  CONFDIR=$(PKG_ROOT)/etc/tuxpaint
else
  CONFDIR=$(PKG_ROOT)$(PREFIX)/etc/tuxpaint
endif


# Commands useful to other arch's (e.g., BeOS)

RSRC_CMD=echo -n
MIMESET_CMD=echo -n


# Icons and launchers:

ICON_PREFIX=$(PKG_ROOT)$(PREFIX)/share/pixmaps
X11_ICON_PREFIX=$(PKG_ROOT)$(PREFIX)/X11R6/include/X11/pixmaps
GNOME_PREFIX=`gnome-config --prefix 2> /dev/null`
KDE_PREFIX=`kde-config --install apps --expandvars 2> /dev/null`
KDE_ICON_PREFIX=`kde-config --install icon --expandvars 2> /dev/null`


# Locale files

LOCALE_PREFIX=$(PKG_ROOT)$(PREFIX)/share/locale
# LOCALE_PREFIX=/usr/share/locale


# Built with sound by default  (override with "make nosound")

NOSOUNDFLAG=__SOUND


# Where to find cursor shape XBMs

MOUSEDIR=mouse
CURSOR_SHAPES=LARGE

# MOUSEDIR=mouse/16x16
# CURSOR_SHAPES=SMALL


# Libraries, paths, and flags:

SDL_LIBS=$(shell sdl-config --libs) -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)
SDL_MIXER_LIB=-lSDL_mixer
SDL_CFLAGS=$(shell sdl-config --cflags)


# The entire set of CFLAGS:

#-ffast-math
CFLAGS=-O2 -W -Wall -fno-common -ffloat-store \
	-Wcast-align -Wredundant-decls \
	-Wbad-function-cast -Wwrite-strings \
	-Waggregate-return \
	-Wstrict-prototypes -Wmissing-prototypes \
	`src/test-option.sh -Wdeclaration-after-statement`

DEFS=-DDATA_PREFIX=\"$(DATA_PREFIX)/\" \
	-D$(NOSOUNDFLAG) -DDOC_PREFIX=\"$(DOC_PREFIX)/\" \
	-DLOCALEDIR=\"$(LOCALE_PREFIX)/\" -DCONFDIR=\"$(CONFDIR)/\" \
	-DVER_VERSION=\"$(VER_VERSION)\" \
	-DVER_DATE=\"$(VER_DATE)\"

DEBUG_FLAGS=
#DEBUG_FLAGS=-g

MOUSE_CFLAGS=-Isrc/$(MOUSEDIR) -D$(CURSOR_SHAPES)_CURSOR_SHAPES


# "make" with no arguments builds the program and man page from sources:

all:	tuxpaint translations
	@echo
	@echo "--------------------------------------------------------------"
	@echo
	@echo "Done compiling."
	@echo
	@echo "Now run 'make install' with any options you ran 'make' with."
	@echo "to install Tux Paint."
	@echo
	@echo "You may need superuser ('root') priveleges, depending on"
	@echo "where you're installing."
	@echo "(Depending on your system, you either need to 'su' first,"
	@echo "or run 'sudo make install'.)"
	@echo

releaseclean:
	@echo
	@echo "Cleaning release directory"
	@echo
	@rm -rf "build/tuxpaint-$(VER_VERSION)" "build/tuxpaint-$(VER_VERSION).tar.gz"
	@-if [ -d build ] ; then rmdir build ; fi

releasedir: build/tuxpaint-$(VER_VERSION)

build/tuxpaint-$(VER_VERSION):
	@echo
	@echo "Creating release directory"
	@echo
	@mkdir -p build/tuxpaint-$(VER_VERSION)
	@find . -follow \
	     \( -wholename '*/CVS' -o -name .cvsignore -o -name 'build' -o -name '.#*' \) \
	     -prune -o -type f -exec cp --parents -vdp \{\} build/tuxpaint-$(VER_VERSION)/ \;

release: releasedir
	@echo
	@echo "Creating release tarball"
	@echo
	@cd build ; \
	    tar -czvf tuxpaint-$(VER_VERSION).tar.gz tuxpaint-$(VER_VERSION)


# "make nosound" builds the program with sound disabled, and man page,
# from sources:

nosound:
	@echo
	@echo "Building with sound DISABLED"
	@echo
	make SDL_MIXER_LIB= NOSOUNDFLAG=NOSOUND


# "make beos" builds the program for BeOS

beos:
	make \
		PREFIX=/boot/develop/tools/gnupro \
		BIN_PREFIX=./ \
		DATA_PREFIX=./data \
		DOC_PREFIX=./docs \
		MAN_PREFIX=./src \
		CONFDIR=./src/ \
		ICON_PREFIX=. \
		X11_ICON_PREFIX=. \
		LOCALE_PREFIX=/boot/home/config/share/locale \
		CFLAGS="-O1 -funroll-loops -fomit-frame-pointer -pipe -Wall" \
		RSRC_CMD="xres -o tuxpaint tuxpaint.rsrc" \
		MIMESET_CMD="mimeset -f tuxpaint" \
		ARCH_LINKS="-lintl -lpng -lz -lbe" \
		ARCH_HEADERS="src/BeOS_Print.h" \
		ARCH_LIBS="obj/BeOS_print.o"

# "make win32" builds the program for Windows using MinGW/MSYS.
# The DATA_, DOC_ and LOCALE_ prefixes are absolute paths.
# Suitable for development/testing in the MinGW/MSYS environment.
win32:
	make \
		PREFIX=/usr/local \
		BIN_PREFIX=$(PREFIX)/bin \
		EXE_EXT=.exe \
		DATA_PREFIX=$(PREFIX)/share/tuxpaint \
		DOC_PREFIX=$(PREFIX)/share/doc/tuxpaint \
		MAN_PREFIX=$(PREFIX)/share/man \
		ICON_PREFIX=. \
		X11_ICON_PREFIX=. \
		LOCALE_PREFIX=$(PREFIX)/share/locale \
		CONFDIR=$(PREFIX)/etc/tuxpaint \
		ARCH_LINKS="-lintl-3 -lpng12 -lwinspool -lshlwapi" \
		ARCH_HEADERS="src/win32_print.h" \
		ARCH_LIBS="obj/win32_print.o obj/resource.o"

# "make nokia770" builds the program for the Nokia 770.

nokia770:
	make \
		DATA_PREFIX=/var/lib/install/usr/share/tuxpaint \
		CFLAGS="-DNOKIA_770"

# "make install" installs all of the various parts
# (depending on the *PREFIX variables at the top, you probably need
# to do this as superuser ("root"))

install:	install-bin install-data install-man install-doc \
		install-icon install-gettext install-importscript \
		install-default-config install-example-stamps \
		install-example-starters \
		install-gnome install-kde install-kde-icons
	@echo
	@echo "--------------------------------------------------------------"
	@echo
	@echo "All done! Now (preferably NOT as 'root' superuser),"
	@echo "you can type the command 'tuxpaint' to run the program!!!"
	@echo
	@echo "For more information, see the 'tuxpaint' man page,"
	@echo "run 'tuxpaint --usage' or see $(DOC_PREFIX)/README.txt"
	@echo
	@echo "Visit Tux Paint's home page for more information, updates"
	@echo "and to learn how you can help out!"
	@echo
	@echo "  http://www.newbreedsoftware.com/tuxpaint/"
	@echo
	@echo "Enjoy!"
	@echo


# Installs the various parts for the MinGW/MSYS development/testing environment.

install-private-win32:	install-bin install-data install-man install-doc \
		install-gettext install-importscript \
		install-default-config install-example-stamps \
		install-example-starters
	@echo
	@echo "--------------------------------------------------------------"
	@echo
	@echo "All done!"
	@echo "Now you can type the command 'tuxpaint' to run the program!!!"
	@echo
	@echo "For more information, see the 'tuxpaint' man page,"
	@echo "run 'tuxpaint --usage' or see $(DOC_PREFIX)/README.txt"
	@echo
	@echo "Visit Tux Paint's home page for more information, updates"
	@echo "and to learn how you can help out!"
	@echo
	@echo "  http://www.newbreedsoftware.com/tuxpaint/"
	@echo
	@echo "Enjoy!"
	@echo


# Installs the various parts for the MinGW/MSYS development/testing environment.

bdist-private-win32:	install-bin install-data install-doc \
		install-gettext install-dlls\
		install-example-stamps install-example-starters
	@echo
	@echo "--------------------------------------------------------------"
	@echo
	@echo "A binary distribution suitable for running on Windows systems"
	@echo "other than this one is now available in $(PREFIX)."
	@echo
	@echo "Try compiling the InnoSetup installer script 'tuxpaint.iss'."
	@echo


# "make install-beos" installs Tux Paint, but using BeOS settings

install-beos:
	make install \
		PREFIX=/boot/develop/tools/gnupro \
		BIN_PREFIX=./ \
		DATA_PREFIX=./data \
		DOC_PREFIX=./docs \
		MAN_PREFIX=./src \
		CONFDIR=./src/ \
		ICON_PREFIX=. \
		X11_ICON_PREFIX=. \
		LOCALE_PREFIX=/boot/home/config/share/locale \
		CFLAGS="-O1 -funroll-loops -fomit-frame-pointer -pipe -Wall" \
		RSRC_CMD="xres -o tuxpaint tuxpaint.rsrc" \
		MIMESET_CMD="mimeset -f tuxpaint" \
		ARCH_LINKS="-lintl -lpng -lz -lbe" \
		ARCH_HEADERS="src/BeOS_Print.h" \
		ARCH_LIBS="obj/BeOS_print.o"

# "make install-win32" installs Tux Paint, but using MinGW/MSYS settings
# Suitable for development/testing in the MinGW/MSYS environment.
install-win32:
	strip -s tuxpaint.exe
	make install-private-win32 \
		PREFIX=/usr/local \
		BIN_PREFIX=$(PREFIX)/bin \
		EXE_EXT=.exe \
		DATA_PREFIX=$(PREFIX)/share/tuxpaint \
		DOC_PREFIX=$(PREFIX)/share/doc/tuxpaint \
		MAN_PREFIX=$(PREFIX)/share/man \
		ICON_PREFIX=. \
		X11_ICON_PREFIX=. \
		LOCALE_PREFIX=$(PREFIX)/share/locale \
		CONFDIR=$(PREFIX)/etc/tuxpaint \

# "make bdist-win32" recompiles Tux Paint to work with executable-relative 
# data, docs and locale directories. Also copies all files, including DLLs,
# into a 'bdist' output directory ready for processing by an installer script.
bdist-win32:
	@-rm -f tuxpaint.exe
	@-rm -f obj/*.o
	make \
		PREFIX=./visualc/bdist \
		BIN_PREFIX=$(PREFIX)/bin \
		EXE_EXT=.exe \
		DATA_PREFIX=data \
		DOC_PREFIX=docs \
		LOCALE_PREFIX=locale \
		ARCH_LINKS="-lintl-3 -lpng12 -lwinspool -lshlwapi" \
		ARCH_HEADERS="src/win32_print.h" \
		ARCH_LIBS="obj/win32_print.o obj/resource.o"
	strip -s tuxpaint.exe
	make bdist-private-win32 \
		PREFIX=./visualc/bdist \
		BIN_PREFIX=./visualc/bdist \
		EXE_EXT=.exe \
		DATA_PREFIX=./visualc/bdist/data \
		DOC_PREFIX=./visualc/bdist/docs \
		LOCALE_PREFIX=./visualc/bdist/locale \

# "make bdist-clean" deletes the 'bdist' directory
bdist-clean:
	@echo
	@echo "Cleaning up the 'bdist' directory! ($(PWD))"
	@-rm -rf ./visualc/bdist
	@echo

# "make clean" deletes the program, the compiled objects and the
# built man page (returns to factory archive, pretty much...)

clean:
	@echo
	@echo "Cleaning up the build directory! ($(PWD))"
	@-rm -f tuxpaint
	@-rm -f obj/*.o
	@#if [ -d obj ]; then rmdir obj; fi
	@-rm -f trans/*.mo
	@if [ -d trans ]; then rmdir trans; fi
	@echo


# "make uninstall" should remove the various parts from their
# installation locations.  BE SURE the *PREFIX variables at the top
# are the same as they were when you installed, of course!!!

uninstall:
	-if [ "x$(GNOME_PREFIX)" != "x" ]; then \
	  rm $(GNOME_PREFIX)/share/gnome/apps/Graphics/tuxpaint.desktop; \
	  rm $(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	else \
	  rm /usr/share/gnome/apps/Graphics/tuxpaint.desktop; \
	  rm /usr/share/pixmaps/tuxpaint.png; \
	fi
	-if [ "x$(KDE_PREFIX)" != "x" ]; then \
	  rm $(KDE_PREFIX)/Graphics/tuxpaint.desktop; \
	fi
	-rm $(ICON_PREFIX)/tuxpaint.png
	-rm $(X11_ICON_PREFIX)/tuxpaint.xpm
	-if [ "x$(KDE_ICON_PREFIX)" != "x" ]; then \
	  rm $(KDE_ICON_PREFIX)/hicolor/scalable/apps/tuxpaint.svg; \
	  rm $(KDE_ICON_PREFIX)/hicolor/192x192/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/128x128/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/96x96/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/64x64/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/22x22/apps/tuxpaint.png; \
	  rm $(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
	fi
	-rm $(BIN_PREFIX)/tuxpaint
	-rm $(BIN_PREFIX)/tuxpaint-import
	-rm -r $(DATA_PREFIX)
	-rm -r $(DOC_PREFIX)
	-rm $(MAN_PREFIX)/man1/tuxpaint.1.gz
	-rm $(MAN_PREFIX)/pl/man1/tuxpaint.1.gz
	-rm $(MAN_PREFIX)/man1/tuxpaint-import.1.gz
	-rm $(LOCALE_PREFIX)/af/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ar/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/be/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/bg/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/bo/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/br/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ca/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/cs/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/cy/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/da/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/de/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/et/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/el/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/en_GB/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/es/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/es_MX/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/eu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/fi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/fo/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/fr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ga/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/gd/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/gl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/gos/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/gu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/he/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/hi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/hr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/hu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/tlh/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/id/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/is/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/it/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ja/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ka/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ko/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ku/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/lt/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ms/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/nl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/nb/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/nn/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/pl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/pt_PT/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/pt_BR/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ro/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ru/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/rw/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sk/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sq/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sv/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/sw/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ta/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/th/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/tr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/uk/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/ve/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/vi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/wa/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/xh/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/zh_CN/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)/zh_TW/LC_MESSAGES/tuxpaint.mo
	-rm -f -r $(CONFDIR)


# Install default config file:

install-default-config:
	@echo
	@echo "...Installing default config file..."
	@install -d $(CONFDIR)
	@cp src/tuxpaint.conf $(CONFDIR)
	@chmod 644 $(CONFDIR)/tuxpaint.conf


# Install example stamps

install-example-stamps:
	@echo
	@echo "...Installing example stamps..."
	@install -d $(DATA_PREFIX)/stamps
	@cp -R stamps/* $(DATA_PREFIX)/stamps
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)/stamps


# Install example starters

install-example-starters:
	@echo
	@echo "...Installing example starter images..."
	@install -d $(DATA_PREFIX)/starters
	@cp -R starters/* $(DATA_PREFIX)/starters
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)/starters


# Install a launcher icon in the Gnome menu, under "Graphics"

install-gnome:
	@echo
	@echo "...Installing launcher icon into GNOME..."
	@if [ "x$(GNOME_PREFIX)" != "x" ]; then \
	 install -d $(PKG_ROOT)$(GNOME_PREFIX)/share/pixmaps; \
	 cp data/images/icon.png $(PKG_ROOT)/$(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 chmod 644 $(PKG_ROOT)$(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 install -d $(PKG_ROOT)$(GNOME_PREFIX)/share/gnome/apps/Graphics; \
	 cp src/tuxpaint.desktop $(PKG_ROOT)$(GNOME_PREFIX)/share/gnome/apps/Graphics/; \
	 chmod 644 $(PKG_ROOT)$(GNOME_PREFIX)/share/gnome/apps/Graphics/tuxpaint.desktop; \
	fi


# Install a launcher icon for the Nokia 770.

install-nokia770:
	@echo
	@echo "...Installing launcher icon into the Nokia 770..."
	@if [ "x$(NOKIA770_PREFIX)" != "x" ]; then \
	 install -d $(PKG_ROOT)$(NOKIA770_PREFIX)/share/pixmaps; \
	 cp data/images/icon.png $(PKG_ROOT)/$(NOKIA770_PREFIX)/share/pixmaps/tuxpaint.png; \
	 chmod 644 $(PKG_ROOT)$(NOKIA770_PREFIX)/share/pixmaps/tuxpaint.png; \
	 install -d $(PKG_ROOT)$(NOKIA770_PREFIX)/share/applications/hildon; \
	 cp hildon/tuxpaint.desktop $(PKG_ROOT)$(NOKIA770_PREFIX)/share/applications/hildon/; \
	 chmod 644 $(PKG_ROOT)$(NOKIA770_PREFIX)/share/applications/hildon/tuxpaint.desktop; \
	fi


# Install a launcher icon in the KDE menu...

install-kde:
	@echo
	@echo "...Installing launcher icon into KDE..."
	@if [ "x$(KDE_PREFIX)" != "x" ]; then \
	  install -d $(PKG_ROOT)$(KDE_PREFIX)/Graphics; \
	  cp src/tuxpaint.desktop $(PKG_ROOT)$(KDE_PREFIX)/Graphics/; \
	  chmod 644 $(PKG_ROOT)$(KDE_PREFIX)/Graphics/tuxpaint.desktop; \
	fi


install-kde-icons:
	@echo "...Installing launcher icon graphics into KDE..."
	@if [ "x$(KDE_ICON_PREFIX)" != "x" ]; then \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/scalable/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/192x192/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/128x128/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/96x96/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/64x64/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/48x48/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/32x32/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/22x22/apps/; \
	  install -d $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/16x16/apps/; \
	  cp data/images/tuxpaint-icon.svg \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/scalable/apps/tuxpaint.svg; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/scalable/apps/tuxpaint.svg; \
	  cp data/images/icon192x192.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/192x192/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/192x192/apps/tuxpaint.png; \
	  cp data/images/icon128x128.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/128x128/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/128x128/apps/tuxpaint.png; \
	  cp data/images/icon96x96.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/96x96/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/96x96/apps/tuxpaint.png; \
	  cp data/images/icon64x64.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/64x64/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/64x64/apps/tuxpaint.png; \
	  cp data/images/icon48x48.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
	  cp data/images/icon32x32.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
	  cp data/images/icon22x22.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/22x22/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/22x22/apps/tuxpaint.png; \
	  cp data/images/icon16x16.png \
		$(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)$(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
	fi


# Install the PNG icon (for GNOME, KDE, etc.)
# and the 24-color 32x32 XPM (for other Window managers):

# FIXME: Should this also use $(PKG_ROOT)?

install-icon:
	@echo
	@echo "...Installing launcher icon graphics..."
	@install -d $(ICON_PREFIX)
	@cp data/images/icon.png $(ICON_PREFIX)/tuxpaint.png
	@chmod 644 $(ICON_PREFIX)/tuxpaint.png
	@install -d $(X11_ICON_PREFIX)
	@cp data/images/icon32x32.xpm $(X11_ICON_PREFIX)/tuxpaint.xpm
	@chmod 644 $(X11_ICON_PREFIX)/tuxpaint.xpm


# Install the program:

install-bin:
	@echo
	@echo "...Installing program itself..."
	@install -d $(BIN_PREFIX)
	@cp tuxpaint $(BIN_PREFIX)
	@chmod a+rx,g-w,o-w $(BIN_PREFIX)/tuxpaint$(EXE_EXT)

# Install the required Windows DLLs into the 'bdist' directory

install-dlls:
	@echo
	@echo "...Installing Windows DLLs..."
	@install -d $(BIN_PREFIX)
	@cp `which libintl-3.dll` $(BIN_PREFIX)
	@cp `which libiconv-2.dll` $(BIN_PREFIX)
	@cp `which libpng12.dll` $(BIN_PREFIX)
	@cp `which SDL.dll` $(BIN_PREFIX)
	@cp `which SDL_image.dll` $(BIN_PREFIX)
	@cp `which SDL_mixer.dll` $(BIN_PREFIX)
	@cp `which SDL_ttf.dll` $(BIN_PREFIX)
	@cp `which zlib1.dll` $(BIN_PREFIX)
	@cp `which libogg-0.dll` $(BIN_PREFIX)
	@cp `which libvorbis-0.dll` $(BIN_PREFIX)
	@cp `which libvorbisfile-3.dll` $(BIN_PREFIX)
	
# Install the import script:

install-importscript:
	@echo
	@echo "...Installing 'tuxpaint-import' script..."
	@cp src/tuxpaint-import.sh $(BIN_PREFIX)/tuxpaint-import
	@chmod a+rx,g-w,o-w $(BIN_PREFIX)/tuxpaint-import


# Install the data (sound, graphics, fonts):

install-data:
	@echo
	@echo "...Installing data files..."
	@install -d $(DATA_PREFIX)
	@cp -R data/* $(DATA_PREFIX)
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)
	@echo
	@echo "...Installing fonts..."
	@install -d $(DATA_PREFIX)/fonts/locale
	@cp -R fonts/locale/* $(DATA_PREFIX)/fonts/locale
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)/fonts/locale

# Install the translated text:

install-gettext:
	@echo
	@echo "...Installing translation files..."
	@#
	@echo "   af_ZA ...Afrikaans..."
	@install -d $(LOCALE_PREFIX)/af/LC_MESSAGES
	@cp trans/af.mo $(LOCALE_PREFIX)/af/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/af/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sq_AL ...Albanian..."
	@install -d $(LOCALE_PREFIX)/sq/LC_MESSAGES
	@cp trans/sq.mo $(LOCALE_PREFIX)/sq/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sq/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ar_SA ...Arabic..."
	@install -d $(LOCALE_PREFIX)/ar/LC_MESSAGES
	@cp trans/ar.mo $(LOCALE_PREFIX)/ar/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ar/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   be_BY ...Belarusian..."
	@install -d $(LOCALE_PREFIX)/be/LC_MESSAGES
	@cp trans/be.mo $(LOCALE_PREFIX)/be/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/be/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pt_BR ...Brazilian Portuguese..."
	@install -d $(LOCALE_PREFIX)/pt_BR/LC_MESSAGES
	@cp trans/pt_br.mo $(LOCALE_PREFIX)/pt_BR/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/pt_BR/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   br_FR ...Breton..."
	@install -d $(LOCALE_PREFIX)/br/LC_MESSAGES
	@cp trans/br.mo $(LOCALE_PREFIX)/br/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/br/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   eu_ES ...Basque..."
	@install -d $(LOCALE_PREFIX)/eu/LC_MESSAGES
	@cp trans/eu.mo $(LOCALE_PREFIX)/eu/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/eu/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   en_GB ...British English..."
	@install -d $(LOCALE_PREFIX)/en_GB/LC_MESSAGES
	@cp trans/en_gb.mo $(LOCALE_PREFIX)/en_GB/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/en_GB/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   bg_BG ...Bulgarian..."
	@install -d $(LOCALE_PREFIX)/bg/LC_MESSAGES
	@cp trans/bg.mo $(LOCALE_PREFIX)/bg/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/bg/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ca_ES ...Catalan..."
	@install -d $(LOCALE_PREFIX)/ca/LC_MESSAGES
	@cp trans/ca.mo $(LOCALE_PREFIX)/ca/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ca/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   zh_CN ...Chinese (Simplified)..."
	@install -d $(LOCALE_PREFIX)/zh_CN/LC_MESSAGES
	@cp trans/zh_cn.mo $(LOCALE_PREFIX)/zh_CN/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/zh_CN/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   zh_TW ...Chinese (Traditional)..."
	@install -d $(LOCALE_PREFIX)/zh_TW/LC_MESSAGES
	@cp trans/zh_tw.mo $(LOCALE_PREFIX)/zh_TW/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/zh_TW/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hr_HR ...Croatian..."
	@install -d $(LOCALE_PREFIX)/hr/LC_MESSAGES
	@cp trans/hr.mo $(LOCALE_PREFIX)/hr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/hr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   cs_CZ ...Czech..."
	@install -d $(LOCALE_PREFIX)/cs/LC_MESSAGES
	@cp trans/cs.mo $(LOCALE_PREFIX)/cs/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/cs/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   da_DK ...Danish..."
	@install -d $(LOCALE_PREFIX)/da/LC_MESSAGES
	@cp trans/da.mo $(LOCALE_PREFIX)/da/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/da/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nl_NL ...Dutch..."
	@install -d $(LOCALE_PREFIX)/nl/LC_MESSAGES
	@cp trans/nl.mo $(LOCALE_PREFIX)/nl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/nl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   et_EE ...Estonian..."
	@install -d $(LOCALE_PREFIX)/et/LC_MESSAGES
	@cp trans/et.mo $(LOCALE_PREFIX)/et/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/et/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   fo_FO ...Faroese..."
	@install -d $(LOCALE_PREFIX)/fo/LC_MESSAGES
	@cp trans/fo.mo $(LOCALE_PREFIX)/fo/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/fo/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   fi_FI ...Finnish..."
	@install -d $(LOCALE_PREFIX)/fi/LC_MESSAGES
	@cp trans/fi.mo $(LOCALE_PREFIX)/fi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/fi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   fr_FR ...French..."
	@install -d $(LOCALE_PREFIX)/fr/LC_MESSAGES
	@cp trans/fr.mo $(LOCALE_PREFIX)/fr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/fr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ga_IE ...Irish Gaelic..."
	@install -d $(LOCALE_PREFIX)/ga/LC_MESSAGES
	@cp trans/ga.mo $(LOCALE_PREFIX)/ga/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ga/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   gl_ES ...Galician..."
	@install -d $(LOCALE_PREFIX)/gl/LC_MESSAGES
	@cp trans/gl.mo $(LOCALE_PREFIX)/gl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/gl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ka_GE ...Georgian..."
	@install -d $(LOCALE_PREFIX)/ka/LC_MESSAGES
	@cp trans/ka.mo $(LOCALE_PREFIX)/ka/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ka/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   de_DE ...German..."
	@install -d $(LOCALE_PREFIX)/de/LC_MESSAGES
	@cp trans/de.mo $(LOCALE_PREFIX)/de/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/de/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   el_GR ...Greek..."
	@install -d $(LOCALE_PREFIX)/el/LC_MESSAGES
	@cp trans/el.mo $(LOCALE_PREFIX)/el/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/el/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   gos_NL ...Gronings..."
	@install -d $(LOCALE_PREFIX)/gos/LC_MESSAGES
	@cp trans/gos.mo $(LOCALE_PREFIX)/gos/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/gos/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   gu_IN ...Gujarati..."
	@install -d $(LOCALE_PREFIX)/gu/LC_MESSAGES
	@cp trans/gu.mo $(LOCALE_PREFIX)/gu/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/gu/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   he_IL ...Hebrew..."
	@install -d $(LOCALE_PREFIX)/he/LC_MESSAGES
	@cp trans/he.mo $(LOCALE_PREFIX)/he/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/he/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hi_IN ...Hindi..."
	@install -d $(LOCALE_PREFIX)/hi/LC_MESSAGES
	@cp trans/hi.mo $(LOCALE_PREFIX)/hi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/hi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hu_HU ...Hungarian..."
	@install -d $(LOCALE_PREFIX)/hu/LC_MESSAGES
	@cp trans/hu.mo $(LOCALE_PREFIX)/hu/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/hu/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   is_IS ...Icelandic..."
	@install -d $(LOCALE_PREFIX)/is/LC_MESSAGES
	@cp trans/is.mo $(LOCALE_PREFIX)/is/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/is/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   id_ID ...Indonesian..."
	@install -d $(LOCALE_PREFIX)/id/LC_MESSAGES
	@cp trans/id.mo $(LOCALE_PREFIX)/id/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/id/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   it_IT ...Italian..."
	@install -d $(LOCALE_PREFIX)/it/LC_MESSAGES
	@cp trans/it.mo $(LOCALE_PREFIX)/it/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/it/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ja_JP ...Japanese..."
	@install -d $(LOCALE_PREFIX)/ja/LC_MESSAGES
	@cp trans/ja.mo $(LOCALE_PREFIX)/ja/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ja/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   tlh   ...Klingon (Romanized)..."
	@install -d $(LOCALE_PREFIX)/tlh/LC_MESSAGES
	@cp trans/tlh.mo $(LOCALE_PREFIX)/tlh/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/tlh/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   rw_RW ...Kinyarwanda..."
	@install -d $(LOCALE_PREFIX)/rw/LC_MESSAGES
	@cp trans/rw.mo $(LOCALE_PREFIX)/rw/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/rw/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ko_KR ...Korean..."
	@install -d $(LOCALE_PREFIX)/ko/LC_MESSAGES
	@cp trans/ko.mo $(LOCALE_PREFIX)/ko/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ko/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ku_TR ...Kurdish..."
	@install -d $(LOCALE_PREFIX)/ku/LC_MESSAGES
	@cp trans/ku.mo $(LOCALE_PREFIX)/ku/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ko/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   lt_LT ...Lithuanian..."
	@install -d $(LOCALE_PREFIX)/lt/LC_MESSAGES
	@cp trans/lt.mo $(LOCALE_PREFIX)/lt/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/lt/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ms_MY ...Malay..."
	@install -d $(LOCALE_PREFIX)/ms/LC_MESSAGES
	@cp trans/ms.mo $(LOCALE_PREFIX)/ms/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ms/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nb_NO ...Norwegian Bokmal..."
	@install -d $(LOCALE_PREFIX)/nb/LC_MESSAGES
	@cp trans/nb.mo $(LOCALE_PREFIX)/nb/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/nb/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nn_NO ...Norwegian Nynorsk..."
	@install -d $(LOCALE_PREFIX)/nn/LC_MESSAGES
	@cp trans/nn.mo $(LOCALE_PREFIX)/nn/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/nn/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pl_PL ...Polish..."
	@install -d $(LOCALE_PREFIX)/pl/LC_MESSAGES
	@cp trans/pl.mo $(LOCALE_PREFIX)/pl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/pl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pt_PT ...Portuguese (Portugal)..."
	@install -d $(LOCALE_PREFIX)/pt_PT/LC_MESSAGES
	@cp trans/pt_pt.mo $(LOCALE_PREFIX)/pt_PT/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/pt_PT/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ro_RO ...Romanian..."
	@install -d $(LOCALE_PREFIX)/ro/LC_MESSAGES
	@cp trans/ro.mo $(LOCALE_PREFIX)/ro/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ro/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ru_RU ...Russian..."
	@install -d $(LOCALE_PREFIX)/ru/LC_MESSAGES
	@cp trans/ru.mo $(LOCALE_PREFIX)/ru/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ru/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   gd_GB ...Scottish Gaelic..."
	@install -d $(LOCALE_PREFIX)/gd/LC_MESSAGES
	@cp trans/gd.mo $(LOCALE_PREFIX)/gd/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/gd/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sr_YU ...Serbian..."
	@install -d $(LOCALE_PREFIX)/sr/LC_MESSAGES
	@cp trans/sr.mo $(LOCALE_PREFIX)/sr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sk_SK ...Slovak..."
	@install -d $(LOCALE_PREFIX)/sk/LC_MESSAGES
	@cp trans/sk.mo $(LOCALE_PREFIX)/sk/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sk/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sl_SI ...Slovenian..."
	@install -d $(LOCALE_PREFIX)/sl/LC_MESSAGES
	@cp trans/sl.mo $(LOCALE_PREFIX)/sl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   es_ES ...Spanish..."
	@install -d $(LOCALE_PREFIX)/es/LC_MESSAGES
	@cp trans/es.mo $(LOCALE_PREFIX)/es/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/es/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   es_MX ...Mexican Spanish..."
	@install -d $(LOCALE_PREFIX)/es_MX/LC_MESSAGES
	@cp trans/es_mx.mo $(LOCALE_PREFIX)/es_MX/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/es_MX/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sw_TZ ...Swahili..."
	@install -d $(LOCALE_PREFIX)/sw/LC_MESSAGES
	@cp trans/sw.mo $(LOCALE_PREFIX)/sw/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sw/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sv_SE ...Swedish..."
	@install -d $(LOCALE_PREFIX)/sv/LC_MESSAGES
	@cp trans/sv.mo $(LOCALE_PREFIX)/sv/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/sv/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   tl_PH ...Tagalog..."
	@install -d $(LOCALE_PREFIX)/tl/LC_MESSAGES
	@cp trans/tl.mo $(LOCALE_PREFIX)/tl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/tl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ta_IN ...Tamil..."
	@install -d $(LOCALE_PREFIX)/ta/LC_MESSAGES
	@cp trans/ta.mo $(LOCALE_PREFIX)/ta/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ta/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   th_TH ...Thai..."
	@install -d $(LOCALE_PREFIX)/th/LC_MESSAGES
	@cp trans/th.mo $(LOCALE_PREFIX)/th/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/th/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   bo_CN ...Tibetan..."
	@install -d $(LOCALE_PREFIX)/bo/LC_MESSAGES
	@cp trans/bo.mo $(LOCALE_PREFIX)/bo/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/bo/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   tr_TR ...Turkish..."
	@install -d $(LOCALE_PREFIX)/tr/LC_MESSAGES
	@cp trans/tr.mo $(LOCALE_PREFIX)/tr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/tr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   uk_UA ...Ukrainian..."
	@install -d $(LOCALE_PREFIX)/uk/LC_MESSAGES
	@cp trans/uk.mo $(LOCALE_PREFIX)/uk/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/uk/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ve_ZA ...Venda..."
	@install -d $(LOCALE_PREFIX)/ve/LC_MESSAGES
	@cp trans/ve.mo $(LOCALE_PREFIX)/ve/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/ve/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   vi_VN ...Vietnamese..."
	@install -d $(LOCALE_PREFIX)/vi/LC_MESSAGES
	@cp trans/vi.mo $(LOCALE_PREFIX)/vi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/vi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   wa_BE ...Walloon..."
	@install -d $(LOCALE_PREFIX)/wa/LC_MESSAGES
	@cp trans/wa.mo $(LOCALE_PREFIX)/wa/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/wa/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   cy_GB ...Welsh..."
	@install -d $(LOCALE_PREFIX)/cy/LC_MESSAGES
	@cp trans/cy.mo $(LOCALE_PREFIX)/cy/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/cy/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   xh_ZA ...Xhosa..."
	@install -d $(LOCALE_PREFIX)/xh/LC_MESSAGES
	@cp trans/xh.mo $(LOCALE_PREFIX)/xh/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)/xh/LC_MESSAGES/tuxpaint.mo


# Install the text documentation:

install-doc:
	@echo
	@echo "...Installing documentation..."
	@install -d $(DOC_PREFIX)
	@cp -R docs/* $(DOC_PREFIX)
	@chmod a=rX,g=rX,u=rwX $(DOC_PREFIX)


# Install the man page:

install-man:
	@echo
	@echo "...Installing man pages..."
	@# man1 directory...
	@install -d $(MAN_PREFIX)/man1
	@# tuxpaint.1
	@cp src/manpage/tuxpaint.1 $(MAN_PREFIX)/man1
	@gzip -f $(MAN_PREFIX)/man1/tuxpaint.1
	@chmod a+rx,g-w,o-w $(MAN_PREFIX)/man1/tuxpaint.1.gz
	@# pl/man1 directory...
	@install -d $(MAN_PREFIX)/pl/man1/
	@# tuxpaint-pl.1
	@cp src/manpage/tuxpaint-pl.1 $(MAN_PREFIX)/pl/man1/tuxpaint.1
	@gzip -f $(MAN_PREFIX)/pl/man1/tuxpaint.1
	@chmod a+rx,g-w,o-w $(MAN_PREFIX)/pl/man1/tuxpaint.1.gz
	@# tuxpaint-import.1
	@cp src/manpage/tuxpaint-import.1 $(MAN_PREFIX)/man1/
	@gzip -f $(MAN_PREFIX)/man1/tuxpaint-import.1
	@chmod a+rx,g-w,o-w $(MAN_PREFIX)/man1/tuxpaint-import.1.gz



# Build the program!

tuxpaint:	obj/tuxpaint.o obj/i18n.o obj/cursor.o obj/pixels.o \
		obj/floodfill.o obj/rgblinear.o obj/playsound.o obj/fonts.o \
		obj/progressbar.o obj/dirwalk.o obj/get_fname.o \
		$(HQXX_O) $(ARCH_LIBS)
	@echo
	@echo "...Linking Tux Paint..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-o tuxpaint \
		$^ \
		$(HQXX_O) \
		$(SDL_LIBS) \
		-lm $(ARCH_LINKS)
	@$(RSRC_CMD)
	@$(MIMESET_CMD)


# Build the object for the program!

obj/tuxpaint.o:	src/tuxpaint.c \
		src/i18n.h src/cursor.h src/pixels.h \
		src/floodfill.h src/rgblinear.h src/playsound.h src/fonts.h \
		src/progressbar.h src/dirwalk.h src/get_fname.h \
		src/compiler.h src/debug.h \
		src/tools.h src/titles.h src/colors.h src/shapes.h \
		src/magic.h src/sounds.h src/tip_tux.h src/great.h \
		$(HQXX_H) \
		src/$(MOUSEDIR)/arrow.xbm src/$(MOUSEDIR)/arrow-mask.xbm \
		src/$(MOUSEDIR)/hand.xbm src/$(MOUSEDIR)/hand-mask.xbm \
		src/$(MOUSEDIR)/insertion.xbm \
		src/$(MOUSEDIR)/insertion-mask.xbm \
		src/$(MOUSEDIR)/wand.xbm src/$(MOUSEDIR)/wand-mask.xbm \
		src/$(MOUSEDIR)/brush.xbm src/$(MOUSEDIR)/brush-mask.xbm \
		src/$(MOUSEDIR)/crosshair.xbm \
		src/$(MOUSEDIR)/crosshair-mask.xbm \
		src/$(MOUSEDIR)/rotate.xbm src/$(MOUSEDIR)/rotate-mask.xbm \
		src/$(MOUSEDIR)/tiny.xbm src/$(MOUSEDIR)/tiny-mask.xbm \
		src/$(MOUSEDIR)/watch.xbm src/$(MOUSEDIR)/watch-mask.xbm \
		src/$(MOUSEDIR)/up.xbm src/$(MOUSEDIR)/up-mask.xbm \
		src/$(MOUSEDIR)/down.xbm src/$(MOUSEDIR)/down-mask.xbm \
		$(ARCH_HEADERS)
	@echo
	@echo "...Compiling Tux Paint from source..."
	$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(MOUSE_CFLAGS) $(DEFS) \
		-c src/tuxpaint.c -o obj/tuxpaint.o

obj/i18n.o:	src/i18n.c src/i18n.h src/debug.h
	@echo
	@echo "...Compiling i18n support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(DEFS) \
		-c src/i18n.c -o obj/i18n.o

obj/get_fname.o:	src/get_fname.c src/get_fname.h src/debug.h
	@echo
	@echo "...Compiling filename support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(DEFS) \
		-c src/get_fname.c -o obj/get_fname.o

obj/fonts.o:	src/fonts.c src/fonts.h src/dirwalk.h src/progressbar.h \
		src/get_fname.h src/debug.h
	@echo
	@echo "...Compiling font support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/fonts.c -o obj/fonts.o

obj/dirwalk.o:	src/dirwalk.c src/dirwalk.h src/progressbar.h src/fonts.h \
		src/debug.h
	@echo
	@echo "...Compiling directory-walking support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/dirwalk.c -o obj/dirwalk.o

obj/cursor.o:	src/cursor.c src/cursor.h src/debug.h
	@echo
	@echo "...Compiling cursor support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(MOUSE_CFLAGS) $(DEFS) \
		-c src/cursor.c -o obj/cursor.o

obj/pixels.o:	src/pixels.c src/pixels.h src/compiler.h src/debug.h
	@echo
	@echo "...Compiling pixel functions..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/pixels.c -o obj/pixels.o

obj/floodfill.o:	src/floodfill.c src/floodfill.h \
			src/rgblinear.h \
			src/playsound.h src/progressbar.h src/sounds.h \
			src/compiler.h src/debug.h
	@echo
	@echo "...Compiling floodfill functions..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/floodfill.c -o obj/floodfill.o

obj/playsound.o:	src/playsound.c src/playsound.h \
			src/compiler.h src/debug.h
	@echo
	@echo "...Compiling sound playback functions..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/playsound.c -o obj/playsound.o

obj/progressbar.o:	src/progressbar.c src/progressbar.h \
			src/compiler.h src/debug.h
	@echo
	@echo "...Compiling progress bar functions..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/progressbar.c -o obj/progressbar.o

obj/rgblinear.o:	src/rgblinear.c src/rgblinear.h \
			src/compiler.h src/debug.h
	@echo
	@echo "...Compiling RGB to Linear functions..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/rgblinear.c -o obj/rgblinear.o


obj/BeOS_Print.o:	src/BeOS_Print.cpp obj src/BeOS_print.h
	@echo
	@echo "...Compiling BeOS print support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/BeOS_print.cpp -o obj/BeOS_print.o

obj/win32_print.o:	src/win32_print.c obj src/win32_print.h src/debug.h
	@echo
	@echo "...Compiling win32 print support..."
	@$(CC) $(CFLAGS) $(DEBUG_FLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/win32_print.c -o obj/win32_print.o

obj/resource.o:	visualc/resources.rc obj visualc/resource.h
	@echo
	@echo "...Compiling win32 resources..."
	@windres -i visualc/resources.rc -o obj/resource.o


# Build the translation files for gettext

translations: trans \
	trans/af.mo \
	trans/ar.mo \
	trans/be.mo \
	trans/bg.mo \
	trans/bo.mo \
	trans/br.mo \
	trans/ca.mo \
	trans/cs.mo \
	trans/cy.mo \
	trans/da.mo \
	trans/de.mo \
	trans/et.mo \
	trans/el.mo \
	trans/en_gb.mo \
	trans/es.mo \
	trans/es_mx.mo \
	trans/eu.mo \
	trans/fi.mo \
	trans/fo.mo \
	trans/fr.mo \
	trans/ga.mo \
	trans/gd.mo \
	trans/gl.mo \
	trans/gos.mo \
	trans/gu.mo \
	trans/he.mo \
	trans/hi.mo \
	trans/hr.mo \
	trans/hu.mo \
	trans/id.mo \
	trans/is.mo \
	trans/it.mo \
	trans/ja.mo \
	trans/ka.mo \
	trans/ko.mo \
	trans/ku.mo \
	trans/lt.mo \
	trans/ms.mo \
	trans/nl.mo \
	trans/nb.mo \
	trans/nn.mo \
	trans/pl.mo \
	trans/pt_br.mo \
	trans/pt_pt.mo \
	trans/ro.mo \
	trans/ru.mo \
	trans/rw.mo \
	trans/sk.mo \
	trans/sl.mo \
	trans/sq.mo \
	trans/sr.mo \
	trans/sv.mo \
	trans/sw.mo \
	trans/ta.mo \
	trans/th.mo \
	trans/tl.mo \
	trans/tlh.mo \
	trans/tr.mo \
	trans/uk.mo \
	trans/ve.mo \
	trans/vi.mo \
	trans/wa.mo \
	trans/xh.mo \
	trans/zh_cn.mo \
	trans/zh_tw.mo

trans:
	@echo
	@echo "...Preparing translation files..."
	@mkdir trans

trans/af.mo:	src/po/af.po
	@echo "   af_ZA ...Afrikaans..."
	@msgfmt -o trans/af.mo src/po/af.po

trans/ar.mo:	src/po/ar.po
	@echo "   ar_SA ...Arabic..."
	@msgfmt -o trans/ar.mo src/po/ar.po

trans/be.mo:	src/po/be.po
	@echo "   be_BY ...Belarusian..."
	@msgfmt -o trans/be.mo src/po/be.po

trans/bg.mo:	src/po/bg.po
	@echo "   bg_BG ...Bulgarian..."
	@msgfmt -o trans/bg.mo src/po/bg.po

trans/bo.mo:	src/po/bo.po
	@echo "   bo_CN ...Tibetan..."
	@msgfmt -o trans/bo.mo src/po/bo.po

trans/br.mo:	src/po/br.po
	@echo "   br_FR ...Breton..."
	@msgfmt -o trans/br.mo src/po/br.po

trans/ca.mo:	src/po/ca.po
	@echo "   ca_ES ...Catalan..."
	@msgfmt -o trans/ca.mo src/po/ca.po

trans/cy.mo:	src/po/cy.po
	@echo "   cy_GB ...Welsh..."
	@msgfmt -o trans/cy.mo src/po/cy.po

trans/cs.mo:	src/po/cs.po
	@echo "   cs_CZ ...Czech..."
	@msgfmt -o trans/cs.mo src/po/cs.po

trans/da.mo:	src/po/da.po
	@echo "   da_DK ...Danish..."
	@msgfmt -o trans/da.mo src/po/da.po

trans/de.mo:	src/po/de.po
	@echo "   de_DE ...German..."
	@msgfmt -o trans/de.mo src/po/de.po

trans/et.mo:	src/po/et.po
	@echo "   et_EE ...Estonian..."
	@msgfmt -o trans/et.mo src/po/et.po

trans/el.mo:	src/po/el.po
	@echo "   el_GR ...Greek..."
	@msgfmt -o trans/el.mo src/po/el.po

trans/en_gb.mo:	src/po/en_gb.po
	@echo "   en_GB ...British English..."
	@msgfmt -o trans/en_gb.mo src/po/en_gb.po

trans/es.mo:	src/po/es.po
	@echo "   es_ES ...Spanish..."
	@msgfmt -o trans/es.mo src/po/es.po

trans/es_mx.mo:	src/po/es_mx.po
	@echo "   es_MX ...Mexcian Spanish..."
	@msgfmt -o trans/es_mx.mo src/po/es_mx.po

trans/eu.mo:	src/po/eu.po
	@echo "   eu_ES ...Basque..."
	@msgfmt -o trans/eu.mo src/po/eu.po

trans/fi.mo:	src/po/fi.po
	@echo "   fi_FI ...Finnish..."
	@msgfmt -o trans/fi.mo src/po/fi.po

trans/fo.mo:	src/po/fo.po
	@echo "   fo_FO ...Faroese..."
	@msgfmt -o trans/fo.mo src/po/fo.po

trans/fr.mo:	src/po/fr.po
	@echo "   fr_FR ...French..."
	@msgfmt -o trans/fr.mo src/po/fr.po

trans/ga.mo:	src/po/ga.po
	@echo "   ga_IE ...Irish Gaelic..."
	@msgfmt -o trans/ga.mo src/po/ga.po

trans/gd.mo:	src/po/gd.po
	@echo "   gd_GB ...Scottish Gaelic..."
	@msgfmt -o trans/gd.mo src/po/gd.po

trans/gl.mo:	src/po/gl.po
	@echo "   gl_ES ...Galician..."
	@msgfmt -o trans/gl.mo src/po/gl.po

trans/gos.mo:	src/po/gos.po
	@echo "   gos_NL ...Gronings..."
	@msgfmt -o trans/gos.mo src/po/gos.po

trans/gu.mo:	src/po/gu.po
	@echo "   gu ...Gujarati..."
	@msgfmt -o trans/gu.mo src/po/gu.po

trans/he.mo:	src/po/he.po
	@echo "   he_IL ...Hebrew..."
	@msgfmt -o trans/he.mo src/po/he.po

trans/hi.mo:	src/po/hi.po
	@echo "   hi_IN ...Hindi..."
	@msgfmt -o trans/hi.mo src/po/hi.po

trans/hr.mo:	src/po/hr.po
	@echo "   hr_HR ...Croatian..."
	@msgfmt -o trans/hr.mo src/po/hr.po

trans/hu.mo:	src/po/hu.po
	@echo "   hu_HU ...Hungarian..."
	@msgfmt -o trans/hu.mo src/po/hu.po

trans/tlh.mo:	src/po/tlh.po
	@echo "   tlh   ...Klingon (Romanized)..."
	@msgfmt -o trans/tlh.mo src/po/tlh.po

trans/id.mo:	src/po/id.po
	@echo "   id_ID ...Indonesian..."
	@msgfmt -o trans/id.mo src/po/id.po

trans/is.mo:	src/po/is.po
	@echo "   is_IS ...Icelandic..."
	@msgfmt -o trans/is.mo src/po/is.po

trans/it.mo:	src/po/it.po
	@echo "   it_IT ...Italian..."
	@msgfmt -o trans/it.mo src/po/it.po

trans/ja.mo:	src/po/ja.po
	@echo "   ja_JP ...Japanese..."
	@msgfmt -o trans/ja.mo src/po/ja.po

trans/ka.mo:	src/po/ka.po
	@echo "   ka_GE ...Georgian..."
	@msgfmt -o trans/ka.mo src/po/ka.po

trans/ko.mo:	src/po/ko.po
	@echo "   ko_KR ...Korean..."
	@msgfmt -o trans/ko.mo src/po/ko.po

trans/ku.mo:	src/po/ku.po
	@echo "   ku_TR ...Kurdish..."
	@msgfmt -o trans/ku.mo src/po/ku.po

trans/lt.mo:	src/po/lt.po
	@echo "   lt_LT ...Lithuanian..."
	@msgfmt -o trans/lt.mo src/po/lt.po

trans/ms.mo:	src/po/ms.po
	@echo "   ms_MY ...Malay..."
	@msgfmt -o trans/ms.mo src/po/ms.po

trans/nl.mo:	src/po/nl.po
	@echo "   nl_NL ...Dutch..."
	@msgfmt -o trans/nl.mo src/po/nl.po

trans/nb.mo:	src/po/nb.po
	@echo "   nb_NO ...Norwegian Bokmal..."
	@msgfmt -o trans/nb.mo src/po/nb.po

trans/nn.mo:	src/po/nn.po
	@echo "   nn_NO ...Norwegian Nynorsk..."
	@msgfmt -o trans/nn.mo src/po/nn.po

trans/pl.mo:	src/po/pl.po
	@echo "   pl_PL ...Polish..."
	@msgfmt -o trans/pl.mo src/po/pl.po

trans/pt_pt.mo:	src/po/pt_pt.po
	@echo "   pt_PT ...Portuguese..."
	@msgfmt -o trans/pt_pt.mo src/po/pt_pt.po

trans/pt_br.mo:	src/po/pt_br.po
	@echo "   pt_BR ...Brazilian Portuguese..."
	@msgfmt -o trans/pt_br.mo src/po/pt_br.po

trans/ro.mo:	src/po/ro.po
	@echo "   ro_RO ...Romanian..."
	@msgfmt -o trans/ro.mo src/po/ro.po

trans/ru.mo:	src/po/ru.po
	@echo "   ru_RU ...Russian..."
	@msgfmt -o trans/ru.mo src/po/ru.po

trans/rw.mo:	src/po/rw.po
	@echo "   rw_RW ...Kinyarwanda..."
	@msgfmt -o trans/rw.mo src/po/rw.po

trans/sk.mo:	src/po/sk.po
	@echo "   sk_SK ...Slovak..."
	@msgfmt -o trans/sk.mo src/po/sk.po

trans/sl.mo:	src/po/sl.po
	@echo "   sl_SI ...Slovenian..."
	@msgfmt -o trans/sl.mo src/po/sl.po

trans/sq.mo:	src/po/sq.po
	@echo "   sq_AL ...Albanian..."
	@msgfmt -o trans/sq.mo src/po/sq.po

trans/sr.mo:	src/po/sr.po
	@echo "   sr_YU ...Serbian..."
	@msgfmt -o trans/sr.mo src/po/sr.po

trans/sv.mo:	src/po/sv.po
	@echo "   sv_SE ...Swedish..."
	@msgfmt -o trans/sv.mo src/po/sv.po

trans/sw.mo:	src/po/sw.po
	@echo "   sw_TZ ...Swahili..."
	@msgfmt -o trans/sw.mo src/po/sw.po

trans/ta.mo:	src/po/ta.po
	@echo "   ta_IN ...Tamil..."
	@msgfmt -o trans/ta.mo src/po/ta.po

trans/th.mo:	src/po/th.po
	@echo "   th_TH ...Thai..."
	@msgfmt -o trans/th.mo src/po/th.po

trans/tl.mo:	src/po/tl.po
	@echo "   tl_PH ...Tagalog..."
	@msgfmt -o trans/tl.mo src/po/tl.po

trans/tr.mo:	src/po/tr.po
	@echo "   tr_TR ...Turkish..."
	@msgfmt -o trans/tr.mo src/po/tr.po

trans/uk.mo:	src/po/uk.po
	@echo "   uk_UA ...Ukrainian..."
	@msgfmt -o trans/uk.mo src/po/uk.po

trans/ve.mo:	src/po/ve.po
	@echo "   ve_ZA ...Venda..."
	@msgfmt -o trans/ve.mo src/po/ve.po

trans/vi.mo:	src/po/vi.po
	@echo "   vi_VN ...Vietnamese..."
	@msgfmt -o trans/vi.mo src/po/vi.po

trans/wa.mo:	src/po/wa.po
	@echo "   wa_BE ...Walloon..."
	@msgfmt -o trans/wa.mo src/po/wa.po

trans/xh.mo:	src/po/xh.po
	@echo "   xh_ZA ...Xhosa..."
	@msgfmt -o trans/xh.mo src/po/xh.po

trans/zh_cn.mo:	src/po/zh_cn.po
	@echo "   zh_CN ...Chinese (Simplified)..."
	@msgfmt -o trans/zh_cn.mo src/po/zh_cn.po

trans/zh_tw.mo:	src/po/zh_tw.po
	@echo "   zh_TW ...Chinese (Traditional)..."
	@msgfmt -o trans/zh_tw.mo src/po/zh_tw.po



# Make the "obj" directory to throw the object(s) into:
# (not necessary any more; bjk 2006.02.20)

obj:
	@mkdir obj

