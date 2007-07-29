                     Creating Tux Paint Magic Tool Plugins

                Copyright 2007-2007 by Bill Kendrick and others
                               New Breed Software

                           bill@newbreedsoftware.com
                            http://www.tuxpaint.org/

                          July 5, 2007 - July 28, 2007

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
     Simple DirectMedia Layer library ('libSDL', or simply 'SDL'; available
     from http://www.libsdl.org/). Therefore, for the moment at least, one
     must understand the C language and how to compile C-based programs.
     Familiarity with the SDL API is highly recommended, but some basic SDL
     concepts will be covered in this document.

   --------------------------------------------------------------------------

Interfaces

     Those who create 'Magic' tool plugins for Tux Paint must provide some
     interfaces (C functions) that Tux Paint may invoke.

     Tux Paint utilizes SDL's "SDL_LoadObject()" and "SDL_LoadFunction()"
     routines to load plugins (shared objects files; e.g., ".so" files on
     Linux or ".dll" files on Windows) and find the functions within.

     In turn, Tux Paint provides a number of helper functions that the plugin
     may (or sometimes is required to) use. This is exposed as a C structure
     (or "struct") which contains pointers to functions and other data inside
     Tux Paint. A pointer to this structure gets passed along to the plugin's
     functions as an argument when Tux Paint invokes them.

     Plugins should #include the C header file "tp_magic_api.h", which
     exposes the 'Magic' tool plugin API. Also, when you run the C compiler
     to build a plugin, you should use the command-line tool
     "tp-magic-config" to get the appropriate compiler flags (such as where
     the compiler can find the Tux Paint plugin header file, as well as SDL's
     header files) for building a plugin.

     The C header file and command-line tool mentioned above are included
     with Tux Paint -- or in some cases, as part of a "Tux Paint 'Magic' Tool
     Plugin Development package".

  'Magic' tool plugin functions

       'Magic' tool plugins must contain the functions listed below. Note: To
       avoid 'namespace' collisions, each function's name must start with the
       shared object's filename (e.g., "blur.so" or "blur.dll" would have
       functions whose names begin with "blur_"). This includes private
       functions (ones not used by Tux Paint directly), unless you declare
       those as 'static'.

    Common arguments to plugin functions:

       Here is a description of arguments that many of your plugin's
       functions will need to accept.

         * magic_api * api
           Pointer to a C structure containing pointers to Tux Paint
           functions and other data that the plugin can (and sometimes
           should) use. The contents of this struct are described below.

           Note: The magic_api struct is defined in the C header file
           "tp_magic_api.h", which you should include at the top of your
           plugin's C source file:

             #include "tp_magic_api.h"

         * int which
           An index the plugin should use to differentiate different 'Magic'
           tools, if the plugin provides more than one. (If not, "which" will
           always be 0.) See "Creating plugins with multiple effects", below.

         * SDL_Surface * snapshot
           A snapshot of the previous Tux Paint canvas, taken when the the
           mouse was first clicked to activate the current magic tool. If you
           don't continuously affect the image during one hold of the mouse
           button, you should base your effects off the contents of this
           canvas. (That is, read from "snapshot" and write to "canvas",
           below.)

         * SDL_Surface * canvas
           The current Tux Paint drawing canvas. Your magical effects should
           end up here!

         * SDL_Rect * update_rect
           A pointer to an SDL 'rectangle' structure that you use to tell
           Tux Paint what part of the canvas has been updated. If your effect
           affects a 32x32 area centered around the mouse pointer, you would
           fill the SDL_Rect as follows:

             update_rect->x = x - 16;
             update_rect->y = y - 16;
             update_rect->w = 32;
             update_rect->h = 32;

           Or, if your effect changes the entire canvas (e.g., flips it
           upside-down), you'd fill it as follows:

             update_rect->x = 0;
             update_rect->y = 0;
             update_rect->w = canvas->w;
             update_rect->h = canvas->h;

           Note: "update_rect" is a C pointer (an "SDL_Rect *" rather than
           just an "SDL_Rect") because you need to fill in its contents.
           Because it is a pointer, you access its elements via "->" (arrow)
           rather than "." (dot).

    Required plugin functions:

         Your plugin is required to contain, at the least, all of the
         following functions.

         Note: Remember, your plugin's function names must be preceded by
         your plugin's filename. That is, if your plugin is called "zoom.so"
         (on Linux) or "zoom.dll" (on Windows), then the names of your
         functions must begin with "zoom_" (e.g., "zoom_get_name(...)").

      Plugin "housekeeping" functions:

           * Uint32 api_version(void)
             The plugin should return an integer value representing the
             version of the Tux Paint 'Magic' tool plugin API the plugin was
             built against. The safest thing to do is return the value of
             TP_MAGIC_API_VERSION, which is defined in "tp_magic_api.h". If
             Tux Paint deems your plugin to be compatible, it will go ahead
             and use it.

             Note: Called once by Tux Paint, at startup. It is called first.
           * int init(magic_api * api)
             The plugin should do any initialization here. Return '1' if
             initialization was successful, or '0' if not (and Tux Paint will
             not present any 'Magic' tools from the plugin).

             Note: Called once by Tux Paint, at startup. It is called first.
             It is called after "api_version()", if Tux Paint believes your
             plugin to be compatible.
           * int get_tool_count(magic_api * api)
             This should return the number of Magic tools this plugin
             provides to Tux Paint.

             Note: Called once by Tux Paint, at startup. It is called after
             your "init()", if it succeeded.

           * char * get_name(magic_api * api, int which)
             This should return a string containing the name of a magic tool.
             This will appear on the button in the 'Magic' selector within
             Tux Paint.

             Tux Paint will free() the string upon exit, so you should wrap
             it in a C strdup() call.

             Note: Called once for each Magic tool your plugin claims to
             contain (by your "get_tool_count()").

           * SDL_Surface * get_icon(magic_api * api, int which)
             This should return an SDL_Surface containing the icon
             representing the tool. (A greyscale image with alpha, no larger
             than 40x40.) This will appear on the button in the 'Magic'
             selector within Tux Paint.

             Tux Paint will free ("SDL_FreeSurface()") the surface upon exit.

             Note: Called once for each Magic tool your plugin claims to
             contain (by your "get_tool_count()").

           * char * get_description(magic_api * api, int which)
             This should return a string containing the description of a
             magic tool. This will appear as a help tip, explained by Tux the
             Penguin, within Tux Paint.

             Tux Paint will free() the string upon exit, so you should wrap
             it in a C strdup() call.

             Note: Called once for each Magic tool your plugin claims to
             contain (by your "get_tool_count()").

           * int requires_colors(magic_api * api, int which)
             Return a '1' if the 'Magic' tool accepts colors (the 'Colors'
             palette in Tux Paint will be available), or '0' if not.

             Note: Called once for each Magic tool your plugin claims to
             contain (by your "get_tool_count()").

           * void shutdown(magic_api * api)
             The plugin should do any cleanup here. If you allocated any
             memory or used SDL_Mixer to load any sounds during init(), for
             example, you should free() the allocated memory and
             Mix_FreeChunk() the sounds here.

             Note: This function is called once, when Tux Paint exits.

      Plugin event functions:

           * void set_color(magic_api * api, Uint8 r, Uint8 g, Uint8 g)
             Tux Paint will call this function to inform the plugin of the
             RGB values of the currently-selected color in Tux Paint's
             'Colors' palette. (It will be called whenever one of the
             plguin's Magic tools that accept colors becomes active, or the
             user picks a new color while such a tool is currently active.)

           * void click(magic_api * api, int which, SDL_Surface * snapshot,
             SDL_Surface * canvas, int x, int y, SDL_Rect * update_rect)
             The plugin should apply the appropriate 'Magic' tool on the
             'canvas' surface. The (x,y) coordinates are where the mouse was
             (within the canvas) when the mouse button was clicked.

             The plugin should report back what part of the canvas was
             affected, by filling in the (x,y) and (w,h) values in
             'update_rect'.

             The contents of the drawing canvas immediately prior to the
             mouse button click is stored within the 'snapshot' canvas.

           * void drag(magic_api * api, int which, SDL_Surface * snapshot,
             SDL_Surface * canvas, int ox, int oy, int x, int y, SDL_Rect *
             update_rect)
             The plugin should apply the appropriate 'Magic' tool on the
             'canvas' surface. The (ox,oy) and (x,y) coordinates are the
             location of the mouse at the beginning and end of the stroke.
             Typically, plugins that let the user "draw" effects onto the
             canvas call the Tux Paint 'Magic' tool plugin "line()" helper
             function. (See below).

             The plugin should report back what part of the canvas was
             affected, by filling in the (x,y) and (w,h) values in
             'update_rect'.

             Note: The contents of the drawing canvas immediately prior to
             the mouse button click remains as it was (when the plugin's
             "click()" function was called), and is still available in the
             'snapshot' canvas.

           * void release(magic_api * api, int which, SDL_Surface * snapshot,
             SDL_Surface * canvas, int x, int y, SDL_Rect * update_rect)
             The plugin should apply the appropriate 'Magic' tool on the
             'canvas' surface. The (x,y) coordinates are where the mouse was
             (within the canvas) when the mouse button was released.

             The plugin should report back what part of the canvas was
             affected, by filling in the (x,y) and (w,h) values in
             'update_rect'.

             Note: The contents of the drawing canvas immediately prior to
             the mouse button click remains as it was (when the plugin's
             "click()" function was called), and is still available in the
             'snapshot' canvas.

  Tux Paint Functions and Data

       Tux Paint provides a number of helper functions that plugins may
       access via the "magic_api" structure, sent to all of the plugin's
       functions (see above).

    Pixel Manipulations

           * Uint32 getpixel(SDL_Surface * surf, int x, int y) Retreives the
             pixel value from the (x,y) coordinates of an SDL_Surface. (You
             can use SDL's "SDL_GetRGB()" function to convert the Uint32
             'pixel' to a set of Uint8 RGB values.)

           * void putpixel(SDL_Surface * surf, int x, int y, Uint32 pixel)
             Sets the pixel value at position (x,y) of an SDL_Surface. (You
             can use SDL's "SDL_MapRGB()" function to convert a set of Uint8
             RGB values to a Uint32 'pixel' value appropriate to the
             destination surface.)

    Helper Functions

           * int in_circle(int x, int y, int radius)
             Returns '1' if the (x,y) location is within a circle of a
             particular radius (centered around the origin: (0,0)). Returns
             '0' otherwise. Useful to create 'Magic' tools that affect the
             canvas with a circular brush shape.

           * void line(int which, SDL_Surface * canvas, SDL_Surface *
             snapshot, int x1, int y1, int x2, int y2, int step, FUNC
             callback)
             This function calculates all points on a line between the
             coordinates (x1,y1) and (x2,y2). Every 'step' iterations, it
             calls the 'callback' function.

             It sends the 'callback' function the (x,y) coordinates on the
             line, Tux Paint's "magic_api" struct (as a "void *" pointer), a
             'which' value, represening which of the plugin's 'Magic' tool is
             being used, and the current and snapshot canvases.

             Example prototype of a callback function that may be sent to
             Tux Paint's "line()" 'Magic' tool plugin helper function:

               void exampleCallBack(void * ptr_to_api, int which_tool,
               SDL_Surface * canvas, SDL_Surface * snapshot, int x, int y);

    Informational

           * char * tp_version
             A string containing the version of Tux Paint that's running
             (e.g., "0.9.18").

           * int canvas_w Returns the width of the drawing canvas.

           * int canvas_h Returns the height of the drawing canvas.

           * int button_down(void)
             A '1' is returned if the mouse button is down; '0' otherwise.

    Tux Paint System Calls

           * void show_progress_bar(void)
             Asks Tux Paint to animate and draw one frame of its progress bar
             (at the bottom of the screen). Useful for routines that may take
             a long time, to provide feedback to the user that Tux Paint has
             not crashed or frozen.

           * void playsound(Mix_Chunk * snd, int pan, int dist)
             This function plays a sound (one loaded by the SDL helper
             library "SDL_mixer"). It uses SDL_mixer's "Mix_SetPanning()" to
             set the volume of the sound on the left and right speakers,
             based on the 'pan' and 'dist' values sent to it.

             A 'pan' of 128 causes the sound to be played at equal volume on
             the left and right speakers. A 'pan' of 0 causes it to be played
             completely on the left, and 255 completely on the right.

             The 'dist' value affects overall volume. 255 is loudest, and 0
             is silent.
             The 'pan' and 'dist' values can be used to simulate location and
             distance of the 'Magic' tool effect.

           * void special_notify(int flag)
             This function notifies Tux Paint of special events. Various
             values defined in "tp_magic_api.h" can be logically 'or'ed ("|")
             together and sent to this function.

                * SPECIAL_FLIP -- The contents of the canvas has been
                  flipped.

                  If a 'Starter' image was used as the basis of this image,
                  it should be flipped too, and a record of the flip should
                  be stored as part of Tux Paint's undo buffer stack.
                  Additionally, the fact that the starter has been flipped
                  (or unflipped) should be recorded on disk when the current
                  drawing is saved.
                * SPECIAL_MIRROR -- Similar to SPECIAL_FLIP, but for magic
                  tools that mirror the contents of the canvas.

    Color Conversions

           * float sRGB_to_linear(Uint8 srbg)
             Converts an 8-bit sRGB value (one between 0 and 255) to a linear
             floating point value (between 0.0 and 1.0).

             See also: sRGB article at Wikipedia.

           * uint8 linear_to_sRGB(float linear)
             Converts a linear floating point value (one between 0.0 and 1.0)
             to an 8-bit sRGB value (between 0 and 255).

           * void rgbtohsv(Uint8 r, Uint8 g, Uint8 b, float * h, float * s,
             float * v)
             Converts 8-bit sRGB values (between 0 and 255) to floating-point
             HSV (Hue, Saturation and Value) values (Hue between 0.0 and
             360.0, and Saturation and Value between 0.0 and 1.0).

             See also: HSV Color Space article at Wikipedia.

           * void hsvtorgb(float h, float s, float v, Uint8 * r, Uint8 * g,
             Uint8 * b)
             Converts floating-point HSV (Hue, Saturation and Value) values
             (Hue between 0.0 and 360.0, and Saturation and Value between 0.0
             and 1.0) to 8-bit sRGB values (between 0 and 255).

  Helper Macros in "tp_magic_api.h":

       Along with the "magic_api" C structure containing functions and data
       described above, the tp_magic_api.h C header file also contains some
       helper macros that you may use.

         * min(x, y)
           The minimum of 'x' and 'y'. (That is, if 'x' is less than or equal
           to 'y', then the value of 'x' will be used. If 'y' is less than
           'x', it will be used.)

         * max(x, y)
           The maximum of 'x' and 'y'. The opposite of min().

         * clamp(lo, value, hi)
           A value, clamped to be no smaller than 'lo', and no higher than
           'hi'. (That is, if 'value' is less than 'lo', then 'lo' will be
           used; if 'value' is greater than 'hi', then 'hi' will be used;
           otherwise, 'value' will be used.)

           Example: red = clamp(0, n, 255);
           Tries to set 'red' to be the value of 'n', but without allowing it
           to become less than 0 or greater than 255.

           Note: This macro is simply a #define of:
           "(min(max(value,lo),hi))".

   --------------------------------------------------------------------------

