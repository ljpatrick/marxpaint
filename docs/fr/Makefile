# Makefile for Tux Paint docs
# 
# Uses "links" to convert docs from HTML to plain text.
# (Normally only ran by the developers after updating the HTML, prior to
# release.)
#
# Bill Kendrick
# bill@newbreedsoftware.com
# 
# Sept. 4, 2005 - October 24, 2005

LINKS:=links -dump -no-numbering -no-references

all:	README1.txt README2.txt OPTIONS.txt FAQ.txt PNG.txt

clean:
	-rm README1.txt
	-rm README2.txt
	-rm OPTIONS.txt
	-rm FAQ.txt
	-rm PNG.txt

README1.txt:	html/README1.html
	$(LINKS) $< > $@

README2.txt:	html/README2.html
	$(LINKS) $< > $@

OPTIONS.txt:	html/OPTIONS.html
	$(LINKS) $< > $@

FAQ.txt:	html/FAQ.html
	$(LINKS) $< > $@

PNG.txt:	html/PNG.html
	$(LINKS) $< > $@

