#!/bin/sh

intltool-update --pot
for i in *.po ; do
  msgmerge $i tuxpaint.pot > tmp.po && cat tmp.po > $i
done
cd ..
intltool-merge -d -u po tuxpaint.desktop.in tuxpaint.desktop
cd po
