WHAT IS THIS
------------
This document describes how to build Tux Paint for macOS 10.12 Sierra and later.

Tux Paint 0.9.22 and earlier required building Tux Paint from the XCode IDE.
Starting with 0.9.23, however, Tux Paint for macOS is built as though it were a
Linux application.


PREREQUISITES
-------------
Although Tux Paint is built without the XCode IDE, XCode itself is still required
to build Tux Paint.  Download it from the App Store, and launch it once to
accept its license agreements.  Also install XCode command line tools using the
command:

  xcode-select --install

Building Tux Paint also requires various packages.  We install them from
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
  XCode, such as Fink or Brew, will prevent your app bundle from being
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


August 2, 2020
Mark K. Kim <markuskimius@gmail.com>
