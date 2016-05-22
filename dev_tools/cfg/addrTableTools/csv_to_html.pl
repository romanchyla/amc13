#!/usr/bin/perl
#
# read CSV file and convert XML address table
# CSV file must have fields:
#    Name, Address, LoBit, HiBit, Permission, Description, Tags
# Name is dotted as ACTIONS.RESET.GENERAL
#
# 2014-08-07, hazen - add P_ prefixed parameters Row, Column, Status, Show

use Text::ParseWords;
use Tree::Simple;
use strict;

my $debug = 0;

# optional attributes.  Ones with P_ prefix are concatenated into "parameters"
# colums to display
my %cols = map { $_ => 1 } ( "Address", "LoBit", "HiBit", "Name", "Description" );

die "usage: csv_to_xml file1.csv file2.xml" if( $#ARGV<1);
my $f1 = $ARGV[0];
my $f2 = $ARGV[1];

#-- load up the CSV file
my ($a1,$c1) = load_csv_aoh( $f1);
my $na1 = @$a1;
my $nc1 = @$c1;
print "Loaded $na1 rows, $nc1 columns from $f1\n";
my $i;

my %bit_color = ( "STATUS" => "#008000", "ACTION" => "#800000", "CONF" => "#000080" );
my %bit_bg =    ( "STATUS" => "#808080", "ACTION" => "#808080", "CONF" => "#808080" );

# print HTML heading info
open HTML, "> $f2" or die "Opening $f2 for output";
print HTML "<html><head>\n";
print HTML "<title>AMC13 Register Map</title></head>\n";
print HTML "<body>\n";
print HTML "<table>\n<tr>";

print HTML "<tr><th>Address";
for( $i=31; $i>=0; $i--) {
    printf HTML "<th><font size=-3>%02d</font>", $i;
}
print HTML "<th>Name<th>Description\n";

# sort by column "Name"
my @sorta = sort item_cmp @$a1;

# output all the stuff
my $r;
my $lastN = "xx";
my $type;

foreach $r ( @sorta ) {
    my $name = $r->{"Name"};
    my @wordz = split /\./, $name;
    $type = $wordz[0];
#    print "Name= $name  type=[$type]\n";

# uncomment below to filter out all repeated items
#    if( $name !~ /([A-Z]+)\d./ ) {
    if( 1) {
	if( $wordz[1] ne $lastN) {
	    print HTML "<tr><td>&nbsp;";
	    for( $i=31; $i>=0; $i--) {
		printf HTML "<th><font size=-3>%02d</font>", $i;
	    }
	    print HTML "<th>&nbsp;<th>&nbsp;\n";
	    printf HTML "<tr><td>%s</td>\n", $r->{"Address"};
	} else {
	    printf HTML "<tr><td>%s</td>\n", $r->{"Address"};
	}
	$lastN = $wordz[1];
	print HTML make_bits( $r->{"LoBit"}, $r->{"HiBit"});
	print HTML "<td>", $r->{"Name"}, "<td>", $r->{"Description"};

	print HTML "\n";
    }
}

print HTML "</table></body></html>\n";

#
# show bits
#
sub make_bits {
    my ($lo,$hi) = @_;
    my $mask = make_mask( $lo,$hi);
#    printf "Mask from $lo, $hi = 0x%08x\n", $mask;
    my $i;
    my $str = "";
    for( $i=31; $i>=0; $i--) {
	if( $mask & (1<<$i)) {
	    $str .= qq{<td bgcolor="} . $bit_color{$type} . qq{">};
	} else {
	    $str .= qq{<td bgcolor="} . $bit_bg{$type} . qq{">};
	}
   }
    return $str;
}

#
# create a bitmask
#
sub make_mask {
    my ($lo,$hi) = @_;
#    print "make_mask( $lo, $hi)\n";
    my $i;
    my $m = 0;
    for( $i=($lo+0); $i<=($hi+0); $i++) {
	$m = $m | (1<<$i);
#	printf "  make_mask loop %d mask 0x%x\n", $i, $m;
    }
    return $m;
}


#
# compare lexically using name field
#
sub item_cmp {
    if( $a->{"Name"} ne $b->{"Name"}) {
	return $a->{"Name"} cmp $b->{"Name"};
    }
    return 0;
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
