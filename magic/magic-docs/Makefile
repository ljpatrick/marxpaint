# Makefile for Tux Paint Magic Tool Plugin API docs
# 
# Uses "links" to convert docs from HTML to plain text.
# (Normally only ran by the developers after updating the HTML, prior to
# release.)
#
# Bill Kendrick
# bill@newbreedsoftware.com
# 
# August 2, 2007 - August 2, 2007
# $Id$

LINKS=links -dump -no-numbering -no-references

TXT_FILES= \
	blocks.txt \
	blur.txt \
	bricks.txt \
	cartoon.txt \
	chalk.txt \
	darken.txt \
	drip.txt \
	emboss.txt \
	fade.txt \
	fill.txt \
	flip.txt \
	flower.txt \
	foam.txt \
	glasstile.txt \
	grass.txt \
	kalidescope.txt \
	metalpaint.txt \
	mirror.txt \
	negative.txt \
	rainbow.txt \
	ripples.txt \
	smudge.txt \
	sparkles.txt \
	tint.txt \
	waves.txt

all:	$(TXT_FILES)

clean:
	@-rm $(TXT_FILES)

blocks.txt: html/blocks.html
	@$(LINKS) $< > $@

blur.txt: html/blur.html
	@$(LINKS) $< > $@

bricks.txt: html/bricks.html
	@$(LINKS) $< > $@

cartoon.txt: html/cartoon.html
	@$(LINKS) $< > $@

chalk.txt: html/chalk.html
	@$(LINKS) $< > $@

darken.txt: html/darken.html
	@$(LINKS) $< > $@

drip.txt: html/drip.html
	@$(LINKS) $< > $@

emboss.txt: html/emboss.html
	@$(LINKS) $< > $@

fade.txt: html/fade.html
	@$(LINKS) $< > $@

fill.txt: html/fill.html
	@$(LINKS) $< > $@

flip.txt: html/flip.html
	@$(LINKS) $< > $@

flower.txt: html/flower.html
	@$(LINKS) $< > $@

foam.txt: html/foam.html
	@$(LINKS) $< > $@

glasstile.txt: html/glasstile.html
	@$(LINKS) $< > $@

grass.txt: html/grass.html
	@$(LINKS) $< > $@

kalidescope.txt: html/kalidescope.html
	@$(LINKS) $< > $@

metalpaint.txt: html/metalpaint.html
	@$(LINKS) $< > $@

mirror.txt: html/mirror.html
	@$(LINKS) $< > $@

negative.txt: html/negative.html
	@$(LINKS) $< > $@

rainbow.txt: html/rainbow.html
	@$(LINKS) $< > $@

ripples.txt: html/ripples.html
	@$(LINKS) $< > $@

smudge.txt: html/smudge.html
	@$(LINKS) $< > $@

sparkles.txt: html/sparkles.html
	@$(LINKS) $< > $@

tint.txt: html/tint.html
	@$(LINKS) $< > $@

waves.txt: html/waves.html
	@$(LINKS) $< > $@
