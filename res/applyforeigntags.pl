#!/usr/bin/perl -w

use strict;
use DBI;

my $dbname = shift @ARGV or usage();
usage() if @ARGV;

my $dbh = opendb($dbname);
my $dirs = listdirs($dbh);

my %tagmap;
while (<STDIN>) {
  chomp;
  /^(.*?) (.*?) + (\d+)$/ or die "Cannot parse line $_\n";
  my $dst = $1;
  my $src = $2;
  my $cnt = $3;
  $dst .= $src if $dst =~ /:$/;
  next if $dst eq "*";
  $tagmap{lc $src} = $dst;
}

my @cmds;

my $N = scalar(keys %{$dirs});
my $n=0;
for my $folderid (keys %{$dirs}) {
  my $path = $dirs->{$folderid};
  $n++;
  print STDERR "$n/$N: $path/             \r";
  my $vsns = versionsindir($dbh, $folderid);
  for my $vsn (keys %{$vsns}) {
    my $leaf = $vsns->{$vsn};
    my $xmps = findxmps("$path/$leaf");
    for my $xmp (@$xmps) {
      addtosql(\@cmds, $dbh, $vsn, $xmp);
    }
  }
}

$dbh->prepare("begin transaction")->execute();

for my $cmd (@cmds) {
  my @cmd = @{$cmd};
  my $kw = shift @cmd;
  if ($kw eq "tag") {
    my $ver = $cmd[0];
    my $oldtag = $cmd[1];
    if (exists($tagmap{lc $oldtag})) {
      my $newtag = $tagmap{lc $oldtag};
      my $cc = chainfind($dbh, split(":", $newtag));
      my @cc = keys %$cc;
      die "No tag for $oldtag -> $newtag" unless @cc;
      my $tag = shift @cc;
      die "Failed to apply tag\n" unless $dbh->prepare("insert into appliedtags (tag, version) values($tag, $ver)")->execute();
    } else {
      print "No tag for $oldtag\n";
    }
  }
}

$dbh->prepare("commit transaction")->execute();


######################################################################

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

sub listdirs {
  # $dirs = listdirs($dbh) returns a reference to a map of folderid to pathname
  # given a database handle.
  my $dbh = shift;
  my %dirs;

  my $sth = $dbh->prepare("select id, pathname from folders");
  die "Could not prepare select statement\n" unless defined $sth;
  my $rv = $sth->execute() or die "Could not list dirs: $DBI::errstr\n";
  die "Negative result from execute: $DBI::errstr\n" if $rv<0;
  
  while (my @row = $sth->fetchrow_array()) {
    my $id = $row[0];
    my $pth = $row[1];
    $dirs{$id} = $pth;
  }
  return \%dirs;
}

sub versionsindir {
  # $vsns = versionsindir($dbh, $folderid) returns a reference to a map of
  # versionid to leafname given a database handle and a folder handle.
  my $dbh = shift;
  my $folderid = shift;
  my %vsns;

  my $sth = $dbh->prepare("select versions.id, photos.filename" .
                          " from versions inner join photos" .
                          " on versions.photo==photos.id" .
                          " where photos.folder==$folderid");
  die "Could not prepare select statement\n" unless defined $sth;
  my $rv = $sth->execute() or die "Could not list versions: $DBI::errstr\n";
  die "Negative result from execute: $DBI::errstr\n" if $rv<0;
  
  while (my @row = $sth->fetchrow_array()) {
    my $id = $row[0];
    my $fn = $row[1];
    $vsns{$id} = $fn;
  }
  return \%vsns;
}

sub findxmps {
  # $xmps = findxmps("/path/file.jpg")
  # Returns a reference to a (possibly empty) list of xmp files associated
  # with the named file
  my $jpgfile = shift;
  my @xmps;
  push @xmps, "$jpgfile.xmp" if -f "$jpgfile.xmp";
  $jpgfile =~ s/\.(jpg|JPG|tif|TIF|tiff|TIFF|png|PNG)$//;
  push @xmps, "$jpgfile.xmp" if -f "$jpgfile.xmp";
  return \@xmps;
}

sub addTag {
  my ($vsn, $tag) = @_;
  return "add tag to $vsn: $tag";
}

sub addPTag {
  my ($vsn, $tag) = @_;
  return "add ptag to $vsn: $tag";
}

sub addtosql {
  # addtosql(\@cmds, $dbh, $vsn, "file.xmp")
  my $cmds = shift;
  my $dbh = shift;
  my $vsn = shift;
  my $xmpfile = shift;
  open XMP, "<$xmpfile" or die "Cannot open XMP file $xmpfile\n";
  while (<XMP>) {
    chomp;
    if (/<lr:hierarchicalSubject>/) {
      while (<XMP>) {
        chomp;
        last if /<\/lr:hierarchicalSubject>/;
        if (/<rdf:li>(.*)<\/rdf:li>/) {
          push @$cmds, ["tag", $vsn, $1];
        }
      }
    } elsif (/<dc:subject>/) {
      while (<XMP>) {
        chomp;
        last if /<\/dc:subject>/;
        if (/<rdf:li>(.*)<\/rdf:li>/) {
          push @$cmds, ["ptag", $vsn, $1];
        }
      }
    }
  }
  close XMP;
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
