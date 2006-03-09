# Makefile for Tux Paint docs
# 
# Uses "links" to convert docs from HTML to plain text.
# (Normally only ran by the developers after updating the HTML, prior to
# release.)
#
# Bill Kendrick
# bill@newbreedsoftware.com
# 
# Sept. 4, 2005 - March 8, 2006
# $Id$

LINKS=links -dump -no-numbering -no-references

all:	README.txt OPTIONS.txt FAQ.txt EXTENDING.txt ADVANCED-STAMPS-HOWTO.txt

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


