#!/usr/bin/perl -w

use strict;

my @icons = qw/colorNone colorRed colorYellow colorGreen colorBlue colorPurple layoutFull layoutGrid layoutHGrid layoutVGrid layoutHLine layoutVLine scaleA2 scaleA3 scaleA4 scaleB2 scaleB3 scaleB4 scaleB5 scaleB6 scaleB7 scaleB8 scaleB9 scaleB10 scaleB11 scaleB12 scaleB13 cameraImport folderAdd folderAdd2/;

#@icons = qw/colorRed/;

for my $id (@icons) {
#  system("inkscape --export-id=$id --export-id-only --export-plain-svg=icons/$id.svg icons.svg") and die;
  system("inkscape --export-id=$id --export-id-only --export-pdf=icons/$id.pdf icons.svg") and die;
  system("inkscape --export-plain-svg=icons/$id.svg icons/$id.pdf") and die;
#  system("inkscape --export-id=$id --export-id-only --export-png=icons/$id.png --export-dpi=90 icons.svg") and die;
}

exit 0;
