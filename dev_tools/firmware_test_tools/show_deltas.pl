#!/usr/bin/perl
#
# readout diff --width=150 -y ./test1.sh_pre_status.txt ./test1.sh_post_status.txt  | fgrep \|
#

$na = $#ARGV+1;
if( $na < 2) {
    print "usage:  ./check.pl <file1> <file2>\n";
    exit;
}

$p = "diff --width=150 -y $ARGV[0] $ARGV[1] | fgrep \\| |";
print "Starting pipe $p\n";

open FP, "diff --width=150 -y $ARGV[0] $ARGV[1] | fgrep \\| |"
    or die "opening diff pipe";

while( $line = <FP>) {
    chomp $line;
    @d = split /\s+/, $line;
    die "mismatched names $d[1] vs $d[4]" if( $d[1] ne $d[4]);
    $dv = hex( $d[5]) - hex($d[2]);
    if( ! exists $names{$dv}) {
	$names{$dv} = [ ];
    }
    push @{$names{$dv}}, $d[1];
}

foreach $dv ( sort {$a<=>$b} keys %names) {
    printf "%d (0x%x)\n", $dv, $dv;
    foreach $n ( @{$names{$dv}}) {
	print "   $n\n";
    }
}
