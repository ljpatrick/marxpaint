Building Tux Paint using MSVC++6.0
==================================

Compiling Tux Paint
-------------------
You will need the tuxpaint-devel-win32.zip archive which contains the Release
and Debug .DLL and .LIB files and all headers required to build Tux Paint on 
Windows.

The source-code and binaries are available here:
[somewhere on the Tux Paint web site]

Then set up an area of your disk something like this:

C:\dev\tuxpaint\        # cvs files here
C:\dev\tuxpaint-config\ # cvs files here
C:\dev\lib\             # release dll, lib
C:\dev\libd\            # debug dll, lib
C:\dev\include\         # header files

You need Python 2.3.3 or later installed (might work with earlier versions).
Copy 'iconv.dll', 'libintl.dll' and 'msgfmt.exe' into the visualc directory.
Using a DOS box or Command-Prompt cd into visualc dir and run 
'python prebuild.py'. This will format the text documentation, build the
.mo I18N files, and build a filelist used by the NSIS installer.

Copy the following DLLs from lib/libd into Release/Debug output
directories respectively:

iconv.dll
jpeg.dll
libintl.dll
libpng1.dll
SDL.dll
sdl_image.dll
SDL_mixer.dll
SDL_ttf.dll
smpeg.dll
zlib.dll

Double click on the 'TuxPaint.dsw' workspace file which will start-up
Visual Studio.

Select 'Batch Build...' from the 'Build' menu and click on "Rebuild All".

If you want to run Tux Paint from inside Visual Studio (usefull!) then you
need to edit tuxpaint.c at around line 139:

/* Set this to 0 during developement and testing in Visual-Studio
   Set this to 1 to make the final executable   */

#if 1

#define DOC_PREFIX "docs/"
#define DATA_PREFIX "data/"
#define LOCALEDIR "locale"

#else

#define DOC_PREFIX  "../../docs/"
#define DATA_PREFIX "../../data/"
#define LOCALEDIR   "../../locale"

#endif /* 1/0 */

Before you use the NSIS installer make sure that this is changed back to a 1
and that you have rebuilt the project.

In addition, you need to set the working directory for the project: Select
Project|Settings from the menu and select the Debug tab (even if you want
to run the Release version). If you have already built the executables, you
will be able to cut-n-paste the path from the "Executable for debug session:"
box into the "" box.

Note: you can also supply command-line arguments to Tux Paint here - useful
for testing the different languages quickly.

Unfortunately, due to the layout of files in CVS, the stamps, starter
images, custom local fonts and locales, aren't in the right place to allow Tux 
Paint to find them. This will probably be fixed at some point, but for now just 
move the  folders about. DONT try and update cvs, and DONT try and build an 
installer if you have moved the folders. The technique I'm using at the moment
is to have a 'cvs' and a 'build' copy of the whole thing - ugly and error prone,
but it does allow testing and debugging to take place easily.


Compiling Tux Paint Config
--------------------------
This is the simpler of the two projects, and depends on the FLTK library,
which is statically linked, and 'libintl.dll' which depends on 'iconv.dll'.
In normal use, these DLLs are shared with Tux Paint itself.

Copy the following DLLs from lib/libd into Release/Debug output
directories respectively:

iconv.dll
libintl.dll

As before, use a DOS box to run the command 'python prebuild.py', then click
on the 'tuxpaintconfig.dsw' project workspace file.

Select 'Batch Build...' from the 'Build' menu and click on "Rebuild All".


NSIS
----
You need to have downloaded and installed NSIS-2 from here:
http://nsis.sourceforge.net/

The NSIS script, 'tuxpaint.nsi', requires that both tuxpaint and 
tuxpaint-config have already been compiled - I haven't put together any kind 
of 'Make' system here.

The easiest way of running the script is to right-click on it in the Windows
Explorer and choose "Compile NSIS Script". The compression defaults to 
LZMA, which consistantly gaves the best results (smallest installer).

The first few lines in 'tuxpaint.nsi' currently need manual editing to make 
sure that the installer has the right name (this keeps bill happy:-)

John Popplewell.
john@johnnypops.demon.co.uk
http://www.johnnypops.demon.co.uk/

