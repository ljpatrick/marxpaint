/*
  tools.h

  For Tux Paint
  List of available tools.

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 14, 2002 - December 10, 2002
*/


#include "tip_tux.h"


/* What tools are available: */

enum {
  TOOL_BRUSH,
  TOOL_STAMP,
  TOOL_LINES,
  TOOL_SHAPES,
  TOOL_TEXT,
  TOOL_MAGIC,
  TOOL_UNDO,
  TOOL_REDO,
  TOOL_ERASER,
  TOOL_NEW,
  TOOL_OPEN,
  TOOL_SAVE,
  TOOL_PRINT,
  TOOL_QUIT,
  NUM_TOOLS
};


/* Tool names: */

char * tool_names[NUM_TOOLS] = {
  gettext_noop("Paint"),
  gettext_noop("Stamp"),
  gettext_noop("Lines"),
  gettext_noop("Shapes"),
  gettext_noop("Text"),
  gettext_noop("Magic"),
  gettext_noop("Undo"),
  gettext_noop("Redo"),
  gettext_noop("Eraser"),
  gettext_noop("New"),
  gettext_noop("Open"),
  gettext_noop("Save"),
  gettext_noop("Print"),
  gettext_noop("Quit")
};


/* Some text to write when each tool is selected: */

char * tool_tips[NUM_TOOLS] = {
  gettext_noop("Pick a color and a brush shape to draw with."),
  gettext_noop("Pick a picture to stamp around your drawing."),
  gettext_noop("Click to start drawing a line.  Let go to complete it."),
  gettext_noop("Pick a shape. Click to pick the center, drag, then let go when it is the size you want.  Move around to rotate it, and click to draw it."),
  gettext_noop("Choose a style of text.  Click on your drawing and you can start typing."),
  gettext_noop("Pick a magical effect to use on your drawing!"),
  /* Undo */ gettext_noop("Undo!"),
  /* Redo */ gettext_noop("Redo!"),
  /* Eraser */ gettext_noop("Eraser!"),
  /* New */ gettext_noop("You now have a blank sheet to draw on!"),
  /* Open */ gettext_noop("Open..."),
  /* Save */ gettext_noop("Your image has been saved!"),
  /* Print */ gettext_noop("Printing..."),
  /* Quit */ gettext_noop("Bye bye!")
};

#define TIP_LINE_START gettext_noop("Let go of the button to complete the line.")
#define TIP_SHAPE_START gettext_noop("Hold the button to stretch the shape.")
#define TIP_SHAPE_NEXT gettext_noop("Move the mouse to rotate the shape. Click to draw it.")
#define TIP_NEW_ABORT gettext_noop("Ok then... Let's keep drawing this one!")


/* Tool icon filenames: */

char * tool_img_fnames[NUM_TOOLS] = {
  DATA_PREFIX "images/tools/brush.png",
  DATA_PREFIX "images/tools/stamp.png",
  DATA_PREFIX "images/tools/lines.png",
  DATA_PREFIX "images/tools/shapes.png",
  DATA_PREFIX "images/tools/text.png",
  DATA_PREFIX "images/tools/magic.png",
  DATA_PREFIX "images/tools/undo.png",
  DATA_PREFIX "images/tools/redo.png",
  DATA_PREFIX "images/tools/eraser.png",
  DATA_PREFIX "images/tools/new.png",
  DATA_PREFIX "images/tools/open.png",
  DATA_PREFIX "images/tools/save.png",
  DATA_PREFIX "images/tools/print.png",
  DATA_PREFIX "images/tools/quit.png"
};


/* Tux icons to use: */

int tool_tux[NUM_TOOLS] = {
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_OOPS,
  TUX_WAIT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_DEFAULT,
  TUX_GREAT,
  TUX_GREAT,
  TUX_DEFAULT
};

