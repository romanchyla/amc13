#!/usr/bin/perl
#
# read XML address table
# convert to CSV suitable for editing in a spreadsheet
#
use Text::ParseWords;

my %ids;

$debug = 0;

$level = 0;			# nesting level

# print CSV heading
print qq{"Address","LoBit","HiBit","Permission","Group","Name","Description"\n};
# printf qq{"%s",%d,%d,"%s","%s","%s","%s"\n}, $addr, $lo, $hi, $perm, $ename, $id, $desc;

while( $line = <>) {
    chomp $line;

    print "PARSING: $line\n" if($debug);

    # closing node
    if( $line =~ m{^\s*</node}) {
	die "Structure error (</node> at level 0)" if( $level == 0);
	$level -= 1;
	print "  DOWN level to $level\n" if($debug);
    }

    # opening node
    if( $line =~ /^\s*<node/) {

	my $hash = { };
	($s,$term) = $line =~ m{^\s*<node\s+(.*?)(/?)>\s*$};
	# split on whitespace or '=' respecting quoted strings
	print "---> Parsing $s [$term]\n" if($debug);
	@d = quotewords( '[\s=]+', 1, $s);
	$nd = $#d;
	# copy all attributes to a hash
	for( $i=0; $i <$#d; $i+=2) {
	    # strip quotes from quoted strings
	    $sval = $d[$i+1];
	    if( $sval =~ m{^".*"$}) {
		($val) = $sval =~ m{^"(.*)"$};
	    } else {
		$val = $sval;
	    }
	    # convert any remaining double quotes to single ones
	    $val =~ s{"}{'};
	    print "    $d[$i] = $val\n" if($debug);
	    $hash->{uc $d[$i]} = $val;
	}

	# must have an ID
	die "no ID in line $line\n" if( !$hash->{"ID"});
	$id = $hash->{"ID"};
	# and it must not be a duplicate
	die "Duplicate ID $id\n" if( $ids{$id});

	$label[$level] = $id;

	print "  UP level to $level\n" if($debug);

	# remember address
	if( $hash->{"ADDRESS"}) {
	    $addr = $hash->{"ADDRESS"};
	}
	
	if( $hash->{"DESCRIPTION"}) {
	    $desc = $hash->{"DESCRIPTION"};
	} else {
	    $desc = "??";
	}

	if( $hash->{"PERMISSION"}) {
	    $perm = $hash->{"PERMISSION"};
	} else {
	    $perm = "rw";
	}

	if( $hash->{"MASK"}) {
	    # convert mask to bit range
	    $imask = hex $hash->{"MASK"};
	    printf "convert %s = %d\n", $hash->{"MASK"}, $imask  if($debug);
	    print "ID=$id mask=$imask\n"  if($debug);
	    for( $lo=0; $lo<32; $lo++) {
		last if( $imask & (1<<$lo));
	    }
	    for( $hi=31; $hi>=0; $hi--) {
		last if( $imask & (1<<$hi));
	    }
	} else {
	    # no mask means all 1's
	    $lo = 0;
	    $hi = 31;
	}

	# if there is a tag closer, then it's an item
	if( $term eq "/") {
	    # make the extended name
	    $ename = "";
	    if( $level > 0) {
		for( $i=1; $i<$level; $i++) {
		    $ename .= "." if( $i > 1);
		    $ename .= $label[$i];
		}
	    }
	    printf qq{"%s",%d,%d,"%s","%s","%s","%s"\n}, $addr, $lo, $hi, $perm, $ename, $id, $desc;
	} else {
	    $level += 1;
	}
    }
}
