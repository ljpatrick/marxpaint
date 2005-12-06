                                   Tux Paint
                                 version 0.9.16

                     A simple drawing program for children

                   Copyright 2005 by Bill Kendrick and others
                               New Breed Software

                           bill@newbreedsoftware.com
                   http://www.newbreedsoftware.com/tuxpaint/

                        June 14, 2002 - December 6, 2005

   --------------------------------------------------------------------------

                                     About

What Is 'Tux Paint?'

       Tux Paint is a free drawing program designed for young children (kids
       ages 3 and up). It has a simple, easy-to-use interface, fun sound
       effects, and an encouraging cartoon mascot who helps guide children as
       they use the program. It provides a blank canvas and a variety of
       drawing tools to help your child be creative.

License:

       Tux Paint is an Open Source project, Free Software released under the
       GNU General Public License (GPL). It is free, and the 'source code'
       behind the program is available. (This allows others to add features,
       fix bugs, and use parts of the program in their own GPL'd software.)

       See COPYING.txt for the full text of the GPL license.

Objectives:

       Easy and Fun
               Tux Paint is meant to be a simple drawing program for young
               children. It is not meant as a general-purpose drawing tool.
               It is meant to be fun and easy to use. Sound effects and a
               cartoon character help let the user know what's going on, and
               keeps them entertained. There are also extra-large
               cartoon-style mouse pointer shapes.

       Extensibility
               Tux Paint is extensible. Brushes and "rubber stamp" shapes can
               be dropped in and pulled out. For example, a teacher can drop
               in a collection of animal shapes and ask their students to
               draw an ecosystem. Each shape can have a sound which is
               played, and textual facts which are displayed, when the child
               selects the shape.

       Portability
               Tux Paint is portable among various computer platforms:
               Windows, Macintosh, Linux, etc. The interface looks the same
               among them all. Tux Paint runs suitably well on older systems
               (like a Pentium 133), and can be built to run better on slow
               systems.

       Simplicity
               There is no direct access to the computer's underlying
               intricacies. The current image is kept when the program quits,
               and reappears when it is restarted. Saving images requires no
               need to create filenames or use the keyboard. Opening an image
               is done by selecting it from a collection of thumbnails.
               Access to other files on the computer is restricted.

   --------------------------------------------------------------------------

                                Using Tux Paint

Loading Tux Paint

  Linux/Unix Users

         Tux Paint should have placed a laucher icon in your KDE and/or GNOME
         menus, under 'Graphics.'

         Alternatively, you can run the following command at a shell prompt
         (e.g., "$"):

           $ tuxpaint

         If any errors occur, they will be displayed on the terminal (to
         "stderr").

   --------------------------------------------------------------------------

  Windows Users

                                                                    [Icon]
                                                                   Tux Paint

         If you installed Tux Paint on your computer using the
         'Tux Paint Installer,' it will have asked you whether you wanted a
         'Start' menu short-cut, and/or a desktop shortcut. If you agreed,
         you can simply run Tux Paint from the 'Tux Paint' section of your
         'Start' menu (e.g., under "All Programs" on Windows XP), or by
         double-clicking the "Tux Paint" icon on your desktop.

         If you installed Tux Paint using the 'ZIP-file' download, or if you
         used the 'Tux Paint Installer,' but chose not to have shortcuts
         installed, you'll need to double-click the "tuxpaint.exe" icon in
         the 'Tux Paint' folder on your computer.

         By default, the 'Tux Paint Installer' will put Tux Paint's folder in
         "C:\Program Files\", though you may have changed this when the
         installer ran.

         If you used the 'ZIP-file' download, Tux Paint's folder will be
         wherever you put it when you unzipped the ZIP file.

   --------------------------------------------------------------------------

  Mac OS X Users

         Simply double-click the "Tux Paint" icon.

   --------------------------------------------------------------------------

Title Screen

       When Tux Paint first loads, a title/credits screen will appear.

                               [Title Screenshot]

       Once loading is complete, press a key or click on the mouse to
       continue. (Or, after about 30 seconds, the title screen will go away
       automatically.)

   --------------------------------------------------------------------------

Main Screen

       The main screen is divided into the following sections:

       Left Side: Toolbar

               The toolbar contains the drawing and editing controls.

[Tools: Paint, Stamp, Lines, Shapes, Text, Magic, Undo, Redo, Eraser, New, Open,
                               Save, Print, Quit]

       Middle: Drawing Canvas

               The largest part of the screen, in the center, is the drawing
               canvas. This is, obviously, where you draw!

                                   [(Canvas)]

       Right Side: Selector

               Depending on the current tool, the selector shows different
               things. e.g., when the Paint Brush tool is selected, it shows
               the various brushes available. When the Rubber Stamp tool is
               selected, it shows the different shapes you can use.

                 [Selectors - Brushes, Letters, Shapes, Stamps]

       Lower: Colors

               A palette of available colors are shown near the bottom of the
               screen.

 [Colors - Black, White, Red, Pink, Orange, Yellow, Green, Cyan, Blue, Purple,
                                  Brown, Grey]

       Bottom: Help Area

               At the very bottom of the screen, Tux, the Linux Penguin,
               provides tips and other information while you draw.

(For example: 'Pick a shape. Click to pick the center, drag, then let go when it
     is the size you want. Move around to rotate it, and click to draw it.)

   --------------------------------------------------------------------------

Available Tools

  Drawing Tools

         Paint (Brush)

                 The Paint Brush tool lets you draw freehand, using various
                 brushes (chosen in the Selector on the right) and colors
                 (chosen in the Color palette towards the bottom).

                 If you hold the mouse button down, and move the mouse, it
                 will draw as you move.

                 As you draw, a sound is played. The bigger the brush, the
                 lower the pitch.

   --------------------------------------------------------------------------

         Stamp (Rubber Stamp)

                 The Stamp tool is like a rubber stamp, or stickers. It lets
                 you paste pre-drawn or photographic images (like a picture
                 of a horse, or a tree, or the moon) in your picture.

                 As you move the mouse around, an outline follows the mouse,
                 showing where the stamp will be placed.

                 Different stamps can have different sound effects. Some
                 stamps can be colored or tinted.

                 Stamps can be shrunk and expanded, and many stamps can be
                 flipped vertically, or displayed as a mirror-image, using
                 controls at the bottom right of the screen.

                 (NOTE: If the "nostampcontrols" option is set, Tux Paint
                 won't display the Mirror, Flip, Shrink and Grow controls for
                 stamps. See the "Options" documentation.)

   --------------------------------------------------------------------------

         Lines

                 This tool lets you draw straight lines using the various
                 brushes and colors you normally use with the Paint Brush.

                 Click the mouse and hold it to choose the starting point of
                 the line. As you move the mouse around, a thin 'rubber-band'
                 line will show where the line will be drawn.

                 Let go of the mouse to complete the line. A "sproing!" sound
                 will play.

   --------------------------------------------------------------------------

         Shapes

                 This tool lets you draw some simple filled, and un-filled
                 shapes.

                 Select a shape from the selector on the right (circle,
                 square, oval, etc.).

                 In the canvas, click the mouse and hold it to stretch the
                 shape out from where you clicked. Some shapes can change
                 proportion (e.g., rectangle and oval), others cannot (e.g.,
                 square and circle).

                 Let go of the mouse when you're done stretching.

                      Normal Mode

                              Now you can move the mouse around the canvas to
                              rotate the shape.

                              Click the mouse button again and the shape will
                              be drawn in the current color.

                      Simple Shapes Mode
                              If simple shapes are enabled (e.g., with the
                              "--simpleshapes" option), the shape will be
                              drawn on the canvas when you let go of the
                              mouse button. (There's no rotation step.)

   --------------------------------------------------------------------------

         Text

                 Choose a font (from the 'Letters' available on the right)
                 and a color (from the color palette near the bottom). Click
                 on the screen and a cursor will appear. Type text and it
                 will show up on the screen.

                 Press [Enter] or [Return] and the text will be drawn onto
                 the picture and the cursor will move down one line.

                 Click elsewhere in the picture and the current line of text
                 will move there, where you can continue editing.

   --------------------------------------------------------------------------

         Magic (Special Effects)

                 The 'Magic' tool is actually a set of special tools. Select
                 one of the "magic" effects from the selector on the right,
                 and then click and drag around the picture to apply the
                 effect.

                      Rainbow
                              This is similar to the paint brush, but as you
                              move the mouse around, it goes through all of
                              the colors in the rainbow.

                      Sparkles
                              This draws glowing yellow sparkles on the
                              picture.

                      Mirror
                              When you click the mouse in your picture with
                              the "Mirror" magic effect selected, the entire
                              image will be reversed, turning it into a
                              mirror image.

                      Flip
                              Similar to "Mirror." Click and the entire image
                              will be turned upside-down.

                      Blur
                              This makes the picture fuzzy wherever you drag
                              the mouse.

                      Smudge
                              This pushes the colors around under the mouse,
                              like finger painting with wet paint.

                      Fade
                              This fades the colors wherever you drag the
                              mouse. (Do it to the same spot many times, and
                              it will eventually become white.)

                      Darken
                              This dakrens the colors wherever you drag the
                              mouse. (Do it to the same spot many times, and
                              it will eventually become black.)

                      Chalk
                              This makes parts of the picture (where you move
                              the mouse) look like a chalk drawing.

                      Blocks
                              This makes the picture blocky looking
                              ("pixelated") wherever you drag the mouse.

                      Negative
                              This inverts the colors wherever you drag the
                              mouse. (e.g., white becomes black, and vice
                              versa.)

                      Tint
                              This changes the parts of the picture to the
                              selected color.

                      Drip
                              This makes the paint "drip" wherever you move
                              the mouse.

                      Cartoon
                              This makes the picture look like a cartoon --
                              with thick outlines and solid colors --
                              wherever you move the mouse.

                      Fill
                              This floods the picture with a color. It lets
                              you quickly fill parts of the picture, as if it
                              were a coloring book.

   --------------------------------------------------------------------------

         Eraser

                 This tool is similar to the Paint Brush. Wherever you click
                 (or click and drag), the picture will be erased either to
                 white, or to the background picture, if you began the
                 current drawing with a 'Starter' image.

                 A number of eraser sizes are available.

                 As you move the mouse around, a square outline follows the
                 pointer, showing what part of the picture will be erased to
                 white.

                 As you erase, a "squeaky clean" eraser/wiping sound is
                 played.

   --------------------------------------------------------------------------

  Other Controls

         Undo

                 Clicking this tool will undo the last drawing action. You
                 can even undo more than once!

                 Note: You can also press [Control]-[Z] on the keyboard to
                 undo.

   --------------------------------------------------------------------------

         Redo

                 Clicking this tool will redo the drawing action you just
                 "undid" with the 'Undo' button.

                 As long as you don't draw again, you can redo as many times
                 as you had "undone!"

                 Note: You can also press [Control]-[R] on the keyboard to
                 redo.

   --------------------------------------------------------------------------

         New

                 Clicking the "New" button will start a new drawing. You will
                 first be asked whether you really want to do this.

                 Note: You can also press [Control]-[N] on the keyboard to
                 start a new drawing.

   --------------------------------------------------------------------------

         Open

                 This shows you a list of all of the pictures you've saved.
                 If there are more than can fit on the screen, use the "Up"
                 and "Down" arrows at the top and bottom of the list to
                 scroll through the list of pictures.

                 Click a picture to select it, then...

                      * Click the green "Open" button at the lower left of
                        the list to load the selected picture.

                        (Alternatively, you can double-click a picture's icon
                        to load it.)

                      * Click the brown "Erase" (trash can) button at the
                        lower right of the list to erase the selected
                        picture. (You will be asked to confirm.)

                      * Or click the red "Back" arrow button at the lower
                        right of the list to cancel and return to the picture
                        you were drawing.

                 'Starter' Images

                   Along with pictures you've created, Tux Paint can provided
                   'Starter' images. Opening them is like creating a new
                   picture, except that the picture isn't blank. 'Starters'
                   can be like a page from a coloring book (a black-and-white
                   outline of a picture, which you can then color in), or
                   like a 3D photograph, where you draw the bits in between.

                   'Starter' images have a green background in the 'Open'
                   screen. (Normal images have a blue background.) When you
                   load a 'Starter,' draw on it, and then click 'Save,' it
                   creates a new picture (it doesn't overwrite the original
                   'Starter,' so you can use it again later).

                 If choose to open a picture, and your current drawing hasn't
                 been saved, you will be prompted as to whether you want to
                 save it or not. (See "Save," below.)

                 Note: You can also press [Control]-[O] on the keyboard to
                 get the 'Open' dialog.

   --------------------------------------------------------------------------

         Save

                 This saves your current picture.

                 If you haven't saved it before, it will create a new entry
                 in the list of saved images. (i.e., it will create a new
                 file)

                 Note: It won't ask you anything (e.g., for a filename). It
                 will simply save the picture, and play a "camera shutter"
                 sound effect.

                 If you HAVE saved the picture before, or this is a picture
                 you just loaded using the "Open" command, you will first be
                 asked whether you want to save over the old version, or
                 create a new entry (a new file).

                 (NOTE: If either the "saveover" or "saveovernew" options are
                 set, it won't ask before saving over. See the "Options"
                 documentation.)

                 Note: You can also press [Control]-[S] on the keyboard to
                 save.

   --------------------------------------------------------------------------

         Print

                 Click this button and your picture will be printed!

                 On most platforms, you can also hold the [Alt] key while
                 clicking the 'Print' button to get a printer dialog, as long
                 as you're not running Tux Paint in fullscreen mode. See
                 below.

                      Disabling Printing

                              If the "noprint" option was set (either with
                              "noprint=yes" in Tux Paint's configuration
                              file, or using "--noprint" on the
                              command-line), the "Print" button will be
                              disabled.

                              See the "Options" documentation.)

                      Restricting Printing

                              If the "printdelay" option was used (either
                              with "printdelay=SECONDS" in the configuration
                              file, or using "--printdelay=SECONDS" on the
                              command-line), you can only print once every
                              SECONDS seconds.

                              For example, with "printdelay=60", you can
                              print only once a minute.

                              See the "Options" documentation.)

                      Printing Commands

                              (Linux and Unix only)

                              Tux Paint prints by generating a PostScript
                              representation of the drawing and sending it to
                              an external program. By default, the program
                              is:

                                lpr

                              This command can be changed by setting the
                              "printcommand" value in Tux Paint's
                              configuration file.

                              If the [Alt] key on the keyboard is being
                              pushed while clicking the 'Print' button, as
                              long as you're not in fullscreen mode, an
                              alternative program is run. By default, the
                              program is KDE's graphical print dialog:

                                kprinter

                              This command can be changed by setting the
                              "altprintcommand" value in Tux Paint's
                              configuration file.

                              For information on how to change the printing
                              commands, see the "Options" documentation.

                      Printer Settings

                              (Windows only)

                              By default, Tux Paint simply prints to the
                              default printer with default settings when the
                              'Print' button is pushed.

                              However, if you hold the [Alt] key on the
                              keyboard while pushing the button, as long as
                              you're not in fullscreen mode, a Windows print
                              dialog will appear, where you can change the
                              settings.

                              You can have the printer configuration changes
                              stored by using the "printcfg" option, either
                              by using "--printcfg" on the command-line, or
                              "printcfg=yes" in Tux Paint's own configuration
                              file ("tuxpaint.cfg").

                              If the "printcfg" option is used, printer
                              settings will be loaded from the file
                              "userdata/print.cfg". Any changes will be saved
                              there as well.

                              See the "Options" documentation.)

                      Printer Dialog Options

                              By default, Tux Paint only shows the printer
                              dialog (or, on Linux/Unix, runs the
                              "altprintcommand", e.g., "kprinter" instead of
                              "lpr") if the [Alt] key is held while clicking
                              the 'Print' button.

                              However, this behavior can be changed. You can
                              have the printer dialog always appear by using
                              "--altprintalways" on the command-line, or
                              "altprint=always" in Tux Paint's configuration
                              file. Or, you can prevent the [Alt] key from
                              having any effect by using "--altprintnever",
                              or "altprint=never".

                              See the "Options" documentation.)

   --------------------------------------------------------------------------

         Quit

                 Clicking the "Quit" button, closing the Tux Paint window, or
                 pushing the [Escape] key will quit Tux Paint.

                 You will first be prompted as to whether you really want to
                 quit.

                 If you choose to quit, and you haven't saved the current
                 picture, you will first be asked if wish to save it. If it's
                 not a new image, you will then be asked if you want to save
                 over the old version, or create a new entry. (See "Save"
                 above.)

                 NOTE: If the image is saved, it will be reloaded
                 automatically the next time you run Tux Paint!

                 NOTE: The "Quit" button and [Escape] key can be disabled
                 (e.g., by selecting "Disable 'Quit' Button" in
                 Tux Paint Config. or running Tux Paint with the "--noquit"
                 command-line option).

                 In that case, the "window close" button on Tux Paint's title
                 bar (if not in fullscreen mode) or the [Alt] + [F4] key
                 sequence may be used to quit.

                 If neither of those are possible, the key sequence of
                 [Shift] + [Control] + [Escape] may be used to quit. (See the
                 "Options" documentation.)

         Sound Muting

                 There is no on-screen control button at this time, but by
                 pressing [Alt] + [S], sound effects can be disabled and
                 re-enabled (muted and unmuted) while the program is running.

                 Note that if sounds are completely disabled (e.g., by
                 unselecting "Enable Sound Effects" in Tux Paint Config or
                 running Tux Paint with the "--nosound" command-line option),
                 the [Alt] + [S] key sequence has no effect. (i.e., it cannot
                 be used to turn on sounds when the parent/teach wants them
                 disabled.)

   --------------------------------------------------------------------------

                     Loading Other Pictures into Tux Paint

     Since Tux Paint's 'Open' dialog only displays pictures you created with
     Tux Paint, what if you want to load some other picture or photograph
     into Tux Paint to edit?

     To do so, you simply need to convert the picture into a PNG (Portable
     Network Graphic) image file, and place it in Tux Paint's "saved"
     directory. ("~/.tuxpaint/saved/" under Linux and Unix, "userdata\saved\"
     under Windows, "Library/Preferences/tuxpaint/saved/" under Mac OS X.)

