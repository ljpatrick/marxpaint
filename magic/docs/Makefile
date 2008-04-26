# Makefile for Tux Paint Magic Tool Plugin API docs
# 
# Uses "links" to convert docs from HTML to plain text.
# (Normally only ran by the developers after updating the HTML, prior to
# release.)
#
# Bill Kendrick
# bill@newbreedsoftware.com
# 
# July 6, 2007 - July 6, 2007
# $Id$

LINKS:=links -dump -no-numbering -no-references

all:	README.txt

clean:
	-rm README.txt

README.txt:	html/README.html
	$(LINKS) $< > $@
