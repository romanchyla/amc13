#!/usr/bin/perl
#
# parse Wu's address map text file
# extract addresses and masks, check against XML
#
my $pending = 0;
my $hi;
my $lo;
my $addr = -1;

while( $line = <>) {
    chomp $line;
    $sfmt = sprintf("%80s  ", substr($line, 0, 80));
    if( $line =~ /chip memory map:/) {                # chip select
	($chip) = $line =~ /(\S+)\s+chip memory map:/;
	print "Select chip \"$chip\"\n";
    }
    if( $line =~ /^0x/) {			 # address item
	$sfmt .= "ADDR ";
	# flush old info
	if( $pending) {
#	    printf "0x%08x [%d-%d] %s\n", $addr, $hi, $lo, $name;
	    $pending = 1;
	}
	($taddr,$rest) = $line =~ /^(\S+)\s+(.*)/;
	$addr = hex $taddr;			 # numeric address
	$sfmt .= sprintf "%08x ", $addr;
	$hi = 31;
	$lo = 0;
	$name = sprintf "ADDR_0x%08x", $addr;	 # default name
	if( $rest eq "read/write" || $rest eq "read only") {
	    ;
	} else {
	    $name = $rest;
	}
	$sfmt .= sprintf "name=%s ", $name;
    }
    if( $addr >= 0) {
	if( $line =~ /^\s*read:/) {
	    ;
	}
	$print = 0;
	if( $line =~ /^\s*bits?\s+\d+-\d+/) {
	    ($hi,$lo,$desc) = $line =~ /^\s*bits?\s+(\d+)-(\d+)\s+(.*)/;
	    $print = 1;
	} elsif( $line =~ /^\s*bit\s+\d+/) {
	    ($hi,$desc) = $line =~ /^\s*bit\s+(\d+)\s+(.*)/;
	    $lo = $hi;
	    $print = 1;
	}
	$sfmt .= sprintf "bits %d-%d desc=%s ", $lo, $hi, $desc;
	if( $print) {
#	    print "WITH $line\n";
#	    printf "....0x%08x [%02d-%02d] %s\n", $addr, $hi, $lo, $desc;
	    $pending = 0;
	    $print = 1;
	}
    }
    print "$sfmt\n";
}
