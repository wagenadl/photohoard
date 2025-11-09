#!/usr/bin/perl -w

use strict;
use DBI;

my $dbname = shift @ARGV or usage();
usage() if @ARGV;

my $dbh = opendb($dbname);
my $dirs = listdirs($dbh);

my %colormap = ( "Red" => 1, "Yellow" => 2, "Green" => 3, "Blue" => 4,
                 "Purple" => 5 );

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

my $Ncmd=0;
for my $cmd (@cmds) {
  my @cmd = @{$cmd};
  my $kw = shift @cmd;
  if ($kw eq "color") {
    print changeColor(@cmd), "\n";
    $Ncmd++;
  } else {
    die "UNKNOWN KW: $kw\n";
  }
}

print "Press return to apply the above list of commands (N=$Ncmd) to the database.\n";
print "(A backup will be made first.)\n";

my $a = <STDIN>;

print "Creating backup: $dbname.bk\n";
system("cp $dbname $dbname.bk") and die "Failed to create backup\n";

print "Executing SQL...\n";
for my $cmd (@cmds) {
  my @cmd = @{$cmd};
  my $kw = shift @cmd;
  if ($kw eq "color") {
    $dbh->prepare("begin transaction")->execute() or die "Could not start transaction\n";
    $dbh->prepare(changeColor(@cmd))->execute() or die "Could not set color\n";
    $dbh->prepare("commit transaction")->execute() or die "Could not commit transaction\n";
  } else {
    die "UNKNOWN KW: $kw\n";
  }
}

print "All done.\n";


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

sub parseColor {
  my $name = shift;
  my $num = $colormap{$name};
  return $colormap{$name};
}

sub changeColor {
  my ($vsn, $numclr) = @_;
  return "update versions set colorlabel=$numclr" .
    " where id==$vsn and (colorlabel is null or colorlabel==0)";
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
    if (/xmp:Label="([A-Za-z]+)"/) {
      my $clr = $1;
      my $numclr = parseColor($clr);
      if (defined $numclr) {
        push @$cmds, ["color", $vsn, $numclr];
      } else {
        print STDERR "Unknown color: $clr.\n";
      }
    }
  }
  close XMP;
}
