# Makefile for tuxpaint

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2004 by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/tuxpaint/

# June 14, 2002 - November 21, 2004


# Where to install things:

PREFIX=/usr/local


# Root directory to place files when creating packages.

PKG_ROOT=


# Program:

BIN_PREFIX=$(PKG_ROOT)/$(PREFIX)/bin


# Data:

DATA_PREFIX=$(PKG_ROOT)/$(PREFIX)/share/tuxpaint/


# Docs and man page:

DOC_PREFIX=$(PKG_ROOT)/$(PREFIX)/share/doc/tuxpaint/
MAN_PREFIX=$(PKG_ROOT)/$(PREFIX)/share/man/


# 'System-wide' Config file:

ifeq ($(PREFIX),/usr)
  CONFDIR=$(PKG_ROOT)/etc/tuxpaint
else
  CONFDIR=$(PKG_ROOT)/$(PREFIX)/etc/tuxpaint
endif


# Commands useful to other arch's (e.g., BeOS)

RSRC_CMD=echo -n
MIMESET_CMD=echo -n


# Icons and launchers:

ICON_PREFIX=$(PKG_ROOT)/$(PREFIX)/share/pixmaps/
X11_ICON_PREFIX=$(PKG_ROOT)/$(PREFIX)/X11R6/include/X11/pixmaps/
GNOME_PREFIX=`gnome-config --prefix`
KDE_PREFIX=`kde-config --install apps --expandvars`
KDE_ICON_PREFIX=`kde-config --install icon --expandvars`


# Locale files

LOCALE_PREFIX=$(PKG_ROOT)/$(PREFIX)/share/locale/
# LOCALE_PREFIX=/usr/share/locale/


# Built with sound by default  (override with "make nosound")

NOSOUNDFLAG=__SOUND


# Where to find cursor shape XBMs

MOUSEDIR=mouse
CURSOR_SHAPES=LARGE

# MOUSEDIR=mouse/16x16
# CURSOR_SHAPES=SMALL


# Don't build with hqxx code yet...
# HQXX_O = obj/hqxx.o obj/hq4x.o
# HQXX_H = src/hqxx.h src/hq4x.h


# Libraries, paths, and flags:

SDL_LIBS=$(shell sdl-config --libs) -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)
SDL_MIXER_LIB=-lSDL_mixer
SDL_CFLAGS=$(shell sdl-config --cflags)


# The entire set of CFLAGS:

CFLAGS=-O2 -Wall
DEFS=-DDATA_PREFIX=\"$(DATA_PREFIX)\" \
	-D$(NOSOUNDFLAG) -DDOC_PREFIX=\"$(DOC_PREFIX)\" \
	-DLOCALEDIR=\"$(LOCALE_PREFIX)\" -DCONFDIR=\"$(CONFDIR)\"

MOUSE_CFLAGS=-Isrc/$(MOUSEDIR) -D$(CURSOR_SHAPES)_CURSOR_SHAPES


# "make" with no arguments builds the program and man page from sources:

all:	tuxpaint translations
	@echo
	@echo "--------------------------------------------------------------"
	@echo
	@echo "Done compiling."
	@echo "Now (probably as 'root' superuser), run 'make install'"
	@echo "to install Tux Paint."
	@echo



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
		DATA_PREFIX=./data/ \
		DOC_PREFIX=./docs/ \
		MAN_PREFIX=./src/ \
		CONFDIR=./src/ \
		ICON_PREFIX=./
		X11_ICON_PREFIX=./ \
		LOCALE_PREFIX=/boot/home/config/share/locale/ \
		CFLAGS="-O1 -funroll-loops -fomit-frame-pointer -pipe -Wall" \
		RSRC_CMD="xres -o tuxpaint tuxpaint.rsrc" \
		MIMESET_CMD="mimeset -f tuxpaint" \
		ARCH_LINKS="-lintl -lpng -lz -lbe" \
		ARCH_HEADERS="src/BeOS_Print.h" \
		ARCH_LIBS="obj/BeOS_print.o"


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
	@echo "run 'tuxpaint --usage' or see $(DOC_PREFIX)README.txt"
	@echo
	@echo "Visit Tux Paint's home page for more information, updates"
	@echo "and to learn how you can help out!"
	@echo
	@echo "  http://www.newbreedsoftware.com/tuxpaint/"
	@echo
	@echo "Enjoy!"
	@echo


# "make install-beos" installs Tux Paint, but using BeOS settings

