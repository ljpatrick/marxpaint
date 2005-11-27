/*
  titles.h

  For Tux Paint
  List of available titles

  Copyright (c) 2004 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - September 14, 2004
  $Id$
*/



/* What titles are available: */

enum {
  TITLE_NONE,
  TITLE_NOCOLORS,
  TITLE_TOOLS,
  TITLE_COLORS,
  TITLE_BRUSHES,
  TITLE_ERASERS,
  TITLE_STAMPS,
  TITLE_SHAPES,
  TITLE_LETTERS,
  TITLE_MAGIC,
  NUM_TITLES
};


/* Title names: */

const char * const title_names[NUM_TITLES] = {
  "",
  "",
  gettext_noop("Tools"),
  gettext_noop("Colors"),
  gettext_noop("Brushes"),
  gettext_noop("Erasers"),
  gettext_noop("Stamps"),
  gettext_noop("Shapes"),
  gettext_noop("Letters"),
  gettext_noop("Magic")
};

