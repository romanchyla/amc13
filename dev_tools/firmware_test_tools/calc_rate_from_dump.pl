#!/usr/bin/perl
#
# use dump tool to measure trigger spacing and average rate
# from a data file
#

$dump_tool = "/home/hazen/work/arcaro/cms/trunk/tools/dump";
$dump_opts = "-v 2";

$orb_len = 3563;

$na = $#ARGV+1;

if( $na < 1) {
    printf"need a data file\n";
    exit;
}

$df = $ARGV[0];

open DF, "$dump_tool $dump_opts $df |" or die "opening dump pipe";

$orz = -1;
$tyme0 = -1;
$sum = 0;
$num = 0;

while( $line = <DF>) {
    chomp $line;
    if( $line =~ /^EvN/) {
	($hex_evn,$hex_bcn) = $line =~ /^EvN\s+(\S+).*BcN\s+(\S+)/;
	$evn = hex $hex_evn;
	$bcn = hex $hex_bcn;
	$orn = -1;
    }
    if( $line =~ /^\s+Blk 00/) {
	($hex_orn) = $line =~ /OrN=(\S+)/;
	$orn = hex $hex_orn;
	if( $orz == -1) {
	    $orz = $orn;
	}
	$tyme = ($orn-$orz)*$orb_len + $bcn;
	print "$hex_orn $hex_bcn -- ";
	if( $tyme0 != -1) {
	    $dt = $tyme - $tyme0;
	    printf "%06x %03x %d\n", ($orn-$orz), $bcn, $dt;
	    $sum += $dt;
	    $num += 1;
	}
	$tyme0 = $tyme;
    }
}

$mean = $sum / $num;
$rate = 1.0/($mean * 25e-9);

printf "Average rate is %8.2f Hz\n", $rate;

