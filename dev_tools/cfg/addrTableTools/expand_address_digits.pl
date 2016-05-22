#!/usr/bin/perl
#
# expand hex addresses in first column to 4 digits for easy sorting
#

while( $line = <>) {
    chomp $line;
    if( $line =~ /^0x\w+,/) {
	($addr,$rest) = $line =~ /^(0x\w+),(.*)$/;
	printf "0x%08x,%s\n", hex($addr), $rest;
    } else {
	print "$line\n";
    }
}
