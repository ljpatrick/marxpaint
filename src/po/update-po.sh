#!/bin/sh

cp POTFILES.in.in POTFILES.in
ls ../../magic/src/*.c | cut -b 4- >> POTFILES.in

intltool-update --pot
msguniq marxpaint.pot > temp.tmp && mv -f temp.tmp marxpaint.pot
for i in *.po ; do
  echo $i
  msgmerge --update --previous --backup=none $i marxpaint.pot
done
cd ..
intltool-merge -d -u po marxpaint.desktop.in marxpaint.desktop
cd po
