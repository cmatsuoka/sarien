#!/usr/bin/perl
#
# $Id$

$graphic  = "color=red";
$input    = "color=magenta";
$sarien   = "color=blue";
$agisys   = "color=lightgrey";
$interp   = "color=green";
$menu     = "color=orange";
$sound    = "color=lightcyan";
$view     = "color=peachpuff";
$logic    = "color=yellow";

$lowlevel = "shape=box style=bold";
$midlevel = "shape=trapezium";
$optional = "shape=hexagon";

$resource = "style=filled";

$def_edge  = "color=black style=solid";

print "digraph sarien {\n";
print "edge [$def_edge];\n";

while (<>) {
	s/^\d+//;

	# Assign node properties to interesting functions
	#
	$x = $_;
	if (/\t*(\w+) {(\w+)\.c/) {
		(my $x, $_) = ($1, $2);
		($x =~ /ccmd_/) && next;
		/^graphic/ && print "\t$x [$graphic $lowlevel];\n";
		/^sprite/  && print "\t$x [$graphic];\n";
		/^console/ && print "\t$x [$sarien $optional];\n";
		/^keyboar/ && print "\t$x [$input];\n";
		/^agi/     && print "\t$x [$agisys $resource];\n";
		/^motion/  && print "\t$x [$interp $midlevel];\n";
		/^checks/  && print "\t$x [$interp $lowlevel];\n";
		/^cycle/   && print "\t$x [$interp];\n";
		/^menu/    && print "\t$x [$menu];\n";
		/^sound/   && print "\t$x [$sound $resource];\n";
		/^view/    && print "\t$x [$view $resource];\n";
		/^logic/   && print "\t$x [$logic $resource];\n";

		/^console/ && ($edge{$x} = "color=blue");
		/^graphic/ && ($edge{$x} = "color=red");
	}
	$_ = $x;

	($f) = /(\w+)/;
	$i = rindex $_, "\t";
	@m[$i] = $f;
	($i < 1) && next;

	$_ = "\t\"@m[$i - 1]\" -> \"@m[$i]\";\n";

	# Filter out functions we don't want charted
	#
	(/[^\w](_D|report|[sg]etflag|[sg]etvar|main|fixpath)[^\w]/
		|| /[^\w](memset|feof|strtok|atoi|atol|abs|tolower)[^\w]/
		|| /[^\w](strtoul|fseek|ftell|abort)[^\w]/
		|| /(_slot)[^\w]/
		|| /[^\w](list_|ccmd_)/
		|| /[^\w](lohi_|hilo_|__|test_|object_|read_)/) && next;

	$edge{@m[$i - 1]} &&
		($_ = "\tedge [$edge{@m[$i - 1]}];\n$_\tedge [$def_edge];\n");

	print;
}

print "}\n";


