# Makefile for Tux Paint docs
# 
# Uses "links" to convert docs from HTML to plain text.
# (Normally only ran by the developers after updating the HTML, prior to
# release.)
#
# Bill Kendrick
# bill@newbreedsoftware.com
# 
# Sept. 4, 2005 - March 28, 2008
# $Id$

# Bah, "-no-numbering" and "-no-references" went away recently!? -bjk 2008.04.28
LINKS_OPTIONS:=-dump
LINKS:=links $(LINKS_OPTIONS)

LYNX_OPTIONS:=-dump
LYNX:=lynx $(LYNX_OPTIONS)

# FIXME: Support finding files in ??/html/ and ??_??/html/)
HTMLFILES:=$(wildcard html/*.html)
TEXTFILES:=$(patsubst html/%.html,%.txt,$(HTMLFILES))

JA_HTMLFILES:=$(wildcard ja/html/*.html)
JA_TEXTFILES:=$(patsubst ja/html/%.html,ja/%.txt,$(JA_HTMLFILES))

.PHONY: all
all:	$(TEXTFILES) \
	$(JA_TEXTFILES)

.PHONY: clean
clean:
	-rm $(TEXTFILES)
	-rm $(JA_TEXTFILES)

$(TEXTFILES): %.txt: html/%.html  
	$(LINKS) $< > $@

$(JA_TEXTFILES): ja/%.txt: ja/html/%.html  
	$(LYNX) $< > $@

