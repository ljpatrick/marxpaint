/*
  tip_tux.h

  For Tux Paint
  List of tux images for tips.

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 17, 2002 - June 27, 2002
*/


#ifndef TIP_TUX_H
#define TIP_TUX_H

/* What tuxes are available: */

enum {
  TUX_DEFAULT,
  TUX_KISS,
  TUX_BORED,
  TUX_GREAT,
  TUX_OOPS,
  TUX_WAIT,
  NUM_TIP_TUX
};


/* Tux filenames: */

const char * const tux_img_fnames[NUM_TIP_TUX] = {
  DATA_PREFIX "images/tux/default.png",
  DATA_PREFIX "images/tux/kiss.png",
  DATA_PREFIX "images/tux/bored.png",
  DATA_PREFIX "images/tux/great.png",
  DATA_PREFIX "images/tux/oops.png",
  DATA_PREFIX "images/tux/wait.png"
};

#endif /* TIP_TUX_H */
