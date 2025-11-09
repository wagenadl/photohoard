#!/bin/sh

cd icons

for a in *.svg; do
    b=`basename $a .svg`
    inkscape -w 128 --export-filename=../../docs/source/icons/$b.png $a
done
