#!/bin/sh

for dll in `ldd tuxpaint.exe $1/tuxpaint-config.exe 2>&1 | grep /mingw | awk '{print $3}' | sort | uniq`
do
  cp $dll $2
done