Compiling

  Linux and other Unix-like Platforms

       Use the C compiler's "-shared" command-line option to generate a
       shared object file (".so") based on your 'Magic' tool plugin's C
       source code.

       Additionally, use the "tp-magic-config --cflags" command, supplied as
       part of Tux Paint, to provide additional command-line flags to your C
       compiler that will help it build your plugin.

       As a stand-alone command, using the GNU C Compiler and BASH shell, for
       example:

         gcc -shared `tp-magic-config --cflags` my_plugin.c -o my_plugin.so

       Note: The characters around the "tp-magic-config" command are a
       grave/backtick/backquote ("`"), and not an apostrophe/single-quote
       ("'"). They tell the shell to execute the command within (in this
       case, "tp-magic-config ..."), and use its output as an argument to the
       command being executed (in this case, "gcc ...").

       A snippet from a more generalized Makefile might look like this:

         CFLAGS=-Wall -O2 $(shell tp-magic-config --cflags)

         my_plugin.so: my_plugin.c    $(CC) -shared $(CFLAGS) -o $@ $<

       You may then install it globally into: /usr/lib/tuxpaint/plugins/ or
       /usr/local/lib/tuxpaint/plugins/ (depending on how Tux Paint was
       installed).

       Or install it locally (for the current user only) into:
       ~/.tuxpaint/magic/
       (FIXME: As of 2007-07-27, Tux Paint does not look here yet!)

  Windows

       TBD

  Mac OS X

       TBD

   --------------------------------------------------------------------------

Creating plugins with multiple effects

     TBD

   --------------------------------------------------------------------------

Example Code

     TBD

   --------------------------------------------------------------------------

   Summary and contact info TBD.
