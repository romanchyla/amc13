#!/usr/bin/perl
#
# load up a CSV file with quoted strings
#
use Text::CSV_XS qw( csv );

my $aoa = csv (in => $ARGV[0]);

foreach $row ( @{$aoa}) {
    foreach $col ( @{$row}) {
	print "$col |";
    }
    print "\n";
}