Using 'tuxpaint-import'

       Linux and Unix users can use the "tuxpaint-import" shell script which
       gets installed when you install Tux Paint. It uses some NetPBM tools
       to convert the image ("anytopnm"), resize it so that it will fit in
       Tux Paint's canvas ("pnmscale"), and convert it to a PNG ("pnmtopng").

       It also uses the "date" command to get the current time and date,
       which is the file-naming convention Tux Paint uses for saved files.
       (Remember, you are never asked for a 'filename' when you go to Save or
       Open pictures!)

       To use 'tuxpaint-import', simply run the command from a command-line
       prompt and provide it the name(s) of the file(s) you wish to convert.

       They will be converted and placed in your Tux Paint 'saved' directory.
       (Note: If you're doing this for a different user - e.g., your child,
       you'll need to make sure to run the command under their account.)

       Example:

         $ tuxpaint-import grandma.jpg
         grandma.jpg -> /home/username/.tuxpaint/saved/20020921123456.png
         jpegtopnm: WRITING A PPM FILE

       The first line ("tuxpaint-import grandma.jpg") is the command to run.
       The following two lines are output from the program while it's
       working.

       Now you can load Tux Paint, and a version of that original picture
       will be available under the 'Open' dialog. Just double-click its icon!

Doing it Manually

       Windows, Mac OS X and BeOS users must currently do the conversion
       manually.

       Load a graphics program that is capable of both loading your picture
       and saving a PNG format file. (See the documentation file "PNG.txt"
       for a list of suggested software, and other references.)

       Reduce the size of the image to no wider than 448 pixels across and no
       taller than 376 pixels tall. (i.e., the maximum size is 448 x 376
       pixels)

       Save the picture in PNG format. It is highly recommended that you name
       the filename using the current date and time, since that's the
       convention Tux Paint uses:

         YYYYMMDDhhmmss.png

         * YYYY = Year
         * MM = Month (01-12)
         * DD = Day (01-31)
         * HH = Hour, in 24-hour format (00-23)
         * mm = Minute (00-59)
         * ss = Second (00-59)

       e.g.:

         20020921130500 - for September 21, 2002, 1:05:00pm

       Place this PNG file in your Tux Paint 'saved' directory. (See above.)

       Under Windows, this is in the "userdata" folder. Under Mac OS X, this
       is in "Library/Application Support/tuxpaint/" in your home directory.

   --------------------------------------------------------------------------

                              Extending Tux Paint

     If you wish to add or change things like Brushes and Rubber Stamps used
     by Tux Paint, you can do it fairly easily by simply putting or removing
     files on your hard disk.

     Note: You'll need to restart Tux Paint for the changes to take effect.

Where Files Go

  Standard Files

         Tux Paint looks for its various data files in its 'data' directory.

    Linux and Unix

           Where this directory goes depends on what value was set for
           "DATA_PREFIX" when Tux Paint was built. See INSTALL.txt for
           details.

           By default, though, the directory is:

             /usr/local/share/tuxpaint/

           If you installed from a package, it is more likely to be:

             /usr/share/tuxpaint/

    Windows

           Tux Paint looks for a directory called 'data' in the same
           directory as the executable. This is the directory that the
           installer used when installing Tux Paint e.g.:

             C:\Program Files\TuxPaint\data

    Mac OS X

           Tux Paint stores files in your account's "Libraries" folder, under
           "Preferences", e.g.:

             /Users/Joe/Library/Preferences/

   --------------------------------------------------------------------------

  Personal Files

         You can also create brushes, stamps, fonts and 'starters' in your
         own directory for Tux Paint to find.

    Linux and Unix

           Your personal Tux Paint directory is "~/.tuxpaint/".

           That is, if your home directory is "/home/karl", then your
           Tux Paint directory is "/home/karl/.tuxpaint/".

           Don't forget the period (".") before the 'tuxpaint'!

    Windows

           Your personal Tux Paint directory is named "userdata" and is in
           the same directory as the executable e.g.:

             C:\Program Files\TuxPaint\userdata

         To add brushes, stamps fonts, and 'starters,' create subdirectories
         under your personal Tux Paint directory named "brushes", "stamps",
         "fonts" and "starters" respectively.

         (For example, if you created a brush named "flower.png", you would
         put it in "~/.tuxpaint/brushes/" under Linux or Unix.)

   --------------------------------------------------------------------------

Brushes

       The brushes used for drawing with the 'Brush' and 'Lines' tools in
       Tux Paint are simply greyscale PNG images.

       The alpha (transparency) of the PNG image is used to determine the
       shape of the brush, which means that the shape can be 'anti-aliased'
       and even partially-transparent!

       Brush images should be no wider than 40 pixels across and no taller
       than 40 pixels high. (i.e., the maximum size can be 40 x 40.)

       Just place them in the "brushes" directory.

       Note: If your new brushes all come out as solid squares or rectangles,
       it's because you forgot to use alpha transparency! See the
       documentation file "PNG.txt" for more information and tips.

   --------------------------------------------------------------------------

Stamps

       All stamp-related files go in the "stamps" directory. It's useful to
       create subdirectories and sub-subdirectories there to organize the
       stamps. (For example, you can have a "holidays" folder with
       "halloween" and "christmas" sub-folders.)

  Images

         Rubber Stamps in Tux Paint can be made up of a number of separate
         files. The one file that is required is, of course, the picture
         itself.

         The Stamps used by Tux Paint are PNG pictures. They can be
         full-color or greyscale. The alpha (transparency) of the PNG is used
         to determine the actual shape of the picture (otherwise you'll stamp
         a large rectangle on your drawings).

         The PNGs can be any size, but in practice, a 100 pixels wide by
         100 pixels tall (100 x 100) is quite large for Tux Paint.

         Note: If your new stamps all have solid rectangular-shaped outlines
         of a solid color (e.g., white or black), it's because you forgot to
         use alpha transparency! See the documentation file "PNG.txt" for
         more information and tips.

   --------------------------------------------------------------------------

  Description Text

         Text (".TXT") files with the same name as the PNG. (e.g.,
         "picture.png"'s description is stored in "picture.txt" in the same
         directory.)

         The first line of the text file will be used as the US English
         description of the stamp's image. It must be encoded in UTF-8.

    Language Support

           Additional lines can be added to the text file to provide
           translations of the description, to be displayed when Tux Paint is
           running in a different locale (like French or Spanish).

           The beginning of the line should correspond to the language code
           of the language in question (e.g., "fr" for French, and "zh_tw"
           for Traditional Chinese), followed by ".utf8=" and the translated
           description (encoded in UTF-8).

           There are scripts in the "po" directory for converting the text
           files to PO format (and back) for easy translation to different
           languages. Therefore you should never add or change translations
           in the .txt files directly.

           If no translation is available for the language Tux Paint is
           currently running in, the US English text is used.

    Windows Users

           Use NotePad or WordPad to edit/create these files. Be sure to save
           them as Plain Text, and make sure they have ".txt" at the end of
           the filename...

   --------------------------------------------------------------------------

  Sound Effects

         WAVE (".WAV") files with the same name as the PNG. (e.g.,
         "picture.png"'s sound effect is the sound "picture.wav" in the same
         directory.)

    Language Support

           For sounds for different locales (e.g., if the sound is someone
           saying a word, and you want translated versions of the word said),
           also create WAV files with the locale's label in the filename, in
           the form: "STAMP_LOCALE.wav"

           "picture.png"'s sound effect, when Tux Paint is run in Spanish
           mode, would be "picture_es.wav". In French mode, "picture_fr.wav".
           And so on...

           If no localized sound effect can be loaded, Tux Paint will attempt
           to load the 'default' sound file. (e.g., "picture.wav")

   --------------------------------------------------------------------------

  Stamp Options

         Aside from a graphical shape, a textual description, and a sound
         effect, stamps can also be given other attributes. To do this, you
         need to create a 'data file' for the stamp.

         A stamp data file is simply a text file containing the options.

         The file has the same name as the PNG image, but a ".dat" extension.
         (e.g., "picture.png"'s data file is the text file "picture.dat" in
         the same directory.)

    Colored Stamps

           Stamps can be made to be either "colorable" or "tintable."

      Colorable

             "Colorable" stamps they work much like brushes - you pick the
             stamp to get the shape, and then pick the color you want it to
             be. (Symbol stamps, like the mathematical and musical ones, are
             an example.)

             Nothing about the original image is used except the transparency
             ("alpha" channel). The color of the stamp comes out solid.

             Add the word "colorable" to the stamp's data file.

      Tinted

             "Tinted" stamps are similar to "colorable" ones, except the
             details of the original image are kept. (To put it technically,
             the original image is used, but its hue is changed, based on the
             currently-selected color.)

             Add the word "tintable" to the stamp's data file.

             Sometimes you don't want the white or gray parts of an image
             tinted (see for example the dry erase marker stamp in the
             default stamp package). You can add the word "notintgray" to the
             stamp's data file to accomplish this. Only areas with saturation
             over 25 % are then tinted.

    Unalterable Stamps

           By default, a stamp can be flipped upside down, shown as a mirror
           image, or both. This is done using the control buttons below the
           stamp selector, at the lower right side of the screen in
           Tux Paint.

           Sometimes, it doesn't make sense for a stamp to be flippable or
           mirrored; for example, stamps of letters or numbers. Sometimes
           stamps are symmetrical, so letting the user flip or mirror them
           isn't useful.

           To make a stamp un-flippable, add the option "noflip" to the
           stamp's data file.

           To keep a stamp from being mirrored, add the option "nomirror" to
           the stamp's data file.

    Initial Stamp Size

           By default, Tux Paint assumes that your stamp is sized
           appropriately for unscaled display on a 608x472 canvas. This is
           the original Tux Paint canvas size, provided by a 640x480 screen.
           Tux Paint will then adjust the stamp according to the current
           canvas size and, if enabled, the user's stamp size controls.

           If your stamp would be too big or too small, you can specify a
           scale factor. If your stamp would be 2.5 times as wide (or tall)
           as it should be, add the option "scale 40%" or "scale 5/2" or
           "scale 2.5" or "scale 2:5" to your image. You may include an "="
           if you wish, as in "scale=40%".

    Windows Users

           You can use NotePad or WordPad to create these file. Be sure to
           save it as Plain Text, and make sure the filename has ".dat" at
           the end, and not ".txt"...

  Pre-Mirrored Images

         In some cases, you may wish to provide a pre-drawn version of a
         stamp's mirror-image. For example, imagine a picture of a fire truck
         with the words "Fire Department" written across the side. You
         probably do not want that text to appear backwards when the image is
         flipped!

         To create a mirrored version of a stamp that you want Tux Paint to
         use, rather than mirroring one on its own, simply create a second
         ".png" graphics file with the same name, except with the string
         "_mirror" before the filename extension.

         For example, for the stamp "truck.png" you would create another file
         named "truck_mirror.png", which will be used when the stamp is
         mirrored (rather than using a backwards version of 'truck.png').

   --------------------------------------------------------------------------

Fonts

       The fonts used by Tux Paint are TrueType Fonts (TTF).

       Simply place them in the "fonts" directory. Tux Paint will load the
       font and provide four different sizes in the 'Letters' selector when
       using the 'Text' tool.

   --------------------------------------------------------------------------

'Starters'

       'Starter' images appear in the 'Open' dialog, along with pictures
       you've created. They have a green button background, instead of blue.

       Unlike your saved pictures, however, when you select and open a
       'starter,' you're actually creating a new drawing. Instead of being
       blank, though, the new drawing contains the contents of the 'starter.'
       Additionally, as you edit your new picture, the contents of the
       original 'starter' affect it.

       Coloring-Book Style

         The most basic kind of 'starter' is similar to a picture in a
         coloring book. It's an outline of a shape which you can then color
         in and add details to. In Tux Paint, as you draw, type text, or
         stamp stamps, the outline remains 'above' what you draw. You can
         erase the parts of the drawing you made, but you can't erase the
         outline.

         To create this kind of 'starter' image, simply draw an outlined
         picture in a paint program, make the rest of the graphic transparent
         (that will come out as white in Tux Paint), and save it as a PNG
         format file.

       Scene-Style

         Along with the 'coloring-book' style overlay, you can also provide a
         separate background image as part of a 'starter' picture. The
         overlay acts the same: it can't be drawn over, erased, or affected
         by 'Magic' tools. However, the background can be!

         When the 'Eraser' tool is used on a picture based on this kind of
         'starter' image, rather than turning the canvas white, it returns
         that part of the canvas to the original background picture.

         By creating both an overlay and a background, you can create a
         'starter' which simulates depth. Imagine a background that shows the
         ocean, and an overlay that's a picture of a reef. You can then draw
         (or stamp) fish in the picture. They'll appear in the ocean, but
         never 'in front of' the reef.

         To create this kind of 'starter' picture, simply create an overlay
         (with alpha transparency) as described above, and save it as a PNG.
         Then create another image (without transparency), and save it with
         the same filename, but with "-back" appended to the name. (e.g.,
         "reef-back.png" would be the background ocean picture that
         corresponds to the "reef.png" overlay, or foreground.)

       The 'starter' images should be the same size as Tux Paint's canvas. In
       the default 640x480 mode, that is 448x376 pixels. If you're using
       800x600 mode, it should be 608x496. (It should be 192 pixels less
       wide, and 104 pixels less tall than the resolution.)

       Place them in the "starters" directory. When the 'Open' dialog is
       accessed in Tux Paint, the 'starter' images will appear at the
       beginning of the list with a green background.

       Note: 'Starters' can't be saved over from within Tux Paint, since
       loading a 'starter' is really like creating a new image. (Instead of
       being blank, though there's already something there to work with.) The
       'Save' command simply creates a new picture, like it would if the
       'New' command had been used.

       Note: 'Starters' are 'attached' to saved pictures, via a small text
       file that has the same name as the saved file, but with ".dat" as the
       extension. This allows the overlay and background, if any, to continue
       to affect the drawing even after Tux Paint has been quit, or another
       picture loaded or started. (In other words, if you base a drawing on a
       'starter' image, it will always be affected by it.)

   --------------------------------------------------------------------------

                                Further Reading

     Other documentation included with Tux Paint (in the "docs"
     folder/directory) include:

       * AUTHORS.txt
         List of authors and contributors
       * CHANGES.txt
         Summary of changed between releases
       * COPYING.txt
         Copying license (The GNU General Public License)
       * INSTALL.txt
         Instructions for compiling/installing, when applicable
       * OPTIONS.html
         Detailed instructions on command-line and configuration-file
         options, for those who don't want to use Tux Paint Config.
       * PNG.txt
         Notes on creating PNG format images for use in Tux Paint

   --------------------------------------------------------------------------

                                How to Get Help

     If you need help, feel free to contact New Breed Software:

       http://www.newbreedsoftware.com/

     You may also wish to participate in the numerous Tux Paint mailing
     lists:

       http://www.newbreedsoftware.com/tuxpaint/lists/
