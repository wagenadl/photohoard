#!/usr/bin/perl -w

use strict;
use DBI;

my $driver = "SQLite";

my $database = "/home/wagenaar/.local/photohoard/photodb.db";

my $dbh = DBI->connect("DBI:$driver:dbname=$database") or die $DBI::errstr;

my $sth = $dbh->prepare("select id, pathname from folders");
die unless defined $sth;
my $rv = $sth->execute() or die $DBI::errstr;
if ($rv<0) {
  print $DBI::errstr;
  exit 1;
}

while (my @row = $sth->fetchrow_array()) {
  my $id = $row[0];
  my $pth = $row[1];
  print "$id: $pth\n";
}

$dbh->disconnect();
