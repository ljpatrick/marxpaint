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

.PHONY: all
all:	README.txt OPTIONS.txt FAQ.txt EXTENDING.txt ADVANCED-STAMPS-HOWTO.txt

.PHONY: clean
clean:
	-rm README.txt
	-rm OPTIONS.txt
	-rm FAQ.txt
	-rm EXTENDING.txt
	-rm ADVANCED-STAMPS-HOWTO.txt

README.txt:	html/README.html
	$(LINKS) $< > $@

OPTIONS.txt:	html/OPTIONS.html
	$(LINKS) $< > $@

FAQ.txt:	html/FAQ.html
	$(LINKS) $< > $@

EXTENDING.txt:	html/EXTENDING.html
	$(LINKS) $< > $@

ADVANCED-STAMPS-HOWTO.txt:	html/ADVANCED-STAMPS-HOWTO.html
	$(LINKS) $< > $@


