/*
  colors.h

  For Tux Paint
  List of colors

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - November 21, 2004
*/



/* What colors are available: */

enum {
  COLOR_BLACK,
  COLOR_DARKGRAY,
  COLOR_LIGHTGREY,
  COLOR_WHITE,
  COLOR_RED,
  COLOR_ORANGE,
  COLOR_YELLOW,
  COLOR_LIGHTGREEN,
  COLOR_DARKGREEN,
  COLOR_SKYBLUE,
  COLOR_BLUE,
  COLOR_LAVENDER,
  COLOR_PURPLE,
  COLOR_PINK,
  COLOR_BROWN,
  COLOR_TAN,
  COLOR_BEIGE,
  NUM_COLORS
};


/* Hex codes: */

const int color_hexes[NUM_COLORS][3] = {
  {0, 0, 0},       /* Black */
  {128, 128, 128}, /* Dark Gray */
  {192, 192, 192}, /* Light Grey */
  {255, 255, 255}, /* White */
  {255,   0,   0}, /* Red */
  {255, 128,   0}, /* Orange */
  {255, 255,   0}, /* Yellow */
  {160, 228, 128}, /* Light green */
  { 33, 148,  70}, /* Dark green */
  {138, 168, 205}, /* Sky blue */
  { 50, 100, 255}, /* Blue */
  {186, 157, 255}, /* Lavender */
  {128,   0, 128}, /* Purple */
  {255, 165, 211}, /* Pink */
  {128,  80,   0}, /* Brown */
  {226, 189, 166}, /* Tan */
  {247, 228, 219}  /* Beige */
};


/* Color names: */

const char * const color_names[NUM_COLORS] = {
  gettext_noop("Black!"),
  gettext_noop("Dark grey! Sometimes, people spell it \"Dark gray\"."),
  gettext_noop("Light gray! Sometimes, people spell it \"Dark grey\"."),
  gettext_noop("White!"),
  gettext_noop("Red!"),
  gettext_noop("Orange!"),
  gettext_noop("Yellow!"),
  gettext_noop("Light green!"),
  gettext_noop("Dark green!"),
  gettext_noop("Sky blue!"),
  gettext_noop("Blue!"),
  gettext_noop("Lavender!"),
  gettext_noop("Purple!"),
  gettext_noop("Pink!"),
  gettext_noop("Brown!"),
  gettext_noop("Tan!"),
  gettext_noop("Beige!")
};
