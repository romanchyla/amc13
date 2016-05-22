#!/usr/bin/perl
#
# read two CSV files with address tables
# check for matches in address/mask
# make a report
#
use Text::ParseWords;
use strict;

my $debug = 0;

die "usage: check_dupes file1.csv file2.csv" if( $#ARGV<1);
my $f1 = $ARGV[0];
my $f2 = $ARGV[1];

#-- load up the files
my ($a1,$c1) = load_csv_aoh( $f1);
my $na1 = @$a1;
my $nc1 = @$c1;
print "Loaded $na1 rows, $nc1 columns from $f1\n";
my $i;
for( $i=0; $i<$nc1; $i++) {
    printf " %s", $c1->[$i];
}
print "\n";

my ($a2,$c2) = load_csv_aoh( $f2);
my $na2 = @$a2;
my $nc2 = @$c2;
print "Loaded $na2 rows, $nc2 columns from $f2\n";
for( $i=0; $i<$nc2; $i++) {
    printf " %s", $c2->[$i];
}
print "\n";

#-- check for dupes with first as master
for( $i=0; $i<$na1; $i++) {
     # check for matches
     my $k;
     my $found = 0;
     for( $k=0; $k<$na2; $k++) {
	 if( item_compare( $a1->[$i], $a2->[$k])) {
#	     print "Match ($i,$k) = " . item_format( $a1->[$i]) . "\n";
	     $found = 1;
	     $a1->[$i]->{"Found"} = 1;
	     $a2->[$k]->{"Found"} = 1;
	     last;
	 }
     }
     if( ! $found) {
#	 print "NO MATCH for row1=$i " . item_format( $a1->[$i]) . "\n";
	 $a1->[$i]->{"Found"} = 0;
     }
}

#-- print a summary
print "--- Not found in $f1:\n";
for( $i=0; $i<$na1; $i++) {
    my $h = $a1->[$i];
    if( ! exists $h->{"Found"}) {
	print "Missing 'Found' in row $i in file $f1:\n";
	print "  in data: " . $a1->[$i]->{"Line"} . "\n";
	die "   no 'Found' in row $i" ;
    }
    if( $h->{"Found"} == 0) {
	print $i . " : " . $h->{"Line"} . "\n";
    }
}
	
print "--- Not found in $f2:\n";
for( $i=0; $i<$na2; $i++) {
    my $h = $a2->[$i];
    if( !exists $h->{"Found"} or $h->{"Found"} == 0) {
	print $i . " : " . $h->{"Line"} . "\n";
    }
}


#------------------------------------------------------------
# item_format( $hr)
#    return item formatted for debug display
#------------------------------------------------------------

sub item_format {
    my $h = shift @_;
    my $s = sprintf( "(%s, %d, %d, %s)",
		     $h->{"Address"}, $h->{"LoBit"}, $h->{"HiBit"},
		     $h->{"Permission"});
    return $s;
}
    


#------------------------------------------------------------
# item_compare( $hr1, $hr2)
#    compare two address table items by address, lo, hi, perm
# return 1 if match, 0 if not
#------------------------------------------------------------

sub item_compare {
    my $h1 = shift @_;
    my $h2 = shift @_;
    if( $h1->{"Address"} eq $h2->{"Address"} and
	$h1->{"LoBit"} == $h2->{"LoBit"} and
	$h1->{"HiBit"} == $h2->{"HiBit"} and
	$h1->{"Permission"} eq $h2->{"Permission"}) {
	return 1;
    } else {
	return 0;
    }
}


	


#------------------------------------------------------------
# ($aoh,$aos) = load_csv_aoh( $file)
#    read a CSV file into an array of hashes
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
    my $a = [ ];		# array for hashrefs
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
	    # take reference to copy of the hash, push on array referenced by $a
	    my $href = {%hash};
	    push @$a, $href;
	}
    }

    close FP;
    return ($a, $c);
}
