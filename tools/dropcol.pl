#!/usr/bin/perl

my $table = shift @ARGV;
my $colno = shift @ARGV;
my $ncols = shift @ARGV;
$colno -= 2;

while (<>) {
  if (/INSERT INTO \"$table\"/) {
    my @bits = split(/,/, $_);
    my @outb;
    if (defined($ncols) && scalar(@bits)!=$ncols) {
      print STDERR "Warning: Field count mismatch: “$_”\n";
    }
    push @outb, shift @bits for (0..$colno);
    shift @bits;
    push @outb, $_ for (@bits);
    print join(",", @outb);
  } else {
    print $_;
  }
}
