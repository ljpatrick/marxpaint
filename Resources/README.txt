This folder exists to ease testing under macOS.  Please ignore but do not
delete.  More details are below.

macOS requires all files in an application have a specific folder structure.
We create this structure under the folder "TuxPaint.app" when `make install` is
run.  But `make install` takes a long time to run under macOS due to the large
number of files that need to be copied, as well as all the processing we do to
those files (see custom/macos.sh for what we need to do.)  This makes testing
even a small code change time consuming.

That's the problem this folder solves:  Instead of creating the folder
structure under TuxPaint.app every time to test a code change to be able to run
the application, we pre-build the folder structure around the tuxpaint binary
so the binary can be executed directly without re-creating the folder structure
every time.  Luckily for us, this only require that we only have a single
folder "Resources" in the root folder of the tuxpaint source code (with some
subfolders and symlinks underneath the Resources folder.)

I hope my fellow Tux Paint contributors will forgive me for taking up yet
another whole folder for use by a single platform, let alone almost empty, in
the root folder of the source code distribution meant for multiple platforms.

Mark Kim
2018.06.27
