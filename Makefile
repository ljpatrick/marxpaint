# Makefile for tuxpaint

# Tux Paint - A simple drawing program for children.

# Copyright (c) 2004 by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/tuxpaint/

# June 14, 2002 - March 4, 2004


# Where to install things:

PREFIX=/usr/local


# Program:

BIN_PREFIX=$(PREFIX)/bin


# Data:

DATA_PREFIX=$(PREFIX)/share/tuxpaint/


# Docs and man page:

DOC_PREFIX=$(PREFIX)/share/doc/tuxpaint/
MAN_PREFIX=$(PREFIX)/share/man/


# 'System-wide' Config file:

ifeq ($(PREFIX),/usr)
  CONFDIR=/etc/tuxpaint
else
  CONFDIR=$(PREFIX)/etc/tuxpaint
endif


# Commands useful to other arch's (e.g., BeOS)

RSRC_CMD=echo -n
MIMESET_CMD=echo -n


# Icons and launchers:

ICON_PREFIX=$(PREFIX)/share/pixmaps/
X11_ICON_PREFIX=$(PREFIX)/X11R6/include/X11/pixmaps/
GNOME_PREFIX=`gnome-config --prefix`
KDE_PREFIX=`kde-config --install apps --expandvars`
KDE_ICON_PREFIX=`kde-config --install icon --expandvars`


# Locale files

LOCALE_PREFIX=$(PREFIX)/share/locale/
# LOCALE_PREFIX=/usr/share/locale/


# Built with sound by default  (override with "make nosound")

NOSOUNDFLAG=__SOUND


# Libraries, paths, and flags:

SDL_LIBS=$(shell sdl-config --libs) -lSDL_image -lSDL_ttf $(SDL_MIXER_LIB)
SDL_MIXER_LIB=-lSDL_mixer
SDL_CFLAGS=$(shell sdl-config --cflags)


# The entire set of CFLAGS:

CFLAGS=-O2 -Wall
DEFS=-DDATA_PREFIX=\"$(DATA_PREFIX)\" \
	-D$(NOSOUNDFLAG) -DDOC_PREFIX=\"$(DOC_PREFIX)\" \
	-DLOCALEDIR=\"$(LOCALE_PREFIX)\" -DCONFDIR=\"$(CONFDIR)\"


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
		install-gnome install-kde install-kde-icons \
		install-icon install-gettext install-importscript \
		install-default-config install-example-stamps
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
	-rm $(BIN_PREFIX)/tuxpaint
	-rm $(BIN_PREFIX)/tuxpaint-import
	-rm -r $(DATA_PREFIX)
	-rm -r $(DOC_PREFIX)
	-rm $(MAN_PREFIX)/man1/tuxpaint.1.gz
	-rm $(LOCALE_PREFIX)af/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ca/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)cs/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)da/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)de/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)el/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)en_GB/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)es/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)eu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)fi/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)fr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)he/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)hu/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)id/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)is/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)it/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ja/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ko/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)lt/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ms/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)nl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)nn/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pl/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pt/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ro/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ru/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sk/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)sv/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)ta/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)tr/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo
	-rm $(LOCALE_PREFIX)zh_CN/LC_MESSAGES/tuxpaint.mo
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


# Install a launcher icon in the Gnome menu, under "Graphics"

install-gnome:
	@echo
	@echo "...Installing launcher icon into GNOME..."
	@if [ "x$(GNOME_PREFIX)" != "x" ]; then \
	 install -d $(GNOME_PREFIX)/share/pixmaps; \
	 cp data/images/icon.png $(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 chmod 644 $(GNOME_PREFIX)/share/pixmaps/tuxpaint.png; \
	 install -d $(GNOME_PREFIX)/share/gnome/apps/Graphics; \
	 cp src/tuxpaint.desktop $(GNOME_PREFIX)/share/gnome/apps/Graphics/; \
	 chmod 644 $(GNOME_PREFIX)/share/gnome/apps/Graphics/tuxpaint.desktop; \
	else \
	 make install-gnome GNOME_PREFIX=/usr; \
	fi


# Install a launcher icon in the KDE menu...

install-kde:
	@echo
	@echo "...Installing launcher icon into KDE..."
	@if [ "x$(KDE_PREFIX)" != "x" ]; then \
	  cp src/tuxpaint.desktop $(KDE_PREFIX)/Graphics/; \
	  chmod 644 $(KDE_PREFIX)/Graphics/tuxpaint.desktop; \
	fi


install-kde-icons:
	@echo "...Installing launcher icon graphics into KDE..."
	@if [ "x$(KDE_ICON_PREFIX)" != "x" ]; then \
	  install -d $(KDE_ICON_PREFIX)/hicolor/48x48/apps/; \
	  install -d $(KDE_ICON_PREFIX)/hicolor/32x32/apps/; \
	  install -d $(KDE_ICON_PREFIX)/hicolor/16x16/apps/; \
	  cp data/images/icon48x48.png \
		$(KDE_ICON_PREFIX)/hicolor/48x48/apps/tuxpaint.png; \
	  cp data/images/icon32x32.png \
		$(KDE_ICON_PREFIX)/hicolor/32x32/apps/tuxpaint.png; \
	  cp data/images/icon16x16.png \
		$(KDE_ICON_PREFIX)/hicolor/16x16/apps/tuxpaint.png; \
	fi


# Install the PNG icon (for GNOME, KDE, etc.)
# and the 24-color 32x32 XPM (for other Window managers):

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
	@echo "   pt_BR ...Brazilian Portuguese..."
	@install -d $(LOCALE_PREFIX)pt_BR/LC_MESSAGES
	@cp trans/pt_br.mo $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)pt_BR/LC_MESSAGES/tuxpaint.mo
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
	@echo "   pt_PT ...Portuguese..."
	@install -d $(LOCALE_PREFIX)pt/LC_MESSAGES
	@cp trans/pt.mo $(LOCALE_PREFIX)pt/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)pt/LC_MESSAGES/tuxpaint.mo
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
	@echo "   wa_BE ...Walloon..."
	@install -d $(LOCALE_PREFIX)wa/LC_MESSAGES
	@cp trans/wa.mo $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo
	@chmod 644 $(LOCALE_PREFIX)wa/LC_MESSAGES/tuxpaint.mo


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

