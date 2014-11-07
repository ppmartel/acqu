#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;


&main;

sub main {

  my $e = readin($ARGV[1] || 'data/Detector-NaI.dat');
  #print Dumper($e);

  for my $elem (keys %$e) {
    my @distances;
    for my $other (keys %$e) {
      #printf("%03d %.1f\n",
      #       $elem,
      #       calc_distance($e->{$elem}->{xyz},
      #                     $e->{$next}->{xyz})
      #       );
      push(@distances,
           [$other,
            calc_distance($e->{$elem}->{xyz},
                          $e->{$other}->{xyz})]);
    }
    @distances = sort {$a->[1] <=> $b->[1]} @distances;

    
    my @expected = @{$e->{$elem}->{next}};
    my @closest = map {$_->[0]} @distances[0..$#expected];

    # compute difference
    my(%count, @diff);
    foreach my $i (@expected, @closest) {
      $count{$i}++;
    }
    foreach my $i (keys %count) {
      push(@diff, $i) if $count{$i}==1;
    }
    

    if(@diff != 0) {
    
      printf("%03d %s\n",
             $elem,
             join(" ", sort @closest)
            );
      printf("%03d %s\n",
             $elem,
             join(" ", sort @expected)
            );
    }
  } 
}

sub calc_distance {
  my $xyz1 = shift;
  my $xyz2 = shift;
  return sqrt(
              ($xyz1->[0] - $xyz2->[0])**2 +
              ($xyz1->[1] - $xyz2->[1])**2 +
              ($xyz1->[2] - $xyz2->[2])**2
             );
}

sub readin {
  my $filename = shift;
  my $e = {};
  open(my $fh, "<$filename") or die "Can't open $filename: $!";
  my $n = 0;
  my $m = 0;
  while(my $line = <$fh>) {
    chomp $line;
    my @items = split(/\s+/,$line);
    if($line =~ /^Element:/) {      
      $e->{$m}->{xyz} = [ @items[11..13] ];
      $m++;
    }
    elsif($line =~ /^Next-Neighbour:/) {
      $e->{$n}->{next} = [ @items[2..$#items] ];
      $n++;
    }
  }

  return $e;
}

