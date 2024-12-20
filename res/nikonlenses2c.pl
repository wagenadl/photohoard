#!/usr/bin/perl -w

use strict;

#
# Refresh list using:
#
# wget http://www.rottmerhusen.com/objektives/lensid/files/exif/fmountlens.p.txt
#

open LENSES, "<fmountlens.p.txt" or die "Cannot read lens list\n";
my @code = <LENSES>;
close LENSES;
my $code = join("", @code);
my %nikonLensIDs = eval($code);
print $code;

open CODE, ">fmountlens.h" or die "Cannot write header file\n";
print CODE "// fmountlens.h - database of Nikon lenses\n";
print CODE "// Automatically generated by nikonlenses2c.pl\n";
print CODE "\n";

for my $k (keys %nikonLensIDs) {
  my $d = $nikonLensIDs{$k};
  $k =~ s/ //g;
  $k =~ s/\..*$//;
  print CODE "lenses.insert(0x$k, \"$d\");\n";
}
