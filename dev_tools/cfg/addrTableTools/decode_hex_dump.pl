#!/usr/bin/perl -w
#
# read a hex dump and display as address table items
#
# usage: decode_hex_dump address_tabl.xml dump_hex.txt
#
# THIS CODE IS CURRENTLY NOT WORKING

use strict;
use XML::Parser;

my $narg = $#ARGV+1;
if( $narg < 2) {
    print "Usage:  $0 <xml_address_table_file> <hex_dump_file>\n";
    exit;
}

my $dump_xml = 0;
my $do_includes = 1;
my $debug = 0;
my $ignore_nz = 1;


my $par = new XML::Parser ( Style => 'Debug', Handlers => {
    Start => \&p_start,
    End => \&p_end,
    Char => \&p_char,
    Default => \&p_def
			    });

# my $par = new XML::Parser ( Style => 'Debug');

my $xml_file = $ARGV[0];
my ($xml_path,$xml_name) = $xml_file =~ m{(.*)/([^/]*)$};

my $dump_file = $ARGV[1];

# read the dump into a hash
open DF, "< $dump_file" or die "opening hex dump file $dump_file";
my %t1;
my $start = 0;
while( my $line = <DF>) {
    chomp $line;
    if( $line =~ /^[0-9a-zA-Z]+:/) {
	my @d = split ' ', $line;
	die "Bad dump format line: $line" if( $#d < 1);
	$start = 1;
	my $daddr = substr $d[0], 0, -1;
	my $iaddr = hex $daddr;
	for( my $i=1; $i<=$#d; $i++) {
	    my $dword = $d[$i];
	    $daddr = sprintf "0x%08x", $iaddr;
#	    printf "Set %s = %s\n", $daddr, $dword;
	    $t1{$daddr} = $dword;
	    $iaddr += 1;
	}
    } elsif( $start == 1) {
	last;
    }
}

# read the XML file, processing "module" style includes
open XF, "< $xml_file" or die "opening XML file $xml_file";
my $xml_string = "";
while( my $line = <XF>) {
    chomp $line;
    if( $line =~ /module="file/) {
	# process the include
 	my ($file,$addr) = $line =~ /module=(.*) address=(.*) /;
	$xml_string .= "<!-- include $file -->\n";
	$file =~ s{"file://}{};
	$file =~ s{"}{}g;
	$addr =~ s{"}{}g;
	my $iaddr = hex $addr;
        print "Processing INCLUDE MODULE=$file OFFSET=$addr\n" if($debug);
	open INCL, "< $xml_path/$file" or die "Opening include file $file in $xml_path";
	while( my $iline = <INCL>) {
	    chomp $iline;
	    my $save = 0;
	    print "INCL($file) $iline\n" if($debug);
	    if( $iline =~ /address=/) {
		my ($ainc) = $iline =~ /address=\"(\w*)\"/;
		print "ADDRESS=$ainc\n" if($debug);
		my $newaddr = $iaddr + hex($ainc);
		my $newstr = sprintf "address=\"0x%x\"", $newaddr;
		$iline =~ s{address=\"(\w*)\"}{$newstr};
		$save = 1;
	    }
#	    if( $iline =~ /node id=/ or $iline =~ /\/node/) {
#		$save = 1;
#	    }
	    $xml_string .= "$iline\n" if($save && $do_includes);
	}
	close INCL;
	$xml_string .= "<!-- end include -->\n";
    } else {
	$xml_string .= "$line\n";
    }
}

#my $xml_string = join("", <XF>);
close XF;

print "---PARSE---\n $xml_string\n---END PARSE---\n" if($dump_xml);

$par->parse( $xml_string);

sub p_start {
    my( $p, $elt, %atts) = @_;
    my $id = $atts{"id"};
    if( $dump_xml) {
	print "--> ID: $id\n";
	foreach my $a ( sort keys %atts) {
	    my $av = $atts{$a};
	    print "    $a = $av\n";
	}
    }
    if( $atts{"address"} and $atts{"permission"}) {
	my $addr = $atts{"address"};
	my $mask = "0xffffffff";
	$mask = $atts{"mask"} if( $atts{"mask"});
	my $perm = $atts{"permission"};
	my $iaddr = hex $addr;
	my $imask = hex $mask;
	my $show = "";
	my $table = "";
	my $row = "";
	my $col = "";
	if( $atts{"parameters"}) {
	    my @par = split ';', $atts{"parameters"};
	    foreach my $pv ( @par ) {
		my @pset = split '=', $pv;
		$show = $pset[1] if( $pset[0] eq "Show");
		$table = $pset[1] if( $pset[0] eq "Table");
		$row = $pset[1] if( $pset[0] eq "Row");
		$col = $pset[1] if( $pset[0] eq "Column");
	    }
	}
	if( $t1{$addr}) {
	    my $rval = $t1{$addr};
	    my $ival = hex $rval;
	    my $mval = $ival & $imask;
	    # find shift for mask
	    my $nb = 0;
	    for( $b=1; $b<=0x80000000; $b<<=1) {
		last if($imask & $b);
		$nb++;
	    }
	    my $sval = $mval >> $nb;
	    if( ($perm =~ /r/) && ($ignore_nz || ($show ne "nz" || $sval != 0)) ) {
		printf "%30s  %s %s %3s  0x%08x  %3s", $id, $addr, $mask, $perm, $sval, $show;
		print " $table\[$row,$col\]" if( length($table) or length($row) or length($col));
		print "\n";
	    }
	}
    }
}

sub p_end {
    my( $p, $elt) = @_;
}

sub p_char {
    my( $p, $str) = @_;
}

sub p_def { }

