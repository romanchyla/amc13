#!/usr/bin/perl
#
# generate trigger mask bits 0x1000-0x1017
#

for( my $amc=1; $amc<=12; $amc++) {
    for( my $bit=0; $bit<8; $bit++) {
	$addr = 0x1000 + ($amc-1)/4 + 3*$bit;
	$lo = (8*(($amc-1) % 8)) % 32;
	$hi = $lo + 7;
	printf qq{0x%08x,%d,%d,"rw","CONF.AMC%02d.BIT%d.TRIGGER_MASK","HCAL Trigger mask AMC%02d Bit %d",,,0,,"HCAL_Trigger_Mask","Bit %d","AMC%02d"\n},
	$addr, $lo, $hi, $amc, $bit,$amc,$bit,$bit,$amc;
    }
}