install-beos:
	make install \
		PREFIX=/boot/develop/tools/gnupro \
		BIN_PREFIX=./ \
		DATA_PREFIX=./data/ \
		DOC_PREFIX=./docs/ \
		MAN_PREFIX=./src/ \
		CONFDIR=./src/ \
		ICON_PREFIX=./
		X11_ICON_PREFIX=./ \
		LOCALE_PREFIX=/boot/home/config/share/locale/ \
		CFLAGS="-O1 -funroll-loops -fomit-frame-pointer -pipe -Wall" \
		RSRC_CMD="xres -o tuxpaint tuxpaint.rsrc" \
		MIMESET_CMD="mimeset -f tuxpaint" \
		ARCH_LINKS="-lintl -lpng -lz -lbe" \
		ARCH_HEADERS="src/BeOS_Print.h" \
		ARCH_LIBS="obj/BeOS_print.o"


# "make clean" deletes the program, the compiled objects and the
# built man page (returns to factory archive, pretty much...)

clean:
	@echo
	@echo "Cleaning up the build directory! ($(PWD))"
	@-rm -f tuxpaint
	@-rm -f obj/*.o
	@if [ -d obj ]; then rmdir obj; fi
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
	-rm $(ICON_PREFIX)tuxpaint.png
	-rm $(X11_ICON_PREFIX)tuxpaint.xpm
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
	-rm $(LOCALE_PREFIX)af/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)be/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)bg/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)br/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ca/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)cs/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)cy/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)da/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)de/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)el/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)en_GB/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)es/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)eu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)fi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)fr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)gl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)he/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)hi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)hr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)hu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)tlh/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)id/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)is/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)it/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ja/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ko/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)lt/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ms/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)nl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)nb/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)nn/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pt_PT/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ro/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ru/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sk/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sq/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sv/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ta/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)tr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)vi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)zh_CN/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)zh_TW/LC_MESSAGES/tuxpaint.mo
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
	@install -d $(DATA_PREFIX)stamps
	@cp -R stamps/* $(DATA_PREFIX)stamps
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)stamps


# Install example starters

install-example-starters:
	@echo
	@echo "...Installing example starter images..."
	@install -d $(DATA_PREFIX)starters
	@cp -R starters/* $(DATA_PREFIX)starters
	@chmod -R a+rX,g-w,o-w $(DATA_PREFIX)starters


# Install a launcher icon in the Gnome menu, under "Graphics"

install-gnome:
	@echo
	@echo "...Installing launcher icon into GNOME..."
	@if [ "x$(GNOME_PREFIX)" != "x" ]; then \
	 install -d $(PKG_ROOT)/$(GNOME_PREFIX)/share/pixmaps; \
	 cp data/images/icon.png $(PKG_ROOT)/$(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 chmod 644 $(PKG_ROOT)/$(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 install -d $(PKG_ROOT)/$(GNOME_PREFIX)/share/gnome/apps/Graphics; \
	 cp src/tuxpaint.desktop $(PKG_ROOT)/$(GNOME_PREFIX)/share/gnome/apps/Graphics/; \
	 chmod 644 $(PKG_ROOT)/$(GNOME_PREFIX)/share/gnome/apps/Graphics/tuxpaint.desktop; \
	else \
	 make install-gnome GNOME_PREFIX=$(PREFIX); \
	fi


# Install a launcher icon in the KDE menu...

install-kde:
	@echo
	@echo "...Installing launcher icon into KDE..."
	@if [ "x$(KDE_PREFIX)" != "x" ]; then \
	  install -d $(PKG_ROOT)/$(KDE_PREFIX)/Graphics; \
	  cp src/tuxpaint.desktop $(PKG_ROOT)/$(KDE_PREFIX)/Graphics/; \
	  chmod 644 $(PKG_ROOT)/$(KDE_PREFIX)/Graphics/tuxpaint.desktop; \
	else \
	  make KDE_PREFIX=$(PREFIX)/share/applnk install-kde; \
	fi


install-kde-icons:
	@echo "...Installing launcher icon graphics into KDE..."
	@if [ "x$(KDE_ICON_PREFIX)" != "x" ]; then \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/scalable/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/192x192/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/128x128/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/96x96/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/64x64/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/48x48/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/32x32/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/22x22/apps/; \
	  install -d $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/16x16/apps/; \
	  cp data/images/tuxpaint-icon.svg \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/scalable/apps/tuxpaint.svg; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/scalable/apps/tuxpaint.svg; \
	  cp data/images/icon192x192.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/192x192/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/192x192/apps/tuxpaint.png; \
	  cp data/images/icon128x128.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/128x128/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/128x128/apps/tuxpaint.png; \
	  cp data/images/icon96x96.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/96x96/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/96x96/apps/tuxpaint.png; \
	  cp data/images/icon64x64.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/64x64/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/64x64/apps/tuxpaint.png; \
	  cp data/images/icon48x48.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
	  cp data/images/icon32x32.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
	  cp data/images/icon22x22.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/22x22/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/22x22/apps/tuxpaint.png; \
	  cp data/images/icon16x16.png \
		$(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
          chmod 644 $(PKG_ROOT)/$(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
	fi


# Install the PNG icon (for GNOME, KDE, etc.)
# and the 24-color 32x32 XPM (for other Window managers):

# FIXME: Should this also use $(PKG_ROOT)?

install-icon:
	@echo
	@echo "...Installing launcher icon graphics..."
	@install -d $(ICON_PREFIX)
	@cp data/images/icon.png $(ICON_PREFIX)tuxpaint.png
	@chmod 644 $(ICON_PREFIX)tuxpaint.png
	@install -d $(X11_ICON_PREFIX)
	@cp data/images/icon32x32.xpm $(X11_ICON_PREFIX)tuxpaint.xpm
	@chmod 644 $(X11_ICON_PREFIX)tuxpaint.xpm


# Install the program:

install-bin:
	@echo
	@echo "...Installing program itself..."
	@install -d $(BIN_PREFIX)
	@cp tuxpaint $(BIN_PREFIX)
	@chmod a+rx,g-w,o-w $(BIN_PREFIX)/tuxpaint


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


# Install the translated text:

install-gettext:
	@echo
	@echo "...Installing translation files..."
	@#
	@echo "   af_ZA ...Afrikaans..."
	@install -d $(LOCALE_PREFIX)af/LC_MESSAGES
	@cp trans/af.mo $(LOCALE_PREFIX)af/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)af/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sq_AL ...Albanian..."
	@install -d $(LOCALE_PREFIX)sq/LC_MESSAGES
	@cp trans/sq.mo $(LOCALE_PREFIX)sq/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)sq/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   be_BY ...Belarusian..."
	@install -d $(LOCALE_PREFIX)be/LC_MESSAGES
	@cp trans/be.mo $(LOCALE_PREFIX)be/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)be/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pt_BR ...Brazilian Portuguese..."
	@install -d $(LOCALE_PREFIX)pt_BR/LC_MESSAGES
	@cp trans/pt_br.mo $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   br_FR ...Breton..."
	@install -d $(LOCALE_PREFIX)br/LC_MESSAGES
	@cp trans/br.mo $(LOCALE_PREFIX)br/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)br/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   eu_ES ...Basque..."
	@install -d $(LOCALE_PREFIX)eu/LC_MESSAGES
	@cp trans/eu.mo $(LOCALE_PREFIX)eu/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)eu/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   en_GB ...British English..."
	@install -d $(LOCALE_PREFIX)en_GB/LC_MESSAGES
	@cp trans/en_gb.mo $(LOCALE_PREFIX)en_GB/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)en_GB/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   bg_BG ...Bulgarian..."
	@install -d $(LOCALE_PREFIX)bg/LC_MESSAGES
	@cp trans/bg.mo $(LOCALE_PREFIX)bg/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)bg/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ca_ES ...Catalan..."
	@install -d $(LOCALE_PREFIX)ca/LC_MESSAGES
	@cp trans/ca.mo $(LOCALE_PREFIX)ca/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ca/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   zh_CN ...Chinese (Simplified)..."
	@install -d $(LOCALE_PREFIX)zh_CN/LC_MESSAGES
	@cp trans/zh_cn.mo $(LOCALE_PREFIX)zh_CN/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)zh_CN/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   zh_TW ...Chinese (Traditional)..."
	@install -d $(LOCALE_PREFIX)zh_TW/LC_MESSAGES
	@cp trans/zh_tw.mo $(LOCALE_PREFIX)zh_TW/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)zh_TW/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hr_HR ...Croatian..."
	@install -d $(LOCALE_PREFIX)hr/LC_MESSAGES
	@cp trans/hr.mo $(LOCALE_PREFIX)hr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)hr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   cs_CZ ...Czech..."
	@install -d $(LOCALE_PREFIX)cs/LC_MESSAGES
	@cp trans/cs.mo $(LOCALE_PREFIX)cs/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)cs/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   da_DK ...Danish..."
	@install -d $(LOCALE_PREFIX)da/LC_MESSAGES
	@cp trans/da.mo $(LOCALE_PREFIX)da/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)da/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nl_NL ...Dutch..."
	@install -d $(LOCALE_PREFIX)nl/LC_MESSAGES
	@cp trans/nl.mo $(LOCALE_PREFIX)nl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)nl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   fi_FI ...Finnish..."
	@install -d $(LOCALE_PREFIX)fi/LC_MESSAGES
	@cp trans/fi.mo $(LOCALE_PREFIX)fi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)fi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   fr_FR ...French..."
	@install -d $(LOCALE_PREFIX)fr/LC_MESSAGES
	@cp trans/fr.mo $(LOCALE_PREFIX)fr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)fr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   gl_ES ...Galician..."
	@install -d $(LOCALE_PREFIX)gl/LC_MESSAGES
	@cp trans/gl.mo $(LOCALE_PREFIX)gl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)gl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   de_DE ...German..."
	@install -d $(LOCALE_PREFIX)de/LC_MESSAGES
	@cp trans/de.mo $(LOCALE_PREFIX)de/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)de/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   el_GR ...Greek..."
	@install -d $(LOCALE_PREFIX)el/LC_MESSAGES
	@cp trans/el.mo $(LOCALE_PREFIX)el/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)el/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   he_IL ...Hebrew..."
	@install -d $(LOCALE_PREFIX)he/LC_MESSAGES
	@cp trans/he.mo $(LOCALE_PREFIX)he/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)he/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hi_IN ...Hindi..."
	@install -d $(LOCALE_PREFIX)hi/LC_MESSAGES
	@cp trans/hi.mo $(LOCALE_PREFIX)hi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)hi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   hu_HU ...Hungarian..."
	@install -d $(LOCALE_PREFIX)hu/LC_MESSAGES
	@cp trans/hu.mo $(LOCALE_PREFIX)hu/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)hu/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   is_IS ...Icelandic..."
	@install -d $(LOCALE_PREFIX)is/LC_MESSAGES
	@cp trans/is.mo $(LOCALE_PREFIX)is/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)is/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   id_ID ...Indonesian..."
	@install -d $(LOCALE_PREFIX)id/LC_MESSAGES
	@cp trans/id.mo $(LOCALE_PREFIX)id/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)id/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   it_IT ...Italian..."
	@install -d $(LOCALE_PREFIX)it/LC_MESSAGES
	@cp trans/it.mo $(LOCALE_PREFIX)it/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)it/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ja_JP ...Japanese..."
	@install -d $(LOCALE_PREFIX)ja/LC_MESSAGES
	@cp trans/ja.mo $(LOCALE_PREFIX)ja/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ja/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   tlh   ...Klingon (Romanized)..."
	@install -d $(LOCALE_PREFIX)tlh/LC_MESSAGES
	@cp trans/tlh.mo $(LOCALE_PREFIX)tlh/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)tlh/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ko_KR ...Korean..."
	@install -d $(LOCALE_PREFIX)ko/LC_MESSAGES
	@cp trans/ko.mo $(LOCALE_PREFIX)ko/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ko/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   lt_LT ...Lithuanian..."
	@install -d $(LOCALE_PREFIX)lt/LC_MESSAGES
	@cp trans/lt.mo $(LOCALE_PREFIX)lt/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)lt/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ms_MY ...Malay..."
	@install -d $(LOCALE_PREFIX)ms/LC_MESSAGES
	@cp trans/ms.mo $(LOCALE_PREFIX)ms/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ms/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nb_NO ...Norwegian Bokmal..."
	@install -d $(LOCALE_PREFIX)nb/LC_MESSAGES
	@cp trans/nb.mo $(LOCALE_PREFIX)nb/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)nb/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   nn_NO ...Norwegian Nynorsk..."
	@install -d $(LOCALE_PREFIX)nn/LC_MESSAGES
	@cp trans/nn.mo $(LOCALE_PREFIX)nn/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)nn/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pl_PL ...Polish..."
	@install -d $(LOCALE_PREFIX)pl/LC_MESSAGES
	@cp trans/pl.mo $(LOCALE_PREFIX)pl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)pl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   pt_PT ...Portuguese (Portugal)..."
	@install -d $(LOCALE_PREFIX)pt_PT/LC_MESSAGES
	@cp trans/pt_pt.mo $(LOCALE_PREFIX)pt_PT/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)pt_PT/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ro_RO ...Romanian..."
	@install -d $(LOCALE_PREFIX)ro/LC_MESSAGES
	@cp trans/ro.mo $(LOCALE_PREFIX)ro/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ro/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ru_RU ...Russian..."
	@install -d $(LOCALE_PREFIX)ru/LC_MESSAGES
	@cp trans/ru.mo $(LOCALE_PREFIX)ru/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ru/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sr_YU ...Serbian..."
	@install -d $(LOCALE_PREFIX)sr/LC_MESSAGES
	@cp trans/sr.mo $(LOCALE_PREFIX)sr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)sr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sk_SK ...Slovak..."
	@install -d $(LOCALE_PREFIX)sk/LC_MESSAGES
	@cp trans/sk.mo $(LOCALE_PREFIX)sk/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)sk/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sl_SI ...Slovenian..."
	@install -d $(LOCALE_PREFIX)sl/LC_MESSAGES
	@cp trans/sl.mo $(LOCALE_PREFIX)sl/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)sl/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   es_ES ...Spanish..."
	@install -d $(LOCALE_PREFIX)es/LC_MESSAGES
	@cp trans/es.mo $(LOCALE_PREFIX)es/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)es/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   sv_SE ...Swedish..."
	@install -d $(LOCALE_PREFIX)sv/LC_MESSAGES
	@cp trans/sv.mo $(LOCALE_PREFIX)sv/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)sv/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   ta_IN ...Tamil..."
	@install -d $(LOCALE_PREFIX)ta/LC_MESSAGES
	@cp trans/ta.mo $(LOCALE_PREFIX)ta/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)ta/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   tr_TR ...Turkish..."
	@install -d $(LOCALE_PREFIX)tr/LC_MESSAGES
	@cp trans/tr.mo $(LOCALE_PREFIX)tr/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)tr/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   vi_VN ...Vietnamese..."
	@install -d $(LOCALE_PREFIX)vi/LC_MESSAGES
	@cp trans/vi.mo $(LOCALE_PREFIX)vi/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)vi/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   wa_BE ...Walloon..."
	@install -d $(LOCALE_PREFIX)wa/LC_MESSAGES
	@cp trans/wa.mo $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo
	@#
	@echo "   cy_GB ...Welsh..."
	@install -d $(LOCALE_PREFIX)cy/LC_MESSAGES
	@cp trans/cy.mo $(LOCALE_PREFIX)cy/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)cy/LC_MESSAGES/tuxpaint.mo


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
	@install -d $(MAN_PREFIX)/man1/
	@# tuxpaint.1
	@cp src/manpage/tuxpaint.1 $(MAN_PREFIX)/man1/
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

tuxpaint:	obj/tuxpaint.o $(HQXX_O) $(ARCH_LIBS)
	@echo
	@echo "...Linking Tux Paint..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEFS) \
		-o tuxpaint obj/tuxpaint.o $(HQXX_O) \
		$(ARCH_LIBS) $(SDL_LIBS) \
		-lm $(ARCH_LINKS)
	@$(RSRC_CMD)
	@$(MIMESET_CMD)


# Build the object for the program!

obj/tuxpaint.o:	src/tuxpaint.c obj \
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
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) $(MOUSE_CFLAGS) $(DEFS) \
		-c src/tuxpaint.c -o obj/tuxpaint.o


obj/BeOS_Print.o:	src/BeOS_Print.cpp obj src/BeOS_print.h
	@echo
	@echo "...Compiling BeOS print support..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEFS) \
		-c src/BeOS_print.cpp -o obj/BeOS_print.o


obj/hq4x.o:	src/hq4x.c src/hq4x.h src/hqxx.h
	@echo
	@echo "...Compiling high quality 4x scale filter..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) \
		-c src/hq4x.c -o obj/hq4x.o

obj/hqxx.o:	src/hqxx.c src/hqxx.h
	@echo
	@echo "...Compiling high quality scale filter helpers..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) \
		-c src/hqxx.c -o obj/hqxx.o


# Build the translation files for gettext

translations: trans \
	trans/af.mo \
	trans/be.mo \
	trans/bg.mo \
	trans/br.mo \
	trans/ca.mo \
	trans/cs.mo \
	trans/cy.mo \
	trans/da.mo \
	trans/de.mo \
	trans/el.mo \
	trans/en_gb.mo \
	trans/es.mo \
	trans/eu.mo \
	trans/fi.mo \
	trans/fr.mo \
	trans/gl.mo \
	trans/he.mo \
	trans/hi.mo \
	trans/hr.mo \
	trans/hu.mo \
	trans/id.mo \
	trans/is.mo \
	trans/it.mo \
	trans/ja.mo \
	trans/ko.mo \
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
	trans/sk.mo \
	trans/sl.mo \
	trans/sq.mo \
	trans/sr.mo \
	trans/sv.mo \
	trans/ta.mo \
	trans/tlh.mo \
	trans/tr.mo \
	trans/vi.mo \
	trans/wa.mo \
	trans/zh_cn.mo \
	trans/zh_tw.mo

trans:
	@echo
	@echo "...Preparing translation files..."
	@mkdir trans

trans/af.mo:	src/po/af.po
	@echo "   af_ZA ...Afrikaans..."
	@msgfmt src/po/af.po -o trans/af.mo

trans/be.mo:	src/po/be.po
	@echo "   be_BY ...Belarusian..."
	@msgfmt src/po/be.po -o trans/be.mo

trans/bg.mo:	src/po/bg.po
	@echo "   bg_BG ...Bulgarian..."
	@msgfmt src/po/bg.po -o trans/bg.mo

trans/br.mo:	src/po/br.po
	@echo "   br_FR ...Breton..."
	@msgfmt src/po/br.po -o trans/br.mo

trans/ca.mo:	src/po/ca.po
	@echo "   ca_ES ...Catalan..."
	@msgfmt src/po/ca.po -o trans/ca.mo

trans/cy.mo:	src/po/cy.po
	@echo "   cy_GB ...Welsh..."
	@msgfmt src/po/cy.po -o trans/cy.mo

trans/cs.mo:	src/po/cs.po
	@echo "   cs_CZ ...Czech..."
	@msgfmt src/po/cs.po -o trans/cs.mo

trans/da.mo:	src/po/da.po
	@echo "   da_DK ...Danish..."
	@msgfmt src/po/da.po -o trans/da.mo

trans/de.mo:	src/po/de.po
	@echo "   de_DE ...German..."
	@msgfmt src/po/de.po -o trans/de.mo

trans/el.mo:	src/po/el.po
	@echo "   el_GR ...Greek..."
	@msgfmt src/po/el.po -o trans/el.mo

trans/en_gb.mo:	src/po/en_gb.po
	@echo "   en_GB ...British English..."
	@msgfmt src/po/en_gb.po -o trans/en_gb.mo

trans/es.mo:	src/po/es.po
	@echo "   es_ES ...Spanish..."
	@msgfmt src/po/es.po -o trans/es.mo

trans/eu.mo:	src/po/eu.po
	@echo "   eu_ES ...Basque..."
	@msgfmt src/po/eu.po -o trans/eu.mo

trans/fi.mo:	src/po/fi.po
	@echo "   fi_FI ...Finnish..."
	@msgfmt src/po/fi.po -o trans/fi.mo

trans/fr.mo:	src/po/fr.po
	@echo "   fr_FR ...French..."
	@msgfmt src/po/fr.po -o trans/fr.mo

trans/gl.mo:	src/po/gl.po
	@echo "   gl_ES ...Galician..."
	@msgfmt src/po/gl.po -o trans/gl.mo

trans/he.mo:	src/po/he.po
	@echo "   he_IL ...Hebrew..."
	@msgfmt src/po/he.po -o trans/he.mo

trans/hi.mo:	src/po/hi.po
	@echo "   hi_IN ...Hindi..."
	@msgfmt src/po/hi.po -o trans/hi.mo

trans/hr.mo:	src/po/hr.po
	@echo "   hr_HR ...Croatian..."
	@msgfmt src/po/hr.po -o trans/hr.mo

trans/hu.mo:	src/po/hu.po
	@echo "   hu_HU ...Hungarian..."
	@msgfmt src/po/hu.po -o trans/hu.mo

trans/tlh.mo:	src/po/tlh.po
	@echo "   tlh   ...Klingon (Romanized)..."
	@msgfmt src/po/tlh.po -o trans/tlh.mo

trans/id.mo:	src/po/id.po
	@echo "   id_ID ...Indonesian..."
	@msgfmt src/po/id.po -o trans/id.mo

trans/is.mo:	src/po/is.po
	@echo "   is_IS ...Icelandic..."
	@msgfmt src/po/is.po -o trans/is.mo

trans/it.mo:	src/po/it.po
	@echo "   it_IT ...Italian..."
	@msgfmt src/po/it.po -o trans/it.mo

trans/ja.mo:	src/po/ja.po
	@echo "   ja_JP ...Japanese..."
	@msgfmt src/po/ja.po -o trans/ja.mo

trans/ko.mo:	src/po/ko.po
	@echo "   ko_KR ...Korean..."
	@msgfmt src/po/ko.po -o trans/ko.mo

trans/lt.mo:	src/po/lt.po
	@echo "   lt_LT ...Lithuanian..."
	@msgfmt src/po/lt.po -o trans/lt.mo

trans/ms.mo:	src/po/ms.po
	@echo "   ms_MY ...Malay..."
	@msgfmt src/po/ms.po -o trans/ms.mo

trans/nl.mo:	src/po/nl.po
	@echo "   nl_NL ...Dutch..."
	@msgfmt src/po/nl.po -o trans/nl.mo

trans/nb.mo:	src/po/nb.po
	@echo "   nb_NO ...Norwegian Bokmal..."
	@msgfmt src/po/nb.po -o trans/nb.mo

trans/nn.mo:	src/po/nn.po
	@echo "   nn_NO ...Norwegian Nynorsk..."
	@msgfmt src/po/nn.po -o trans/nn.mo

trans/pl.mo:	src/po/pl.po
	@echo "   pl_PL ...Polish..."
	@msgfmt src/po/pl.po -o trans/pl.mo

trans/pt_pt.mo:	src/po/pt_pt.po
	@echo "   pt_PT ...Portuguese..."
	@msgfmt src/po/pt_pt.po -o trans/pt_pt.mo

trans/pt_br.mo:	src/po/pt_br.po
	@echo "   pt_BR ...Brazilian Portuguese..."
	@msgfmt src/po/pt_br.po -o trans/pt_br.mo

trans/ro.mo:	src/po/ro.po
	@echo "   ro_RO ...Romanian..."
	@msgfmt src/po/ro.po -o trans/ro.mo

trans/ru.mo:	src/po/ru.po
	@echo "   ru_RU ...Russian..."
	@msgfmt src/po/ru.po -o trans/ru.mo

trans/sk.mo:	src/po/sk.po
	@echo "   sk_SK ...Slovak..."
	@msgfmt src/po/sk.po -o trans/sk.mo

trans/sl.mo:	src/po/sl.po
	@echo "   sl_SI ...Slovenian..."
	@msgfmt src/po/sl.po -o trans/sl.mo

trans/sq.mo:	src/po/sq.po
	@echo "   sq_AL ...Albanian..."
	@msgfmt src/po/sq.po -o trans/sq.mo

trans/sr.mo:	src/po/sr.po
	@echo "   sr_YU ...Serbian..."
	@msgfmt src/po/sr.po -o trans/sr.mo

trans/sv.mo:	src/po/sv.po
	@echo "   sv_SE ...Swedish..."
	@msgfmt src/po/sv.po -o trans/sv.mo

trans/ta.mo:	src/po/ta.po
	@echo "   ta_IN ...Tamil..."
	@msgfmt src/po/ta.po -o trans/ta.mo

trans/tr.mo:	src/po/tr.po
	@echo "   tr_TR ...Turkish..."
	@msgfmt src/po/tr.po -o trans/tr.mo

trans/vi.mo:	src/po/vi.po
	@echo "   vi_VN ...Vietnamese..."
	@msgfmt src/po/vi.po -o trans/vi.mo

trans/wa.mo:	src/po/wa.po
	@echo "   wa_BE ...Walloon..."
	@msgfmt src/po/wa.po -o trans/wa.mo

trans/zh_cn.mo:	src/po/zh_cn.po
	@echo "   zh_CN ...Chinese..."
	@msgfmt src/po/zh_cn.po -o trans/zh_cn.mo

trans/zh_tw.mo:	src/po/zh_tw.po
	@echo "   zh_TW ...Chinese..."
	@msgfmt src/po/zh_tw.po -o trans/zh_tw.mo



# Make the "obj" directory to throw the object(s) into:

obj:
	@mkdir obj

