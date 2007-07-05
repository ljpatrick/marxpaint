#!/bin/sh

# tp-magic-config

# "Tux Paint Magic Config"
# Tool that reports compiler options used when buidling Magic Tool
# shared objects for Tux Paint

# (c) Copyright 2007, by Bill Kendrick
# bill@newbreedsoftware.com
# http://www.tuxpaint.org/

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# (See COPYING.txt)

# Note: "__VERSION__" and "__INCLUDE__" are replaced by values
# in Tux Paint's Makefile, via 'sed', by the 'make install-magic-plugin-dev'
# target.

# July 5, 2007 - July 5, 2007


if [ $# -ne 0 ]; then
  if [ $1 = "--version" ]; then
    echo "__VERSION__"
    exit
  fi
  if [ $1 = "--cflags" ]; then
    echo `sdl-config --cflags` -I__INCLUDE__
    exit
  fi
  if [ $1 = "--libs" ]; then
    echo `sdl-config --libs`
    exit
  fi
fi

echo "Usage: tp-magic-config [--version] [--cflags] [--libs]"

