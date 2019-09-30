#!/usr/bin/env tclsh
# directory: demo/FreePDK45/NangateOpenCellLibrary

# windows usage: cacdtcl install.tcl

if [file exists .dmrc] {
	puts "NangateOpenCellLibrary already installed"
	puts "use \"rmpr -f .\" to remove the project data"
	exit 1
}

set fp [open install.log w]

# turn cwd '.' into a project
puts "+ mkpr -p FreePDK45 ."
puts $fp "+ mkpr -p FreePDK45 ."
exec mkpr -p FreePDK45 .

# install the layouts
puts "+ cgi NangateOpenCellLibrary.gds"
puts $fp "+ cgi NangateOpenCellLibrary.gds"
puts $fp "(see cgi.log)"
exec cgi NangateOpenCellLibrary.gds >& cgi.log
set fq [open cgi.log r]
puts [read $fq]
close $fq

# foreach cell -> extract the netlist
# and set the library status (not for the FILLCELL's)
# metal1 must be a free-mask, else the basic cell
# terminals can not connect to the above level
foreach cell [exec dblist -l] {
	puts "+ space -F $cell"
	puts $fp "+ space -F $cell"
	exec space -Scompression=off -F $cell >&@ $fp
	puts "+ sls_exp $cell"
	puts $fp "+ sls_exp $cell"
	exec sls_exp $cell >&@ $fp
	if {[string match FILLCELL* $cell]} {
		puts "skip xcontrol for $cell"
		puts $fp "skip xcontrol for $cell"
	} else {
		puts "+ xcontrol -library -freemasks:metal1 $cell"
		puts $fp "+ xcontrol -library -freemasks:metal1 $cell"
		exec xcontrol -library -freemasks:metal1 $cell >&@ $fp
	}
}

# remove extract data from the layouts
puts "+ dbclean -l -a"
puts $fp "+ dbclean -l -a"
exec dbclean -l -a >&@ $fp

puts "-- finished (see install.log)"
