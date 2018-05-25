WHAT IS THIS
------------
This document describes how to build Tux Paint for macOS 10.12 Sierra and later.

Tux Paint 0.9.22 and earlier required building Tux Paint from the XCode IDE.
Starting with 0.9.23, however, Tux Paint for macOS is built as though it were a
Linux application.


REQUIREMENTS
------------
Although Tux Paint is built without the XCode IDE, XCode itself is still required
to build Tux Paint.  Download it from the App Store, and launch it once to
accept its license agreements.  Also install XCode command line tools using the
command:

  xcode-select --install

Building Tux Paint also requires various libraries from MacPorts.  Install them
to the default /opt/local path according to the instructions found on their
website:

  https://www.macports.org/

As of this writing, the required libraries are:

  cairo
  fribidi
  lbzip2
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
copied to /Applications.  It also creates TuxPaint.dmg for distribution.


KNOWN BUGS
----------
On macOS 10.13 High Sierra:
- The mouse cursor appears with an invert-transparent background due to an
  issue with SDL1 on macOS 10.13.  A patch to SDL has been issued on May 16,
  2018 to fix this issue.  Until this patch is officially relased and becomes a
  part of the MacPorts package, SDL will need to be built from the source code
  that includes the patch to display the mouse cursor correctly under macOS 10.13.
  Here are more details:

  Patch - https://github.com/kanjitalk755/SDL/commit/0296d5e601a5deb5ce2f540a8eafd64dd22dbe69
  Source + patch - https://github.com/kanjitalk755/SDL/tree/forHighSierra
  Full discussion - https://bugzilla.libsdl.org/show_bug.cgi?id=4076


BACKWARD COMPATIBILITY
----------------------
Broadly speaking, a Mac binary built on macOS 10.12 Sierra (for example) runs
only on macOS 10.12 and later.  To compile a binary that can also execute on an
earlier version of macOS (say, 10.7 Lion and later), one of the following must
be done:

  (A) Pass the flag -mmacosx-version-min=10.7 to the compiler.
  (B) Or set the environment variable MACOSX_DEPLOYMENT_TARGET to 10.7

Tux Paint binary itself is built by doing (A) in the Makefile (by passing the
parameter to osx_ARCH_CFLAGS).  However, the MacPorts libraries used by Tux
Paint are not, so Tux Paint package itself will not run on any version of macOS
earlier than the macOS on which it is built.

To build the Tux Paint package that can run on earlier versions of macOS, the
MacPorts libraries also need to be built with either #1 or #2.  This is done by
configuring MacPorts to install all packages from their sources and build them
to run on macOS 10.7 and later:

  1. Install the MacPorts base normally.
  2. Before installing any MacPorts package, add the following settings to
     /opt/local/etc/macports/macports.conf:

     buildfromsource            always
     macosx_deployment_target   10.7

  3. Install all packages normally.

... then build Tux Paint normally.

Please note building MacPorts packages from the source takes significantly
longer than installing the prebuilt packages.

If you have already installed MacPorts' prebuilt packages, it is possible to
uninstall them, add the above configuration from step #2, then install them
again from the source.  The MacPorts webpage on migration explains how uninstall
packages and reinstall them after making system changes:

  https://trac.macports.org/wiki/Migration

Even when Tux Paint and MacPorts are built to run on a specific version of
macOS, it is possible Tux Paint will not compile for or run on that version of
macOS.  As of this writing, 10.7 is the oldest version of macOS that can be
targetted without errors when compiling the sources of MacPorts libraries
required by Tux Paint.

21st May 2018
Mark K. Kim <mkkim214@gmail.com>
