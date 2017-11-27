WHAT IS THIS
------------
This document describes how to build Tux Paint for macOS 10.12 Sierra and later.

Tux Paint 0.9.22 and earlier required building Tux Paint from the XCode IDE.
Starting with 0.9.23, however, Tux Paint for macOS is built as though it were a
Linux application.


REQUIREMENTS
------------
Although Tux Paint is run without the XCode IDE, XCode itself is still required
to build Tux Paint. Download it from the App Store, and launch it once to
accept its license agreements.

Building Tux Paint also requires various libraries from MacPorts.  Install them
to the default /opt/local path according to the instructions found on their
website:

  https://www.macports.org/

As of this writing, the required libraries are:

  cairo
  fribidi
  libpaper
  libpng
  librsvg
  libsdl
  libsdl_image
  libsdl_mixer
  libsdl_pango
  libsdl_ttf
  zlib

... but you should intall any package that is required by the latest version of
Tux Paint.


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
copied to /Applications.  Zip it up for distribution.

