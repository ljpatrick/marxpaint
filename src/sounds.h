/*
  sounds.h

  For Tux Paint
  List of sound effects.

  Copyright (c) 2002 by Bill Kendrick
  bill@newbreedsoftware.com
  http://www.newbreedsoftware.com/tuxpaint/

  June 15, 2002 - December 12, 2004
*/



/* Sounds available: */

enum {
  SND_HARP,          /* Begin / New */
  SND_CLICK,         /* Tool selections */
  SND_BLEEP,         /* Selector selection */
  SND_BUBBLE,        /* Color selection */
  SND_STAMP,         /* Using stamp tool */
  SND_LINE_START,    /* Using line tool */
  SND_LINE_END,
  SND_SCROLL,        /* Selector scroll buttons */
  SND_PAINT1,        /* Sound while painting */
  SND_PAINT2,
  SND_PAINT3,
  SND_PAINT4,
  SND_ERASER1,       /* Sound while erasing */
  SND_ERASER2,
  SND_SAVE,          /* Save sound effect */
  SND_PROMPT,        /* Prompt animation sound effect */
  SND_DRIP,          /* Magic drip */
  SND_CHALK,         /* Magic chalk */
  SND_SPARKLES1,     /* Magic sparkles */
  SND_SPARKLES2,
  SND_THICK,         /* Magic thick */
  SND_THIN,          /* Magic thin */
  SND_FLIP,          /* Magic flip */
  SND_MIRROR,        /* Magic mirror */
  SND_NEGATIVE,      /* Magic negative */
  SND_BLUR,          /* Magic blur */
  SND_BLOCKS,        /* Magic blocks */
  SND_FADE,          /* Magic fade */
  SND_DARKEN,        /* Magic darken */
  SND_RAINBOW,       /* Magic rainbow */
  SND_SMUDGE,        /* Magic smudge */
  SND_TINT,          /* Magic tint */
  SND_CARTOON,       /* Magic cartoon */
  SND_KEYCLICK,      /* Text tool keyboard click feedback */
  SND_RETURN,        /* Text tool carriage return sound */
  SND_SHRINK,        /* Stamp shrink */
  SND_GROW,          /* Stamp grow */
  NUM_SOUNDS
};


/* Sound file filenames: */

const char * const sound_fnames[NUM_SOUNDS] = {
  DATA_PREFIX "sounds/harp.wav",
  DATA_PREFIX "sounds/click.wav",
  DATA_PREFIX "sounds/bleep.wav",
  DATA_PREFIX "sounds/bubble.wav",
  DATA_PREFIX "sounds/stamp.wav",
  DATA_PREFIX "sounds/line_start.wav",
  DATA_PREFIX "sounds/line_end.wav",
  DATA_PREFIX "sounds/scroll.wav",
  DATA_PREFIX "sounds/paint1.wav",
  DATA_PREFIX "sounds/paint2.wav",
  DATA_PREFIX "sounds/paint3.wav",
  DATA_PREFIX "sounds/paint4.wav",
  DATA_PREFIX "sounds/eraser1.wav",
  DATA_PREFIX "sounds/eraser2.wav",
  DATA_PREFIX "sounds/save.wav",
  DATA_PREFIX "sounds/prompt.wav",
  DATA_PREFIX "sounds/drip.wav",
  DATA_PREFIX "sounds/chalk.wav",
  DATA_PREFIX "sounds/sparkles1.wav",
  DATA_PREFIX "sounds/sparkles2.wav",
  DATA_PREFIX "sounds/thick.wav",
  DATA_PREFIX "sounds/thin.wav",
  DATA_PREFIX "sounds/flip.wav",
  DATA_PREFIX "sounds/mirror.wav",
  DATA_PREFIX "sounds/negative.wav",
  DATA_PREFIX "sounds/blur.wav",
  DATA_PREFIX "sounds/blocks.wav",
  DATA_PREFIX "sounds/fade.wav",
  DATA_PREFIX "sounds/darken.wav",
  DATA_PREFIX "sounds/rainbow.wav",
  DATA_PREFIX "sounds/smudge.wav",
  DATA_PREFIX "sounds/tint.wav",
  DATA_PREFIX "sounds/cartoon.wav",
  DATA_PREFIX "sounds/keyclick.wav",
  DATA_PREFIX "sounds/return.wav",
  DATA_PREFIX "sounds/shrink.wav",
  DATA_PREFIX "sounds/grow.wav"
};

