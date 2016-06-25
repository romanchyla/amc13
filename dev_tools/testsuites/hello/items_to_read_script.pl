#!/usr/bin/perl
#
# convert list of status items from 'nodes' command to a list of reads
#
while( $line = <>) {
    chomp $line;
    if( $line =~ /^\s+\d+:\s/) {
	my ($name) = $line =~ /^\s+\d+:\s+(\S+)\s/;
	print "rv $name\n";
    }
}
print "q\n";
