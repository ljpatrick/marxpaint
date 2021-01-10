WHAT IS THIS
------------
This document describes how to build Tux Paint 0.9.23 or later on macOS.

Tux Paint 0.9.22 and earlier required building Tux Paint from the Xcode IDE.
Starting with 0.9.23, however, Tux Paint for macOS is built as though it were a
Linux application.


PREREQUISITES
-------------
Although Tux Paint is built without the Xcode IDE, Xcode itself is still required
to build Tux Paint.  Download it from the App Store, and launch it once to
accept its license agreements.  You may also need to install the Xcode command
line tools using the command:

  xcode-select --install

Building Tux Paint also requires various libraries.  We install them from
MacPorts where possible, source code otherwise.  Install MacPorts to the
default /opt/local path according to the instructions found on their website:

  https://www.macports.org/

As of this writing, the following packages need to be installed:

  ImageMagick
  cairo
  fribidi
  lbzip2
  libimagequant*
  libpaper
  libpng
  librsvg
  libsdl
  libsdl_image
  libsdl_mixer
  libsdl_pango
  libsdl_ttf
  pkgconfig
  zlib

  * - Not available from MacPorts as of this writing, see below

... but you should install any package that is required by the latest version of
Tux Paint.

  libimagequant
  -------------
  libimagequant is not available from MacPorts as of this writing.  It can be
  installed from the source code as follows.  It should be installed to
  /opt/local (same as MacPorts) for the library to be included in TuxPaint.dmg.

    $ git clone https://github.com/ImageOptim/libimagequant.git
    $ cd libimagequant
    $ ./configure --prefix=/opt/local
    $ make
    $ sudo make install


  *** WARNING ***
  ---------------
  Having any UNIX-like toolset installed on your Mac besides MacPorts and
  Xcode, such as Fink or Brew, will prevent your app bundle from being
  portable.  Be sure Fink and Brew are not accessible from your build
  environment.


HOW TO BUILD
------------
Simply, run:

  % make
  % make install

... to create the TuxPaint.app application bundle that can be run in-place or
copied to /Applications.  It also creates TuxPaint.dmg for distribution.


KNOWN ISSUES
------------
- A macOS binary built on a specific version of macOS only runs on that version
  of macOS or later.  To ensure Tux Paint can run on the oldest version of
  macOS possible, build it on the oldest version of macOS available.  As of
  this writing we know Tux Paint cannot be built to run on macOS 10.7 or
  earlier.

  See "OLD VERSIONS OF MACOS" below for best-effort instructions on how to
  obtain, install, and build Tux Paint on an old version of macOS.


OLD VERSIONS OF MACOS
---------------------
Some old versions of macOS can be downloaded from Apple's support page:

  https://support.apple.com/en-us/HT211683

macOS does allow dual booting of multiple versions of the OS, but it's safer
and easier to install the old macOS onto a flash drive.  Wherever you're
installing it, the target drive's partitioniong scheme and partition type must
match what the old macOS expects, so use the Disk Utility to partition and
format the flash drive accordingly.

As of this writing, the oldest version of macOS available on Apple's support
site is Yosemite 10.10, which expects `GPT (GUID Partition Table)` partitioning
scheme instead of the older MBR scheme, and `Mac OS Extended (Journaled)` as
the partition type instead of the newer APFS partition type.

Upon launching the installer, if you get a popup about macOS being too old or
new to be installed, a bootable installer can be created using the instructions
found here:

  https://support.apple.com/en-mide/HT201372

I found that macOS can be installed onto the bootable media itself, so you can
make the flash drive into a bootable installer then install the old macOS onto
the same flash drive.

Once the old macOS is installed, you may find the Xcode on the App Store is too
new to run on the version of the old macOS.  Old versions of Xcode can be
downloaded from Apple's Developer site in an area accessible with free
registration:

  https://developer.apple.com/download/more/

The list of macOS versions and the last version of Xcode compatible with them
are laid out nicely on the Wikipedia page on Xcode:

  https://en.wikipedia.org/wiki/Xcode#Version_comparison_table

And because Xcode is being installed manually, you can skip the step to install the Xcode
command line tools (do not run `xcode-select --install`) but otherwise build Tux Paint
using the same steps described in the earlier part of this document.


January 9, 2021
Mark K. Kim <markuskimius@gmail.com>
