#!/usr/bin/perl
#
# find all AMC13s in a crate, report serial number, IP, versions
# requires environment setup so that working AMC13Tool.exe is in the path
# also requires it be run from the 'amc13Config' directory so that
# the python tools mmcVersion.py etc can be found
#
# E. Hazen - 2014-04-08
# 2014-10-08 - port to AMC13Tool2 (requires environment variable
#              AMC13_ADDRESS_TABLE_PATH set correctly)

require File::Temp;

my $mch_opt = "";

if( $#ARGV == 0) {
    $mch_opt = "-o $ARGV[0]";
}


my $debug = 0;

# need to find a set of Python tools... default is just current dir
$tool_path = ".";

$lo_slot = 1;
$hi_slot = 13;

my @slot_status;

# loop over slots
for( $slot=$lo_slot; $slot<=$hi_slot; $slot++) {
    $slot_status[$slot] = 0;	# default to happiness for this slot

    # first get the MMC version by running python tool
    open (RMMC, "$tool_path/mmcVersion.py $mch_opt --slot=$slot 2>&1 |") or die
	"Looking for mmcVersion.py in $tool_path";
    $maj = 0;
    $min = 0;
    while( $line = <RMMC>) {
	chomp $line;
	# yow!  match hex strings (must be a better way)
	if( $line =~ m{^\s*[0-9a-zA-Z][0-9a-zA-Z]\s+[0-9a-zA-Z][0-9a-zA-Z]\s*$}) {
	    ($smaj, $smin) = $line =~ m{^\s*([0-9a-zA-Z][0-9a-zA-Z])\s+([0-9a-zA-Z][0-9a-zA-Z])\s*$};
	    $maj = hex $smaj;
	    $min = hex $smin;
	}
    }
    close RMMC;
    # format MMC version nicely
    $mmcv = sprintf( "%-6s", sprintf("%d.%d", $maj, $min));

    # AMC13 must have at least 2.1
    if( $maj >= 2 && $min >= 1) {
	# now run readIPs
	open (RIPS, "$tool_path/readIPs.py $mch_opt --slot=$slot 2>&1 |") or die
	    "Looking for readIPs.py in $tool_path";
	$ips = "";
	while( $line = <RIPS>) {
	    chomp $line;
	    if( $line =~ m{\[\s*\d+\s*,}) {
		($b1,$b2,$b3,$b4) = $line =~ m{\[\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*,\s*(\d+)\s*\]};
		$ips .= " " . sprintf( "%16s", sprintf( "%d.%d.%d.%d", $b1, $b2, $b3, $b4));
	    }
	}
	close RIPS;

	# now try to get the serial number and firmware versions
	# for this we need a temporary script to run AMC13Tool.exe
	$sip = substr $ips, 1, 16;
	($fh, $fname) = File::Temp::tempfile();
	print $fh "list\n";
	print $fh "q\n";
	close $fh;
	print "Opening $sip...\n";
	open( AMC, "AMC13Tool2.exe -c $sip -X $fname 2>/dev/null |") or die
	    "opening \"AMC13Tool2.exe\" (maybe you need to source env.sh to set the path?)";
	# set all to 0 in case they aren't found
	$serno = "0x0";
	$vver = "0x0";
	$sver = "0x0";
	my @stuff;
	while( $line = <AMC>) {
	    chomp $line;
	    print "AMC: $line\n" if( $debug);
	    push @stuff, $line;
	    if( $line =~ /\*0\:/) {
		print "AMC[match SN]: $line\n" if($debug);
		($serno,$vver,$sver) = $line =~ /\sSN\:\s+(\S+)\s+T1v\:\s+(\S+)\s+T2v\:\s+(\S+)\s/;
		print "AMC serno=$serno vver=$vver  sver=$sver\n" if($debug);
 	    }
	    if( $line =~ /Caught/) {
		close AMC;
		print "AMC13Tool2 threw an exception\n";
		foreach my $s ( @stuff) {
		    print "$s\n";
		}
	    }
	    $slot_status[$slot] = 2;
	}	    
	close AMC;
	# finally, output the formatted stuff
	printf "%2d: MMC: %s IP: %s vv: 0x%04x sv: 0x%04x sn: %d\n",
	$slot, $mmcv, $ips, hex($vver), hex($sver), $serno;
    } else {
	if( $maj == 0 && $min == 0) {
	    $mmcv = "-none-";
	}
	printf "%2d: MMC: %s\n", $slot, $mmcv;
	$slot_status[$slot] = 1; # bad MMC version
    }
}

