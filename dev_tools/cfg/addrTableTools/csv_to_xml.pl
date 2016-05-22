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
my @optAtt = ( "Mode", "Size", "P_Table", "P_Row", "P_Column", "P_Status", "P_Show", "P_Format" );

die "usage: csv_to_xml file1.csv file2.xml" if( $#ARGV<1);
my $f1 = $ARGV[0];
my $f2 = $ARGV[1];

#-- load up the CSV file
my ($a1,$c1) = load_csv_aoh( $f1);
my $na1 = @$a1;
my $nc1 = @$c1;
print "Loaded $na1 rows, $nc1 columns from $f1\n";
my $i;

# print XML heading info
open XML, "> $f2" or die "Opening $f2 for output";
print XML qq{<?xml version="1.0" encoding="ISO-8859-1"?>\n};
print XML qq{<!-- Generated from $f1 -->\n};
print XML qq{<!-- Input fields: };
for( $i=0; $i<$nc1; $i++) {
    printf XML " %s", $c1->[$i];
}
print XML "-->\n";

# sort by column "Name"
my @sorta = sort item_cmp @$a1;

# create tree with one top node
my $tree = Tree::Simple->new("0", Tree::Simple->ROOT);

# read the list, convert to a tree
for( $i=0; $i <= $#sorta; $i++) {
    my $a = $sorta[$i];
    add_path_to_tree( $tree, $a, $a->{"Name"});
}

# define functions for tree traversal

# define function to call for each node
my $tfunc = sub {
    my ($_tree) = @_;
    my $item = $_tree->getNodeValue();
    my $id = $item->[0];	# node name
    my $csv = $item->[1];	# reference to full CSV record
    my $addr = $csv->{"Address"};
    my $desc = $csv->{"Description"};
    if( $_tree->isLeaf()) {
	print XML ( '   ', ('   ' x $_tree->getDepth()));
	# check special case for description of form "module="
	if( $desc =~ /module=/) {
	    # handle special case of XML module ("include")
	    (my $modu) = $desc =~ /module=(.*)$/;
	    print "Module from $modu for $id\n";
	    printf XML qq[<node id="%s" module="%s" address="%s" />\n], $id, $modu, $addr;
	} else {
	    # print XML for normal node... first the mandatory attributes
	    printf XML qq[<node id="%s" address="%s" permission="%s" description="%s"],
	    $id, $csv->{"Address"}, 
	    $csv->{"Permission"}, $csv->{"Description"};
	    my $att;
	    # include optional attributes
	    my $param_str = "";
	    foreach $att ( @optAtt ) {
		if( length($csv->{$att})) {
		    my $lcatt = lc $att;
		    if( $att =~ /^P_/) {
			my ($par_name) = $att =~ /^P_(.*)$/;
			my $par_value = $csv->{$att};
			# check for funny characters in parameters
			if( $par_value =~ /[^A-Za-z0-9 _]/) {
			    print "ERROR: Non-alphanumeric character in parameter value: $par_value\n";
			    print "Please fix this\n";
			    exit;
			}
			$param_str .= ";" if( length($param_str));
			$param_str .= "$par_name=$par_value";
		    } else {
			printf XML qq[ %s="%s"], $lcatt, $csv->{$att};
		    }
		}
	    }
	    if( length( $param_str)) {
		printf XML qq[ parameters="%s"], $param_str;
	    }
	    # mask only if not bits 0..31
	    if( $csv->{"LoBit"} ne "0" or $csv->{"HiBit"} ne "31") {
		printf XML qq[ %s="0x%08x"], "mask", make_mask($csv->{"LoBit"},$csv->{"HiBit"});
	    }
	    printf XML qq[/>\n];
	}

    } else {
	print XML ( '   ', ('   ' x $_tree->getDepth()), '<node id="', $id,"\">\n");
    }
};

# define function to call after processing each node (closing tags)
my $pfunc = sub {
    my ($_tree) = @_;
    my $item = $_tree->getNodeValue();
    my $id = $item->[0];
    if( !$_tree->isLeaf()) {
	print XML ('   ', ('   ' x $_tree->getDepth()),
		   "</node>\n");
    }
};

# traverse and display the tree

print XML "<node id=\"TOP\">\n";
$tree->traverse( $tfunc, $pfunc);
print XML "</node>\n";

#
# add item to tree
#   $tr    is a Tree::Simple tree
#   $item  is any scalar item to add
#   $path  is dot-separated heirarchial name
#
# actual item stored is a reference to a two-element array with the ( $item, $path )
#
sub add_path_to_tree {

    my( $tr, $item, $path) = @_;

    my @s = split '\.', $path;

    my $i;
    for( $i=0; $i<=$#s; $i++) {
	my $id = $s[$i];

	if( $tr->isLeaf()) {	# it's a leaf, so add as child
	    my $new_child = Tree::Simple->new( [ $id, $item] );
	    $tr->addChild( $new_child);
	    $tr = $new_child;	# descend to new child
	} else {
	    # see if we have a child with this name
	    my $c;
	    my $matched = 0;
	    foreach $c ( $tr->getAllChildren) {
		if( $c->getNodeValue()->[0] eq $id) {
		    $tr = $c;	# descend to this child
		    $matched = 1;
		}
	    }
	    if( !$matched) {	# no matching child
		my $new_child = Tree::Simple->new( [$id, $item]);
		$tr->addChild( $new_child);
		$tr = $new_child;
	    }
	}
    }
}


#
# create a bitmask
#
sub make_mask {
    my ($lo,$hi) = @_;
    my $i0 = $lo + 0;
    my $i1 = $hi + 0;
    my $i;
    my $m = 0;
    for( $i=$lo; $i<=$hi; $i++) {
	$m = $m | (1<<$i);
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
