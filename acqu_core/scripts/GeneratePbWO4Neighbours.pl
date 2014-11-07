#!/usr/bin/perl
use strict;
use warnings;
use Data::Dumper;
use POSIX; # for ceil

&main;

sub main {
  my $datafile = $ARGV[1] || 'Detector-BaF2-PbWO4-2013.11.dat';
  my $e = readin($datafile);
  #print Dumper($e);

  #my $i = 60;
  #for my $j (@{$e->{$i}->{next}}) {
  #  print "Distance: $j ",
  #  calc_distance($e->{$i}->{xyz}, $e->{$j}->{xyz}),"\n";
  #}

  #exit;
  
  #print Dumper($e->{0});
  
  # modify the PbWO4 neighbours consistently
  for my $elem (sort keys %$e) {
    if(($elem % 73) >= 15) {
      next;
    }

    my @neighbours = @{$e->{$elem}->{next}};
    my @new_neighbours;
    my @removals;
    for my $n (@neighbours) {
      #printf("%03d %.1f\n",
      #       $elem,
      #       calc_distance($e->{$elem}->{xyz},
      #                     $e->{$next}->{xyz})
      #       );
      my $distance = calc_distance($e->{$elem}->{xyz},
                                   $e->{$n}->{xyz});
      #printf( "%d->%.1f ", $n, $distance);
      if($distance < 6.1 ) { # 6 cm is distance of usual BaF2...
        push(@new_neighbours, $n);
        next;
      }
      
      push(@removals, $n);
    }
    #print "\n";
    
    #@distances = sort {$a->[1] <=> $b->[1]} @distances;
    #my $last_element = $distances[$#distances];
    #print Dumper(\@distances),"\n";

    for my $r (@removals) {
      my @new;
      my @old = @{$e->{$r}->{next}};
      for my $i (@old) {
        if($i==$elem) {
          next;
        }
        push(@new, $i);
      }
      if(@new != @old-1) {
        print STDERR "Symmetric partner of $elem not found in $r!\n";
      }
      $e->{$r}->{next} = \@new;
    }

    $e->{$elem}->{next} = \@new_neighbours;
    
    #print "Before ", scalar @neighbours," After ",scalar @new_neighbours, "\n";
    
    #last;
    #$e->{distances} = \@distances;
  }

  #print Dumper($e->{0});
  
  #exit;

  
  my @lines_acqu = read_file($datafile);
  
  for my $line (@lines_acqu) {
    unless($line =~ /^Next-Neighbour:\s+\d+\s+(\d+)/) {
      print $line;
      next;
    }
    my $elem = $1;

    if(($elem % 73) >= 15) {
      print $line;
      next;
    }

    # a PbWO4 crystal

    
    #$number = ceil($number/2);
    my @closest = @{$e->{$elem}->{next}};
    
    my $neighbour_str = join('', map { sprintf("%5s",  $_) } @closest);

    printf("Next-Neighbour: %3s  %s\n",scalar @closest, $neighbour_str); 
    
  }

  
  
}

    sub read_file {
      my $file = shift;
      open(my $fh, "<$file") or die "can't open $file: $!";
      my @lines = <$fh>;
      close $fh;
      return @lines;
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

