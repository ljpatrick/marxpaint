/*
  shapes.h

  For Tux Paint
  List of available shapes.

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - January 8, 2003
*/



/* What shapes are available: */

enum {
  SHAPE_SQUARE,
  SHAPE_SQUARE_FILL,
  SHAPE_RECTANGLE,
  SHAPE_RECTANGLE_FILL,
  SHAPE_CIRCLE,
  SHAPE_CIRCLE_FILL,
  SHAPE_OVAL,
  SHAPE_OVAL_FILL,
  SHAPE_TRIANGLE,
  SHAPE_TRIANGLE_FILL,
  SHAPE_PENTAGON,
  SHAPE_PENTAGON_FILL,
  SHAPE_DIAMOND,
  SHAPE_DIAMOND_FILL,
  NUM_SHAPES
};


/* How many sides do they have? */

int shape_sides[NUM_SHAPES] = {
  4,   /* Square */
  4,   /* Square */
  4,   /* Rectangle */
  4,   /* Rectangle */
  72,  /* Circle */
  72,  /* Circle */
  72,  /* Oval */
  72,  /* Oval */
  3,   /* Triangle */
  3,   /* Triangle */
  5,   /* Pentagon */
  5,   /* Pentagon */
  4,   /* Diamond */
  4    /* Diamond */
};


/* Which shapes are 1:1 aspect? */

int shape_locked[NUM_SHAPES] = {
  1,   /* Square */
  1,   /* Square */
  0,   /* Rectangle */
  0,   /* Rectangle */
  1,   /* Circle */
  1,   /* Circle */
  0,   /* Oval */
  0,   /* Oval */
  0,   /* Triangle */
  0,   /* Triangle */
  0,   /* Pentagon */
  0,   /* Pentagon */
  0,   /* Diamond */
  0    /* Diamond */
};


/* Which shapes are filled? */

int shape_filled[NUM_SHAPES] = {
  0,   /* Square */
  1,   /* Square */
  0,   /* Rectangle */
  1,   /* Rectangle */
  0,   /* Circle */
  1,   /* Circle */
  0,   /* Oval */
  1,   /* Oval */
  0,   /* Triangle */
  1,   /* Triangle */
  0,   /* Pentagon */
  1,   /* Pentagon */
  0,   /* Diamond */
  1    /* Diamond */
};



/* Initial angles for shapes: */

int shape_init_ang[NUM_SHAPES] = {
  45,  /* Square */
  45,  /* Square */
  45,  /* Rectangle */
  45,  /* Rectangle */
  0,   /* Circle */
  0,   /* Circle */
  0,   /* Oval */
  0,   /* Oval */
  210, /* Triangle */
  210, /* Triangle */
  162, /* Pentagon */
  162, /* Pentagon */
  0,   /* Diamond */
  0    /* Diamond */
};


/* Shapes that don't make sense rotating (e.g., circles): */

int shape_no_rotate[NUM_SHAPES] = {
  0,  /* Square */
  0,  /* Square */
  0,  /* Rectangle */
  0,  /* Rectangle */
  1,  /* Circle */
  1,  /* Circle */
  0,  /* Oval */
  0,  /* Oval */
  0,  /* Triangle */
  0,  /* Triangle */
  0,  /* Pentagon */
  0,  /* Pentagon */
  0,  /* Diamond */
  0   /* Diamond */
};


/* Shape names: */

char * shape_names[NUM_SHAPES] = {
  gettext_noop("Square"),
  gettext_noop("Square"),
  gettext_noop("Rectangle"),
  gettext_noop("Rectangle"),
  gettext_noop("Circle"),
  gettext_noop("Circle"),
  gettext_noop("Oval"),
  gettext_noop("Oval"),
  gettext_noop("Triangle"),
  gettext_noop("Triangle"),
  gettext_noop("Pentagon"),
  gettext_noop("Pentagon"),
  gettext_noop("Diamond"),
  gettext_noop("Diamond")
};


/* Some text to write when each shape is selected: */

char * shape_tips[NUM_SHAPES] = {
  gettext_noop("A square has four sides, each the same length."),
  gettext_noop("A square has four sides, each the same length."),
  gettext_noop("A rectangle has four sides."),
  gettext_noop("A rectangle has four sides."),
  gettext_noop("A circle is exactly round."),
  gettext_noop("A circle is exactly round."),
  gettext_noop("Oval"),
  gettext_noop("Oval"),
  gettext_noop("A triangle has three sides."),
  gettext_noop("A triangle has three sides."),
  gettext_noop("A pentagon has five sides."),
  gettext_noop("A pentagon has five sides."),
  gettext_noop("A diamond is a square, turned around slightly."),
  gettext_noop("A diamond is a square, turned around slightly.")
};


/* Shape icon filenames: */

char * shape_img_fnames[NUM_SHAPES] = {
  DATA_PREFIX "images/shapes/square.png",
  DATA_PREFIX "images/shapes/square_f.png",
  DATA_PREFIX "images/shapes/rectangle.png",
  DATA_PREFIX "images/shapes/rectangle_f.png",
  DATA_PREFIX "images/shapes/circle.png",
  DATA_PREFIX "images/shapes/circle_f.png",
  DATA_PREFIX "images/shapes/oval.png",
  DATA_PREFIX "images/shapes/oval_f.png",
  DATA_PREFIX "images/shapes/triangle.png",
  DATA_PREFIX "images/shapes/triangle_f.png",
  DATA_PREFIX "images/shapes/pentagon.png",
  DATA_PREFIX "images/shapes/pentagon_f.png",
  DATA_PREFIX "images/shapes/diamond.png",
  DATA_PREFIX "images/shapes/diamond_f.png"
};

