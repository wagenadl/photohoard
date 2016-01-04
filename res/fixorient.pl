#!/usr/bin/perl -w

use strict;
use DBI;

my $dbname = "/home/wagenaar/.local/photohoard/photodb.db";
my $dbh = opendb($dbname);

$dbh->prepare("begin transaction")->execute();

my $sth = $dbh->prepare("select photos.id, pathname, filename"
                        . " from folders inner join photos"
                        . " on folders.id==photos.folder");
die "Could not prepare select statement\n" unless defined $sth;
my $rv = $sth->execute() or die "Could not execute statement\n";
die "Negative result from execute: $DBI::errstr\n" if $rv<0;

while (my @row = $sth->fetchrow_array()) {
  my $id = $row[0];
  my $pth = $row[1];
  my $lf = $row[2];
  my $orient = getOrientation("$pth/$lf");
  if ($orient>0) {
    print "Updating orientation for $id: $pth/$lf\n";
    $dbh->prepare("update versions set orient=$orient"
                  ." where photo==$id and orient==0")->execute()
                    or die "Could not update orientation for photo $id\n";
  } else {
    print "Keeping orientation for $id.    \r";
  }
}

$dbh->prepare("commit transaction")->execute();

######################################################################

sub getOrientation {
  my $fn = shift;
  open EXIF, "exiftool \"$fn\" |" or die "Could not run exiftool on $fn\n";
  while (<EXIF>) {
    /Orientation/ or next;
    close EXIF;
    return 3 if /270/;
    return 1 if /90/;
    return 2 if /180/;
    return 0 if /normal/;
    return 0 if /Unknown/;
    die "I didn't recognize orientation $_\n";
  }
  close EXIF;
  return 0;
}

sub opendb {
  # $dbh = opendb("dbfile")
  my $dbfile = shift;
  my $dbh = DBI->connect("DBI:SQLite:dbname=$dbfile");
  return $dbh if $dbh;
  die "Could not open database '$dbname': $DBI::errstr\n";
}
