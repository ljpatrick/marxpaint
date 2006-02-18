/*
  shapes.h

  For Tux Paint
  List of available shapes.

  Copyright (c) 2002-2006 by Bill Kendrick and others
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
  (See COPYING.txt)

  June 14, 2002 - February 17, 2006
  $Id$
*/



/* What shapes are available: */

enum {
  SHAPE_SQUARE,
  SHAPE_SQUARE_FILL,
  SHAPE_RECTANGLE,
  SHAPE_RECTANGLE_FILL,
  SHAPE_CIRCLE,
  SHAPE_CIRCLE_FILL,
  SHAPE_ELLIPSE,
  SHAPE_ELLIPSE_FILL,
  SHAPE_TRIANGLE,
  SHAPE_TRIANGLE_FILL,
  SHAPE_PENTAGON,
  SHAPE_PENTAGON_FILL,
  SHAPE_RHOMBUS,
  SHAPE_RHOMBUS_FILL,
  NUM_SHAPES
};


/* How many sides do they have? */

const int shape_sides[NUM_SHAPES] = {
  4,   /* Square */
  4,   /* Square */
  4,   /* Rectangle */
  4,   /* Rectangle */
  72,  /* Circle */
  72,  /* Circle */
  72,  /* Ellipse */
  72,  /* Ellipse */
  3,   /* Triangle */
  3,   /* Triangle */
  5,   /* Pentagon */
  5,   /* Pentagon */
  4,   /* Rhombus */
  4    /* Rhombus */
};


/* Which shapes are 1:1 aspect? */

const int shape_locked[NUM_SHAPES] = {
  1,   /* Square */
  1,   /* Square */
  0,   /* Rectangle */
  0,   /* Rectangle */
  1,   /* Circle */
  1,   /* Circle */
  0,   /* Ellipse */
  0,   /* Ellipse */
  0,   /* Triangle */
  0,   /* Triangle */
  0,   /* Pentagon */
  0,   /* Pentagon */
  0,   /* Rhombus */
  0    /* Rhombus */
};


/* Which shapes are filled? */

const int shape_filled[NUM_SHAPES] = {
  0,   /* Square */
  1,   /* Square */
  0,   /* Rectangle */
  1,   /* Rectangle */
  0,   /* Circle */
  1,   /* Circle */
  0,   /* Ellipse */
  1,   /* Ellipse */
  0,   /* Triangle */
  1,   /* Triangle */
  0,   /* Pentagon */
  1,   /* Pentagon */
  0,   /* Rhombus */
  1    /* Rhombus */
};



/* Initial angles for shapes: */

const int shape_init_ang[NUM_SHAPES] = {
  45,  /* Square */
  45,  /* Square */
  45,  /* Rectangle */
  45,  /* Rectangle */
  0,   /* Circle */
  0,   /* Circle */
  0,   /* Ellipse */
  0,   /* Ellipse */
  210, /* Triangle */
  210, /* Triangle */
  162, /* Pentagon */
  162, /* Pentagon */
  0,   /* Rhombus */
  0    /* Rhombus */
};


/* Shapes that don't make sense rotating (e.g., circles): */

const int shape_no_rotate[NUM_SHAPES] = {
  0,  /* Square */
  0,  /* Square */
  0,  /* Rectangle */
  0,  /* Rectangle */
  1,  /* Circle */
  1,  /* Circle */
  0,  /* Ellipse */
  0,  /* Ellipse */
  0,  /* Triangle */
  0,  /* Triangle */
  0,  /* Pentagon */
  0,  /* Pentagon */
  0,  /* Rhombus */
  0   /* Rhombus */
};


/* Shape names: */

const char * const shape_names[NUM_SHAPES] = {
  gettext_noop("Square"),
  gettext_noop("Square"),
  gettext_noop("Rectangle"),
  gettext_noop("Rectangle"),
  gettext_noop("Circle"),
  gettext_noop("Circle"),
  gettext_noop("Ellipse"),
  gettext_noop("Ellipse"),
  gettext_noop("Triangle"),
  gettext_noop("Triangle"),
  gettext_noop("Pentagon"),
  gettext_noop("Pentagon"),
  gettext_noop("Rhombus"),
  gettext_noop("Rhombus")
};


/* Some text to write when each shape is selected: */

const char * const shape_tips[NUM_SHAPES] = {
  gettext_noop("A square is a rectangle with four equal sides."),
  gettext_noop("A square is a rectangle with four equal sides."),
  gettext_noop("A rectangle has four sides and four right angles."),
  gettext_noop("A rectangle has four sides and four right angles."),
  gettext_noop("A circle is a curve where all points have the same distance from the centre."),
  gettext_noop("A circle is a curve where all points have the same distance from the centre."),
  gettext_noop("An ellipse is a stretched circle."),
  gettext_noop("An ellipse is a stretched circle."),
  gettext_noop("A triangle has three sides."),
  gettext_noop("A triangle has three sides."),
  gettext_noop("A pentagon has five sides."),
  gettext_noop("A pentagon has five sides."),
  gettext_noop("A rhombus has four equal sides, and opposite sides are parallel."),
  gettext_noop("A rhombus has four equal sides, and opposite sides are parallel.")
};


/* Shape icon filenames: */

const char * const shape_img_fnames[NUM_SHAPES] = {
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

