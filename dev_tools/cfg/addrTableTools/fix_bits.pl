#!/usr/bin/perl
#
# process things like this from Wu
# and produce CSV output
#
# input:
#		0x20	AMC DAQ_Link ststus
#			bit 31-16 Always 0
#			bit 15 FIFO_ovf
#			bit 14-13 Always 0
#			bit 12-8 EventStatus_ra
#			bit 7-6 Always 0
#			bit 4-0 EventStatus_wa
#
# output
# "0x20",16,31,"r","Always 0"
# "0x20",15,15,"r","FIFO_ovf"
#

while( $line = <>) {
    chomp $line;
    if( $line =~ /^\s*0x/) {
	($addr,$rname) = $line =~ /^\s*(0x\S+)\s+(.*)$/;
    }
    if( $line =~ /^\s*bit\s+\d/) {
	($bits,$name) = $line =~ /^\s*bit\s+(\S+)\s+(.*)$/;
	($hi,$lo) = $bits =~ /(\d+)-(\d+)/;
	printf qq{"%s",%d,%d,"r","%s: %s"\n}, $addr, $lo, $hi, $rname, $name;
    }
}
