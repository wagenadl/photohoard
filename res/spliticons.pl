#!/usr/bin/perl -w

use strict;

my @icons = qw/colorNone colorRed colorYellow colorGreen colorBlue colorPurple layoutFull layoutGrid layoutHGrid layoutVGrid layoutHLine layoutVLine scaleA2 scaleA3 scaleA4 scaleA5  scaleB2 scaleB3 scaleB4 scaleB5 scaleB6 scaleB7 scaleB8 scaleB9 scaleB10 scaleB11 scaleB12 scaleB13 cameraImport folderAdd folderAdd2 export searchSquares searchLines  cameraImport-1 folderAdd-1 folderAdd2-1 export-1 searchSquares-1 searchLines-1 searchLines2-1/;

if (@ARGV) {
  @icons = @ARGV;
}

my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
    $atime,$mtime,$ctime,$blksize,$blocks) = stat("icons.svg");
my $t0 = $mtime;

for my $id (@icons) {
  if (-f "icons/$id.svg") {
    my ($dev,$ino,$mode,$nlink,$uid,$gid,$rdev,$size,
	$atime,$mtime,$ctime,$blksize,$blocks)
      = stat("icons/$id.svg");
    next if $mtime>$t0;
  }
  system("inkscape --export-id=$id --export-id-only --export-pdf=icons/$id.pdf icons.svg") and die;
  system("inkscape --export-plain-svg=icons/$id.svg icons/$id.pdf") and die;
  system("rm icons/$id.pdf");
#  system("inkscape --export-id=$id --export-id-only --export-png=icons/$id.png --export-dpi=90 icons.svg") and die;
}

exit 0;
