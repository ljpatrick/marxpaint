#!/bin/sh

intltool-update --pot
msguniq tuxpaint.pot > temp.tmp && mv -f temp.tmp tuxpaint.pot
for i in *.po ; do
  echo $i
  msgmerge $i tuxpaint.pot > temp.tmp && mv -f temp.tmp $i
done
cd ..
intltool-merge -d -u po tuxpaint.desktop.in tuxpaint.desktop
cd po
