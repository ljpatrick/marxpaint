Tux Paint Customization

This folder allows for customization of Tux Paint. One example is adding resources such as fonts, stamps, starters...

On Mac OS X, at the very end of the build, macos.sh script in this folder is run.

On Windows, the win32.bat in this folder has to be run manually. At some point, a calling to the script could be integrated in the build process so as to be run automatically, just like it is on Mac OS X (see above).

On Linux, it should be straightforward to replicate the process describe above on Mac OS X, and automatically call a linux.sh script, in this folder, from the build process.
