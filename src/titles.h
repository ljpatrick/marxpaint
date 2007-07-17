/*
  titles.h

  For Tux Paint
  List of available titles

  Copyright (c) 2002-2007 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.tuxpaint.org/

  June 14, 2002 - July 17, 2007
  $Id$
*/



/* What titles are available: */

enum
{
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

const char *const title_names[NUM_TITLES] = {
  "",
  "",
  // Title of tool selector (buttons down the left)
  gettext_noop("Tools"),

  // Title of color palette (buttons across the bottom)
  gettext_noop("Colors"),

  // Title of brush selector (buttons down the right for paint and line tools)
  gettext_noop("Brushes"),

  // Title of eraser selector (buttons down the right for eraser tool)
  gettext_noop("Erasers"),

  // Title of stamp selector (buttons down the right for stamps tool)
  gettext_noop("Stamps"),

  // Title of shape selector (buttons down the right for shapes tool)
  gettext_noop("Shapes"),

  // Title of font selector (buttons down the right for text tool)
  gettext_noop("Letters"),

  // Title of magic tool selector (buttons down the right for magic (effect plugin) tool)
  gettext_noop("Magic")
};
