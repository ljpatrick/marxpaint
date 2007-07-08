                     Creating Tux Paint Magic Tool Plugins

                Copyright 2007-2007 by Bill Kendrick and others
                               New Breed Software

                           bill@newbreedsoftware.com
                            http://www.tuxpaint.org/

                          July 5, 2007 - July 6, 2007

   --------------------------------------------------------------------------

Overview

     Beginning with version 0.9.18, Tux Paint's 'Magic' tools were converted
     from routines that lived within the application itself, to a set of
     'plugins' that are loaded when Tux Paint starts up.

     This division allows more rapid development of 'Magic' tools, and allows
     programmers to create and test new tools without needing to integrate
     them within the main Tux Paint source code. (Users of more professional
     graphics tools, such as The GIMP, should be familiar with this plugin
     concept.)

   --------------------------------------------------------------------------

Prerequisites

     Tux Paint is written in the C programming language, and uses the
     Simple DirectMedia Layer library ('libSDL', or simply 'SDL'). Therefore,
     for the moment at least, one must understand the C language, how to
     compile C-based programs. Familiarity with the SDL API is highly
     recommended, but some basic SDL concepts will be covered in this
     document.

   --------------------------------------------------------------------------

Interfaces

     Those who create 'Magic' tool plugins for Tux Paint must provide some
     interfaces (C functions) that Tux Paint may invoke.

     Tux Paint utilizes SDL's "SDL_LoadObject()" and "SDL_LoadFunction()"
     routines to load plugins (shared objects files; e.g., ".so" files on
     Linux or ".dll" files on Windows) and find the functions within.

     In turn, Tux Paint provides a number of helper functions that the plugin
     may (or sometimes should) use. This is exposed as a C structure
     (containing pointers to functions inside Tux Paint and other data) that
     gets passed along to the plugin's functions as an argument.

     Plugins should #include the file "tp_magic_api.h", and compiler flags
     which should be used when building plugins (to find the aforementioned
     header file, as well as SDL's header files) can be acquired by invoking
     the tool "tp-magic-config".

     (These are included with Tux Paint -- or in some cases, as part of a
     "Tux Paint 'Magic' Tool Plugin Development package".)

  'Magic' tool plugin functions

       'Magic' tool plugins must provide the functions listed below. Note: To
       avoid namespace collisions, each function's name must start with the
       shared object's filename (e.g., "blur.so" or "blur.dll" would have
       functions whose names begin with "blur_").

    Common arguments to plugin functions:

         * magic_api * api
           Pointer to the struct containing pointers to Tux Paint functions
           and other data (see below)
         * int which
           An index the plugin should use to differentiate different 'Magic'
           tools, if the plugin provides more than one. (If not, "which" will
           always be 0.)
         * SDL_Surface * canvas
           The active Tux Paint drawing canvas. Your magical effects should
           end up here!
         * SDL_Surface * last
           A snapshot of the previous Tux Paint canvas, taken when the the
           mouse was first clicked to activate the magic tool. If you don't
           continuously affect the image during one hold of the mouse button,
           you should base your effects off the contents of this canvas.

    Required plugin functions:

         * int get_tool_count(magic_api * api)
           This should return the number of Magic tools this plugin provides
           to Tux Paint.
         * char * get_name(magic_api * api, int which)
           This should return a string containing the name of a magic tool.
           This will appear on the button in the 'Magic' selector within
           Tux Paint. Tux Paint will free() the string upon exit, so you
           should wrap it in a C strdup() call.
         * SDL_Surface * get_icon(magic_api * api, int which)
           This should return an SDL_Surface containing the icon representing
           the tool. (A greyscale image with alpha, no larger than 40x40.)
           This will appear on the button in the 'Magic' selector within
           Tux Paint.
           Tux Paint will SDL_FreeSurface() the surface upon exit.
         * char * get_description(magic_api * api, int which)
           This should return a string containing the description of a magic
           tool. This will appear as a help tip, explained by Tux the
           Penguin, within Tux Paint.
           Tux Paint will free() the string upon exit, so you should wrap it
           in a C strdup() call.
         * int requires_colors(magic_api * api, int which)
           Return a '1' if the 'Magic' tool accepts colors (the 'Colors'
           palette in Tux Paint will be available), or '0' if not.
         * void set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 g)
           Tux Paint will call this function to inform the plugin of the RGB
           values of the currently-selected color in Tux Paint's 'Colors'
           palette. (It will be called whenever one of the plguin's Magic
           tools that accept colors becomes active, or the user picks a new
           color while such a tool is currently active.)
         * Uint32 api_version(void)
           The plugin should return an integer value representing the version
           of the Tux Paint 'Magic' tool plugin API it was built against.
           Simply return TP_MAGIC_API_VERSION, which is defined in
           "tp_magic_api.h", to satisfy this requirement.
         * int init(magic_api * api)
           The plugin should do any initialization here. This function is
           called once, at Tux Paint startup. Return '1' if initialization
           was successful, or '0' if not (and Tux Paint will not present any
           'Magic' tools from the plugin).
         * void shutdown(magic_api * api)
           The plugin should do any cleanup here. This function is called
           once, at Tux Paint exit.
         * void click(magic_api * api, int which, SDL_Surface * snapshot,
           SDL_Surface * canvas, int x, int y)
           The plugin should apply the appropriate 'Magic' tool on the
           'canvas' surface. The (x,y) coordinates are where the mouse was
           (within the canvas) when the mouse button was clicked.
           The contents of the drawing canvas immediately prior to the mouse
           button click is stored within the 'snapshot' canvas.
         * void drag(magic_api * api, int which, SDL_Surface * snapshot,
           SDL_Surface * canvas, int ox, int oy, int x, int y)
           The plugin should apply the appropriate 'Magic' tool on the
           'canvas' surface. The (ox,oy) and (x,y) coordinates are the
           location of the mouse at the beginning and end of the stroke.
           Typically, plugins that let the user "draw" effects onto the
           canvas call the Tux Paint 'Magic' tool plugin "line()" helper
           function. (See below).
           Note: The contents of the drawing canvas immediately prior to the
           mouse button click remains as it was (when the plugin's "click()"
           function was called), and is still available in the 'snapshot'
           canvas.

  Tux Paint Functions

             Tux Paint provides a number of helper functions that plugins may
             access via the "magic_api" structure, sent to all of the
             plugin's functions (see above).

                * Uint32 getpixel(SDL_Surface * surf, int x, int y) Retreives
                  the pixel value from the (x,y) coordinates of an
                  SDL_Surface. (You can use SDL's "SDL_GetRGB()" function to
                  convert the Uint32 'pixel' to a set of Uint8 RGB values.)
                * void putpixel(SDL_Surface * surf, int x, int y, Uint32
                  pixel)
                  Sets the pixel value at position (x,y) of an SDL_Surface.
                  (You can use SDL's "SDL_MapRGB()" function to convert a set
                  of Uint8 RGB values to a Uint32 'pixel' value appropriate
                  to the destination surface.)
                * int in_circle(int x, int y, int radius)
                  Returns '1' if the (x,y) location is within a circle of a
                  particular radius (centered around the origin: (0,0)).
                  Returns '0' otherwise. Useful to create 'Magic' tools that
                  affect the canvas with a circular brush shape.
                * void show_progress_bar(void)
                  Asks Tux Paint to animate and draw one frame of its
                  progress bar (at the bottom of the screen). Useful for
                  routines that may take a long time, to provide feedback to
                  the user that Tux Paint has not crashed or frozen.
                * void tuxpaint_version(int * major, int * minor, int *
                  revision)
                  Returns the version of Tux Paint being used (e.g.,
                  "0.9.18"), separated into three integers.
                * void line(int which, SDL_Surface * canvas, SDL_Surface *
                  snapshot, int x1, int y1, int x2, int y2, int step, FUNC
                  callback)
                  This function calculates all points on a line between the
                  coordinates (x1,y1) and (x2,y2). Every 'step' iterations,
                  it calls the 'callback' function.
                  It sends the 'callback' function the (x,y) coordinates on
                  the line, Tux Paint's "magic_api" struct (as a "void *"
                  pointer), a 'which' value, represening which of the
                  plugin's 'Magic' tool is being used, and the current and
                  snapshot canvases.

                  Example prototype of a callback function that may be sent
                  to Tux Paint's "line()" 'Magic' tool plugin helper
                  function:

                    void exampleCallBack(void * ptr_to_api, int which_tool,
                    SDL_Surface * canvas, SDL_Surface * snapshot, int x, int
                    y);

                * void playsound(Mix_Chunk * snd, int pan, int dist)
                  This function plays a sound (one loaded by the SDL helper
                  library "SDL_mixer"). It uses SDL_mixer's
                  "Mix_SetPanning()" to set the volume of the sound on the
                  left and right speakers, based on the 'pan' and 'dist'
                  values sent to it.
                  A 'pan' of 128 causes the sound to be played at equal
                  volume on the left and right speakers. A 'pan' of 0 causes
                  it to be played completely on the left, and 255 completely
                  on the right.
                  The 'dist' value affects overall volume. 255 is loudest,
                  and 0 is silent.
                  The 'pan' and 'dist' values can be used to simulate
                  location and distance of the 'Magic' tool effect.
                * void special_notify(int flag)
                  This function notifies Tux Paint of special events. Various
                  values defined in "tp_magic_api.h" can be logically 'or'ed
                  together and sent to this function.

                     * SPECIAL_FLIP -- The contents of the canvas has been
                       flipped. If a 'Starter' image was used as the basis of
                       this image, it should be flipped too, and a record of
                       the flip should be stored as part of Tux Paint's undo
                       buffer stack. Additionally, the fact that the starter
                       has been flipped (or unflipped) should be recorded on
                       disk when the current drawing is saved.
                     * SPECIAL_MIRROR -- Similar to SPECIAL_FLIP, but for
                       magic tools that mirror the contents of the canvas.

                * int button_down(void)
                  A '1' is returned if the mouse button is down; '0'
                  otherwise.
                * float sRGB_to_linear(Uint8 srbg)
                  Converts an 8-bit sRGB value (one between 0 and 255) to a
                  linear floating point value (between 0.0 and 1.0).
                * uint8 linear_to_sRGB(float linear)
                  Converts a linear floating point value (one between 0.0 and
                  1.0) to an 8-bit sRGB value (between 0 and 255).

   --------------------------------------------------------------------------

Compiling

  Linux and other Unix-like Platforms

               Use the C compiler's "-shared" command-line option to generate
               a shared object file (".so") based on your 'Magic' tool
               plugin's C source code.

               Additionally, use the "tp-magic-config --cflags" command,
               supplied as part of Tux Paint, to provide additional
               command-line flags to your C compiler that will help it build
               your plugin.

               As a stand-alone command, using the GNU C Compiler and BASH
               shell, for example:

                 gcc -shared `tp-magic-config --cflags` my_plugin.c -o
                 my_plugin.so

               A snippet from a more generalized Makefile might look like
               this:

                 CFLAGS=-Wall -O2 $(shell tp-magic-config --cflags)

                 my_plugin.so: my_plugin.c    $(CC) -shared $(CFLAGS) -o $@
                 $<

               Then install globally into: /usr/[local/]lib/tuxpaint/. Or
               locally into: ~/.tuxpaint/magic/

  Windows

               TBD

  Mac OS X

               TBD

   --------------------------------------------------------------------------

Example Code

   --------------------------------------------------------------------------

           Summary and contact info TBD.
