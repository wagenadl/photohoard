#!/usr/bin/perl -w

use strict;

use DBI;

my $dbname = shift @ARGV or usage();
usage() if @ARGV;

my $dbh = opendb($dbname);

system("cp $dbname $dbname.bk");
$dbh->prepare("begin transaction")->execute();
while (<STDIN>) {
  chomp;
  /^(.*?) (.*?) + (\d+)$/ or die "Cannot parse line $_\n";
  my $dst = $1;
  my $src = $2;
  my $cnt = $3;
  $dst .= $src if $dst =~ /:$/;
  next if $dst eq "*";
  my @bits = split(/\s*:\s*/, $dst);
  ensureExists($dbh, @bits);
}
$dbh->prepare("commit transaction")->execute();

######################################################################

sub ensureExists {
  my $dbh = shift;
  my @bits = @_;
  print "Ensuring existence of ", join(":", @bits), "\n";

  my $cc = chainfind($dbh, @bits);
  unless (scalar keys %$cc) {
    my $leaf = pop @bits;
    if (scalar @bits) {
      ensureExists($dbh, @bits);
      my $cc = chainfind($dbh, @bits);
      die "Failed to create parents\n" unless scalar keys %$cc;
      my @cc = keys %$cc;
      my $pid = shift @cc;
      print "Creating $leaf in ", join(":", @bits), "\n";
      my $txt = 'insert into tags (tag, parent) values("' .
        $leaf . '", ' . $pid . ')';
      print "::[ $txt ]::\n";
      my $sth = $dbh->prepare($txt);
      die unless $sth->execute();
    } else {
      print "Creating $leaf as root\n";
      my $sth = $dbh->prepare('insert into tags (tag) values("' .
                              $leaf . '")');
      die unless $sth->execute();
    }
  }
}

sub findAll {
  # $ids = findAll($dbh, $tag) returns a set.
  my $dbh = shift;
  my $tag = shift;
  my $sth = $dbh->prepare('select id from tags where tag like "' .
                          $tag . '" collate nocase');
  die unless $sth->execute();
  my %ids;
  while (my @row = $sth->fetchrow_array()) {
    $ids{$row[0]} = 1;
  }
  return \%ids;
}

sub findAbbreviated {
  # $ids = findAbbreviated($dbh, $tag) returns a set.
  my $dbh = shift;
  my $tag = shift;
  my $sth = $dbh->prepare('select id from tags where tag like "' .
                          $tag . '%" collate nocase');
  die unless $sth->execute();
  my %ids;
  while (my @row = $sth->fetchrow_array()) {
    $ids{$row[0]} = 1;
  }
  return \%ids;
}

sub findAllOrAbbreviated {
  # $ids = findAllOrAbbreviated($dbh, $tag) returns a set.
  my $dbh = shift;
  my $tag = shift;
  my $ids = findAll($dbh, $tag);
  return $ids if scalar keys %$ids;
  return findAbbreviated($dbh, $tag);
}

sub parent {
  # $p = parent($dbh, $tagid)
  my $dbh = shift;
  my $id = shift;
  my $sth = $dbh->prepare("select parent from tags where id==$id");
  die unless $sth->execute();
  my @row = $sth->fetchrow_array();
  die unless @row;
  return $row[0];
}

sub chainfind {
  # $ids = chainfind($dbh, @taghierarchy)
  my $dbh = shift;
  my @taghierarchy = @_;
  my %ids;
  return \%ids unless @taghierarchy;
  my $leaf = pop @taghierarchy;
  my $cc = findAllOrAbbreviated($dbh, $leaf);
  return $cc unless @taghierarchy;
  my $pp = chainfind($dbh, @taghierarchy);
  return \%ids unless %{$pp};
  for my $c (keys %$cc) {
    $ids{$c} = 1 if exists($pp->{parent($dbh, $c)});
  }
  return \%ids;
}

sub usage {
  print STDERR "Usage: importxmp.pl databasefile.db\n";
  exit 1;
}

sub opendb {
  # $dbh = opendb("dbfile")
  my $dbfile = shift;
  my $dbh = DBI->connect("DBI:SQLite:dbname=$dbfile");
  return $dbh if $dbh;
  die "Could not open database '$dbname': $DBI::errstr\n";
}

