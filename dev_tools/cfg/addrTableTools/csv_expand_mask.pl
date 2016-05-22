#!/usr/bin/perl
#
# read CSV file and expand bit masks in semi-intelligent way
#

use Text::ParseWords;
use strict;

my $debug = 0;

my $f1 = $ARGV[0];

#-- load up the CSV file
my ($a1,$c1) = load_csv_aoh( $f1);
my $na1 = @$a1;
my $nc1 = @$c1;
print "Loaded $na1 rows, $nc1 columns from $f1\n";
my $i;

foreach my $rr ( @$a1 ) {
    if( $rr->{Name} =~ /^STATUS.*MASK$/ && ($rr->{LoBit} != $rr->{HiBit})) {
	my $msize = $rr->{HiBit}-$rr->{LoBit}+1;
	my @parts = split '\.', $rr->{Name};
	print "# Mask to expand:  $rr->{Name}\n";
	my $ifmt;
	my $nit;
	if( $msize == 12) {
	    $ifmt = "AMC%02d";
	    $nit = 1;
	} elsif( $msize == 3) {
	    $ifmt = "SFP%d";
	    $nit = 0;
	}

	for( my $b = $rr->{LoBit}; $b <= $rr->{HiBit}; $b++) {
	    my $item = sprintf $ifmt, $nit;
	    if( $#parts > 1) {
		$parts[1] = $item;
	    } else {
		splice @parts, 1, 0, ($item);
	    }
	    # output the new line
	    my $newname = join '.', @parts;
	    my $outstr = "";
	    foreach my $col ( @$c1 ) {
		if( $col eq "Name") {
		    $outstr .= "\"$newname\",";
		} elsif( $col eq "LoBit" || $col eq "HiBit") {
		    $outstr .= $b . ",";
		} elsif( $col eq "P_Column")  {
		    $outstr .= qq["$item",];
		} elsif( $col eq "P_Row") {
		    my $s = $parts[$#parts];
		    $s =~ s{_MASK}{};
		    $outstr .= qq["$s",];
		} else {
		    $outstr .= qq["$rr->{$col}"] . ",";
		}
	    }
	    chop $outstr;
	    print "$outstr\n";
	    $nit++;
	}
    }
}

#------------------------------------------------------------
# ($aoh,$aos) = load_csv_aoh( $file)
#    read a CSV file into an array of hash references
#
# skip lines beginning with '#' as comments
# parse first line as column headings
# parse subsequent lines and create a hash from each line
# using column headings as keys
#
# return a list of two references:
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
	    $c = [@d];
	    $nc = $nd;
	    $hdr = 0;
	} else {		# non-heading
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
    return ($aoh, $c);
}