tuxpaint:	obj/tuxpaint.o obj/hqxx.o obj/hq4x.o $(ARCH_LIBS)
	@echo
	@echo "...Linking Tux Paint..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEFS) \
		-o tuxpaint obj/tuxpaint.o obj/hqxx.o obj/hq4x.o \
		$(ARCH_LIBS) $(SDL_LIBS) \
		-lm $(ARCH_LINKS)
	@$(RSRC_CMD)
	@$(MIMESET_CMD)


# Build the object for the program!

obj/tuxpaint.o:	src/tuxpaint.c obj \
		src/tools.h src/titles.h src/colors.h src/shapes.h \
		src/magic.h src/sounds.h src/tip_tux.h src/great.h \
		src/hqxx.h src/hq4x.h \
		src/mouse/arrow.xbm src/mouse/arrow-mask.xbm \
		src/mouse/hand.xbm src/mouse/hand-mask.xbm \
		src/mouse/insertion.xbm src/mouse/insertion-mask.xbm \
		src/mouse/wand.xbm src/mouse/wand-mask.xbm \
		src/mouse/brush.xbm src/mouse/brush-mask.xbm \
		src/mouse/crosshair.xbm src/mouse/crosshair-mask.xbm \
		src/mouse/rotate.xbm src/mouse/rotate-mask.xbm \
		src/mouse/tiny.xbm src/mouse/tiny-mask.xbm \
		src/mouse/watch.xbm src/mouse/watch-mask.xbm \
		src/mouse/up.xbm src/mouse/up-mask.xbm \
		src/mouse/down.xbm src/mouse/down-mask.xbm \
		$(ARCH_HEADERS)
	@echo
	@echo "...Compiling Tux Paint from source..."
	@$(CC) $(CFLAGS) $(SDL_CFLAGS) $(DEFS) \
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
	trans/ca.mo \
	trans/cs.mo \
	trans/da.mo \
	trans/de.mo \
	trans/el.mo \
	trans/en_gb.mo \
	trans/es.mo \
	trans/eu.mo \
	trans/fi.mo \
	trans/fr.mo \
	trans/he.mo \
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
	trans/pt.mo \
	trans/ro.mo \
	trans/ru.mo \
	trans/sk.mo \
	trans/sr.mo \
	trans/sv.mo \
	trans/ta.mo \
	trans/tr.mo \
	trans/wa.mo \
	trans/zh_cn.mo

trans:
	@echo
	@echo "...Preparing translation files..."
	@mkdir trans

trans/af.mo:	src/messages/af.po
	@echo "   af_ZA ...Afrikaans..."
	@msgfmt src/messages/af.po -o trans/af.mo

trans/ca.mo:	src/messages/ca.po
	@echo "   ca_ES ...Catalan..."
	@msgfmt src/messages/ca.po -o trans/ca.mo

trans/cs.mo:	src/messages/cs.po
	@echo "   cs_CZ ...Czech..."
	@msgfmt src/messages/cs.po -o trans/cs.mo

trans/da.mo:	src/messages/da.po
	@echo "   da_DK ...Danish..."
	@msgfmt src/messages/da.po -o trans/da.mo

trans/de.mo:	src/messages/de.po
	@echo "   de_DE ...German..."
	@msgfmt src/messages/de.po -o trans/de.mo

trans/el.mo:	src/messages/el.po
	@echo "   el_GR ...Greek..."
	@msgfmt src/messages/el.po -o trans/el.mo

