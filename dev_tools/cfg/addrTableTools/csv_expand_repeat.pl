#!/usr/bin/perl
#
# read CSV file and expand using P_Repeat and P_Offset fields
# (eventually this will be handled in C++ code)
#
# assumes Address comes first in the CSV data
#

use Text::ParseWords;
use strict;

die "usage: csv_expand_repeat file1.csv" if( $#ARGV<1);
my $f1 = $ARGV[0];
my $f2 = $ARGV[1];

open EXP, "> $f2" or die "Opening $f2 for output";

my $debug = 0;

#-- load up the CSV file
my ($header, $rows, $a1,$c1) = load_csv_aoh( $f1);
my $na1 = @$a1;
my $nc1 = @$c1;
my $i;

print EXP "$header\n";

#-- traverse the rows looking for P_Repeat
for( $i=0; $i < $na1; $i++) {
    my $a = $a1->[$i];
    my $name = $a->{Name};
    my $raw = $rows->[$i];
    print "RAW=$raw\n" if($debug);
    if( $a->{P_Repeat}) {
	my $rep = $a->{P_Repeat};
	my $off = 1;
	if( $a->{P_Offset}) {
	    $off = $a->{P_Offset};
	}
	print "---Looping $rep times with offset $off\n" if($debug);
	my $addr = hex $a->{Address};
	my $fmt = "%d";
	if( $rep > 9) {
	    $fmt = "%02d";
	}
	if( $rep > 99) {
	    $fmt = "%03d";
	}
	# expand, replacing "_d" with formatted loop count in Name, P_Row, P_Column
	# increment address by 1 each time
	# if more than "$more" rows, increase P_Show by one to truncate output
	# 
	for( my $n=0; $n < $rep; $n++) {
	    print "---Loop $n\n" if($debug);
	    my $mod = $raw;
	    my $nfmt = sprintf $fmt, $n;
	    # substitute _d
	    $mod =~ s/_d/_$nfmt/g;
	    # strip the address and adjust it
	    my $fix = $mod;
	    my $hexa = sprintf "0x%08x", $addr;
	    $fix =~ s/^\"0x[[:xdigit:]]+/\"$hexa/; #s{^"0x[[:xdigit:]]*"}{"$hexa"};
	    print EXP "$fix\n";
	    #printf "%s ---- %s\n",$fix, $hexa;
	    print "---output: $fix\n" if($debug);
	    $addr += $off;
	}
    } else {
	print EXP "$raw\n";
    }
}



#
# reconstruct CSV from columns and rows data
#
sub new_csv {
    my $col = shift @_;
    my $row = shift @_;

    my $csv = "";

    print "--- new_csv( $col, $row)\n" if( $debug);


    foreach my $f ( @$col ) {
	my $v = $row->{$f};
	if( $v =~ /^\d+$/) { # is only digits?
	    $csv += qq[$v,];
	} else {
	    $csv += qq["$v",];
	}
	print "$f = $v\n" if( $debug);
    }

    $csv = substr( $csv, 0, -1); # strip trailing ,
    return $csv;
}


#------------------------------------------------------------
# ($header,$aor,$aoh,$aos) = load_csv_aoh( $file)
#    read a CSV file into an array of hash references
#
# skip lines beginning with '#' as comments
# parse first line as column headings
# parse subsequent lines and create a hash from each line
# using column headings as keys
#
# return a string and a list of 3 references
#   string with header
#   reference to array of raw strings as read from file
#   reference to array of hash references, one per row
#   reference to array of column names
#
# every line must have the same number of fields
# strings must be properly quoted with double quotes
#------------------------------------------------------------
sub load_csv_aoh {
    my $f = shift @_;
    my $aoh = [ ];		# array for hashrefs
    my $hdr = 1;
    my $line;
    my $c;
    my $nc;
    my $aor = [ ];		# array for rows
    my $header_str;

    open FP, "< $f" or die "opening $f";
    
    while( $line = <FP>) {
	chomp $line;

	print "PARSE: $line\n" if($debug);
	next if( $line =~ /^\s*#/); # skip hash comments
	my @d = quotewords( ',', 0, $line); # parse line
	my $nd = $#d+1;

	if( $debug) {
	    print "$nd columns:\n";
	    for( $i=0; $i<$nd; $i++) {
		printf "%d: %s\n", $i, $d[$i];
	    }
	}

	if( $hdr) {		# first line is column headings
	    # make a copy of the column headings
	    $header_str = $line;
	    $c = [@d];
	    $nc = $nd;
	    $hdr = 0;
	} else {		# non-heading
	    push @$aor, $line;

	    die "wrong number of fields ($nd vs $nc) in $line\n" if( $nd != $nc);
	    # make a hash by column names
	    my %hash;
	    for( $i=0; $i<$nd; $i++) {
		$hash{$c->[$i]} = $d[$i];
		printf "  field %d set %s = %s\n", $i, $c->[$i], $d[$i] if($debug);
	    }
	    # store the raw line with key "Line"
	    $hash{"Line"} = $line;
	    # take reference to copy of the hash, push on array referenced by $aoh
	    my $href = {%hash};
	    push @$aoh, $href;
	}
    }

    close FP;
    return ($header_str, $aor, $aoh, $c);
}
