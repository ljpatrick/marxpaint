#!/bin/sh

# tuxpaint-import

# "Tux Paint Import"
# Import an arbitrary GIF, JPEG or PNG into Tux Paint

# by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.newbreedsoftware.com/tuxpaint/

# September 21, 2002 - February 26, 2005


TMPDIR=/tmp
SAVEDIR=$HOME/.tuxpaint/saved


if [ $# -eq 0 ]; then
  # No arguments provided (sorry, you can't pipe into this script's stdin!)
  echo "Usage: tuxpaint-import filename(s)"
  echo "       tuxpaint-import --help"
  exit
fi

if [ $1 = "--help" ]; then
  # --help, show usage:
  echo
  echo "tuxpaint-import"
  echo
  echo "Imports an arbitrary image (GIF, JPEG, PNG, etc. format)"
  echo "into Tux Paint (see: tuxpaint(1)) so that it appears in the"
  echo "'Open' dialog."
  echo 
  echo "Usage: tuxpaint-import filename(s)"
  echo "       tuxpaint-import --help"
  echo
  exit
fi


# Make sure savedir exists!
if [ ! -d $SAVEDIR ]; then
  echo "Creating $SAVEDIR"
  mkdir -p $SAVEDIR
fi

# Make sure savedir thumbs directory exists!
if [ ! -d $SAVEDIR/.thumbs ]; then
  echo "Creating $SAVEDIR/.thumbs"
  mkdir -p $SAVEDIR/.thumbs
fi


# For each picture list...
for i in $*
do
  if [ -e $i ]; then
    # Determine a filename for it:
    NEWFILENAME=`date "+%Y%m%d%H%M%S"`
    echo "$i -> $SAVEDIR/$NEWFILENAME.png"

    # Convert and scale down, save as a temp file:
    anytopnm $i | pnmscale -xysize 448 376 > $TMPDIR/$NEWFILENAME.ppm
    
    # Place inside the correctly-sized canvas:
    # FIXME: Center, instead of placing at upper right
    ppmmake "#FFFFFF" 448 376 \
    | pnmpaste -replace $TMPDIR/$NEWFILENAME.ppm 0 0 \
    | pnmtopng > $SAVEDIR/$NEWFILENAME.png
    
    # Remove temp file:
    rm $TMPDIR/$NEWFILENAME.ppm

    # Create thumbnail for 'Open' dialog:
    pngtopnm $SAVEDIR/$NEWFILENAME.png | pnmscale -xysize 92 56 \
    | pnmtopng > $SAVEDIR/.thumbs/$NEWFILENAME-t.png
    
  else
    # File wasn't there!
    echo "$i - File not found"
  fi
done