trans/en_gb.mo:	src/messages/en_gb.po
	@echo "   en_GB ...British English..."
	@msgfmt src/messages/en_gb.po -o trans/en_gb.mo

trans/es.mo:	src/messages/es.po
	@echo "   es_ES ...Spanish..."
	@msgfmt src/messages/es.po -o trans/es.mo

trans/eu.mo:	src/messages/eu.po
	@echo "   eu_ES ...Basque..."
	@msgfmt src/messages/eu.po -o trans/eu.mo

trans/fi.mo:	src/messages/fi.po
	@echo "   fi_FI ...Finnish..."
	@msgfmt src/messages/fi.po -o trans/fi.mo

trans/fr.mo:	src/messages/fr.po
	@echo "   fr_FR ...French..."
	@msgfmt src/messages/fr.po -o trans/fr.mo

trans/he.mo:	src/messages/he.po
	@echo "   he_IL ...Hebrew..."
	@msgfmt src/messages/he.po -o trans/he.mo

trans/hu.mo:	src/messages/hu.po
	@echo "   hu_HU ...Hungarian..."
	@msgfmt src/messages/hu.po -o trans/hu.mo

trans/id.mo:	src/messages/id.po
	@echo "   id_ID ...Indonesian..."
	@msgfmt src/messages/id.po -o trans/id.mo

trans/is.mo:	src/messages/is.po
	@echo "   is_IS ...Icelandic..."
	@msgfmt src/messages/is.po -o trans/is.mo

trans/it.mo:	src/messages/it.po
	@echo "   it_IT ...Italian..."
	@msgfmt src/messages/it.po -o trans/it.mo

trans/ja.mo:	src/messages/ja.po
	@echo "   ja_JP ...Japanese..."
	@msgfmt src/messages/ja.po -o trans/ja.mo

trans/ko.mo:	src/messages/ko.po
	@echo "   ko_KR ...Korean..."
	@msgfmt src/messages/ko.po -o trans/ko.mo

trans/lt.mo:	src/messages/lt.po
	@echo "   lt_LT ...Lithuanian..."
	@msgfmt src/messages/lt.po -o trans/lt.mo

trans/ms.mo:	src/messages/ms.po
	@echo "   ms_MY ...Malay..."
	@msgfmt src/messages/ms.po -o trans/ms.mo

trans/nl.mo:	src/messages/nl.po
	@echo "   nl_NL ...Dutch..."
	@msgfmt src/messages/nl.po -o trans/nl.mo

trans/nb.mo:	src/messages/nb.po
	@echo "   nb_NO ...Norwegian Bokmal..."
	@msgfmt src/messages/nb.po -o trans/nb.mo

trans/nn.mo:	src/messages/nn.po
	@echo "   nn_NO ...Norwegian Nynorsk..."
	@msgfmt src/messages/nn.po -o trans/nn.mo

trans/pl.mo:	src/messages/pl.po
	@echo "   pl_PL ...Polish..."
	@msgfmt src/messages/pl.po -o trans/pl.mo

trans/pt.mo:	src/messages/pt.po
	@echo "   pt_PT ...Portuguese..."
	@msgfmt src/messages/pt.po -o trans/pt.mo

trans/pt_br.mo:	src/messages/pt_br.po
	@echo "   pt_BR ...Brazilian Portuguese..."
	@msgfmt src/messages/pt_br.po -o trans/pt_br.mo

trans/ro.mo:	src/messages/ro.po
	@echo "   ro_RO ...Romanian..."
	@msgfmt src/messages/ro.po -o trans/ro.mo

trans/ru.mo:	src/messages/ru.po
	@echo "   ru_RU ...Russian..."
	@msgfmt src/messages/ru.po -o trans/ru.mo

trans/sk.mo:	src/messages/sk.po
	@echo "   sk_SK ...Slovak..."
	@msgfmt src/messages/sk.po -o trans/sk.mo

trans/sr.mo:	src/messages/sr.po
	@echo "   sr_YU ...Serbian..."
	@msgfmt src/messages/sr.po -o trans/sr.mo

trans/sv.mo:	src/messages/sv.po
	@echo "   sv_SE ...Swedish..."
	@msgfmt src/messages/sv.po -o trans/sv.mo

trans/ta.mo:	src/messages/ta.po
	@echo "   ta_IN ...Tamil..."
	@msgfmt src/messages/ta.po -o trans/ta.mo

trans/tr.mo:	src/messages/tr.po
	@echo "   tr_TR ...Turkish..."
	@msgfmt src/messages/tr.po -o trans/tr.mo

trans/wa.mo:	src/messages/wa.po
	@echo "   wa_BE ...Walloon..."
	@msgfmt src/messages/wa.po -o trans/wa.mo

trans/zh_cn.mo:	src/messages/zh_cn.po
	@echo "   zh_CN ...Chinese..."
	@msgfmt src/messages/zh_cn.po -o trans/zh_cn.mo



# Make the "obj" directory to throw the object(s) into:

obj:
	@mkdir obj

