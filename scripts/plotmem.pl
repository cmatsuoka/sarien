#!/usr/bin/perl
#
# Run this script on the dmalloc log file generated with log-trans
# (DMALLOC_OPTIONS=debug=0x14f47d8b,log=log)

$\ = "\n";
$mem = 0;

open PLOT, "|gnuplot";

print PLOT <<EOF;
set terminal png small color
set grid
set size 1.0,0.5
set data style lines
set title "Dmalloc memory usage report"
set xlabel "Transactions"
set ylabel "KBytes"
set output "dmalloc.png"
plot '-' using 1:2 title "Bytes allocated"
EOF

while (<>) {
	$when = 0;

	if (/(\d+): \*\*\* c?alloc:.*for (\d+) bytes/) {
		$when = $1;
		$mem += $2;
	} elsif (/(\d+): \*\*\* free:.*: size (\d+)/) {
		$when = $1;
		$mem -= $2;
	}

	$when || next;

	printf PLOT "$when %f\n", $mem / 1024;
}

print PLOT "e";

close PLOT;
