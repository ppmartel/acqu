#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;


&main;

sub main {

  my $e = readin($ARGV[1] || 'data/Detector-BaF2-PbWO4.dat');
  #print Dumper($e);

  for my $elem (sort {$a<=>$b} keys %$e) {
    #print "Elem $elem\n";
    #print Dumper($e->{$elem}->{next});
    for my $n (@{$e->{$elem}->{next}}) {
      #print "Checking $elem -> $n\n";
      if(!contains($e->{$n}->{next}, $elem)) {
        print "Elem $elem does not appear in list of neighbour $n\n";
        print "Neighbours of $elem: ",join(",",@{$e->{$elem}->{next}}),"\n";
        print "Neighbours of $n: ",join(",",@{$e->{$n}->{next}}),"\n";
      }
    }

  }
}

sub contains {
  my $arr = shift;
  my $elem = shift;
  for (@$arr) {
    if ($_ == $elem) {
      return 1;
    }
  }
  return 0;
}

sub readin {
  my $filename = shift;
  my $e = {};
  open(my $fh, "<$filename") or die "Can't open $filename: $!";
  my $n = 0;
  my $m = 0;
  while (my $line = <$fh>) {
    chomp $line;
    my @items = split(/\s+/,$line);
    if ($line =~ /^Element:/) {
      $e->{$m}->{xyz} = [ @items[11..13] ];
      $m++;
    } elsif ($line =~ /^Next-Neighbour:/) {
      $e->{$n}->{next} = [ @items[2..$#items] ];
      $n++;
    }
  }

  return $e;
}
