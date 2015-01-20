#!/usr/bin/perl

use strict;
use warnings;


# Variable setting
our $DEFAULT = 1;


my (%counter, %lister);
my $totalword    = 0;
my $functionword = 0;
my $notionalword = 0;
my $specialword  = 0;

# Load the word weights dictonary
#
my %weights;

open GENERAL, "< word.list"    or die print $!;
open SPECIAL, "< special.list" or die print $!;

while (<GENERAL>) {
	chomp;
	my ($k, $v) = split /,/;
	$weights{$k} = $v;
}
while (<SPECIAL>) {
	chomp;
	my ($k, $v) = split /,/;
	$weights{$k} = $v;
}


# Load the article file
#
while (<>) {
	chomp;
	foreach my $token (split) {
		if ($token =~ /([a-zA-Z0-9]+)/) {
			my $word = lc $1;

			$counter{$word} //= 0;
			$counter{$word}  += 1;

			$totalword += 1;
		}
	}
}

# Print the words term frequency table
#
print "============================================================\n";

foreach my $word (sort { $counter{$b} <=> $counter{$a} } keys %counter) {

	my ($flag, $w) = ("", $DEFAULT);
	my $tf = $counter{$word} / $totalword;

	printf "  %-16s  %-4s", $word, $counter{$word};

	if (exists $weights{$word}) {
		$w = $weights{$word};
		$flag = "FUNCTION" if $w == 0;
		$flag = "NOTIONAL" if $w > 10;
		$flag = "#SEPICAL" if $w > 30;
	}
	printf "  %-10s", $flag;
	printf "  %1.4f", $tf;
	printf "  %1.4f\n", $tf * $w;

	$lister{$word} = $tf * $w;
}

format DISPLAY = 

.

print "============================================================\n";
print "  total words: $totalword\n";
print "============================================================\n";


foreach my $word (sort { $lister{$b} <=> $lister{$a} } keys %lister) {
	printf "  %-14s  %1.4f\n", $word, $lister{$word};
}
