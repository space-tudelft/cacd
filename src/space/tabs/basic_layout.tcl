#
# ISC License
# Copyright (C) 2004-2018 by
#	Kees-Jan van der Kolk
#	Simon de Graaf
#	Nick van der Meijs
# Delft University of Technology
#

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace eval ::tabs::basic_layout {
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#===============================================================================
#  Compute capacitances.
#
#  This procedure computes coupling capacitances for a given layout, using
#  space3d. The layout should be specified in the table `layout_table'.
#  See generate_layout for a description of the layout table format.
#
#  The results are returned in the array `results'. See read_results for
#  a description of the format.
#===============================================================================

proc compute_capacitances {_layout_table _diel_table _precision_table _caps} {

    upvar $_layout_table layout_table
    upvar $_diel_table diel_table
    upvar $_precision_table precision_table
    upvar $_caps caps

    std::auto_chdir $::tabs::project_directory {

	set cell_name [::tabs::get_unique_cell_name]

	generate_layout $cell_name layout_table

	# Estimate complexity with space3d.
	#
        set cmd [list]
        lappend cmd space3d -3CU
        lappend cmd -Scap3d.be_mode=$precision_table(be_mode)
        lappend cmd -Scap3d.be_window=$precision_table(be_window)

	if {$precision_table(be_area) >= 1000} {
	    set be_area [expr round($precision_table(be_area))]
	} else {
	    set be_area [format %.3g $precision_table(be_area)]
	}
	set cmd2 $cmd
	lappend cmd2 -Scap3d.max_be_area=$be_area $cell_name
	eval exec $cmd2 >& est_dat
	set fp [open est_dat r]
	set n 23
	while {[gets $fp line] >= 0} {
	    set n [string first "elements" $line]
	    if {$n > 0} {
		set n [expr [string first ":" $line $n] + 2]
		break
	    }
	}
	close $fp
	set complexity [string range $line $n end]

	# Run space3d.
	#
        set cmd [list]
        lappend cmd space3d -3C

	if [info exists ::main_options(--verboser)] {lappend cmd -v}

        lappend cmd -Scap3d.be_mode=$precision_table(be_mode)
        lappend cmd -Scap3d.max_be_area=$be_area
        lappend cmd -Scap3d.be_window=$precision_table(be_window)
        lappend cmd $cell_name

	set fp [open "$cell_name.cmd" w]
	puts $fp "# complexity = $complexity"
	puts $fp $cmd
	close $fp

	if ![info exists ::main_options(--fake)] {

	    std::enable_verbosity
	    eval std::quiet_execute $cmd
	    std::lower_verbosity

	    read_results $cell_name layout_table caps

	} else {
	    puts stderr "+ $cmd"
	}
    }
}


#===============================================================================
#
#  Generate layout (all units in 0.1 nm)
#
#  This procedure generates a new layout in the given project with the given
#  cell name.
#
#  The format of the layout table can best be described by an example:
#
#    set layout_table(0:n) 10            # number of wires (>= 1)
#    set layout_table(0:width)    10000  # width of the wire (1 um)
#    set layout_table(0:spacing)  10000  # spacing (1 um) between wires (> 0)
#    set layout_table(0:length) 1000000  # length of the wire (100 um)
#    set layout_table(0:z)         2000  # height (.2 um) of the wire (>= 0)
#    set layout_table(0:thickness)  700  # thickness (70 nm) of the wire (>= 0)
#
#    set layout_table(1:n) 10
#    set layout_table(1:width)    10000  # 1 um
#    set layout_table(1:spacing)  10000  # 1 um
#    set layout_table(1:length) 1000000  # 100 um
#    set layout_table(1:z)        12000  # 1.2 um
#    set layout_table(1:thickness)  700  # 70 nm
#
#    set layout_table(size) 2
#
#  This layout table describes two planes of wires; both planes
#  contain 10 wires.
#
#===============================================================================

proc generate_layout {cell_name _layout_table} {

    upvar $_layout_table layout_table

    set fp [open "$cell_name.ldm" w]

    puts $fp "ms $cell_name"

    for {set j 0} {$j < $layout_table(size)} {incr j} {

	if {$layout_table($j:z) == 0 && $layout_table($j:thickness) == 0} continue

	# width, length and spacing are in lambda=0.1nm units
	set w  [expr {int($layout_table($j:width))}]
	set s  [expr {int($layout_table($j:spacing))}]
	set y1 0
	if [info exists layout_table($j:y1)] { set y1 $layout_table($j:y1) }
	set y2 [expr {$y1 + int($layout_table($j:length))}]

	set n $layout_table($j:n)
	set center [expr {($n-1)/2}]

	set x1 0
	if [info exists layout_table($j:x1)] { set x1 $layout_table($j:x1) }

	for {set i 0} {$i < $n} {incr i} {
	    set x2 [expr {$x1 + $w}]
	    set name "t[encode_name $j [expr {$i-$center}]]"
	    puts $fp "term $layout_table($j:mask) $x1 $x2 $y1 $y2 $name"
	    set x1 [expr {$x2 + $s}]
	}
    }

    puts $fp "me"

    close $fp

    std::quiet_execute cldm "$cell_name.ldm"
}

#===============================================================================
#  Read results.
#
#  This procedure reads capacitance values from a circuit extracted using
#  space3d. The layout should have been created by generate_layout (because a
#  special node naming scheme is used).
#
#  The results are stored in the table `caps'.
#===============================================================================

proc read_results {cell_name _layout_table _caps} {

    upvar $_layout_table layout_table
    upvar $_caps caps

    set text [exec xsls $cell_name]
    regsub -all {[(){},;]} $text "" text
    regsub {[\n]$} $text "" text
##  puts stderr $text

    array unset caps
    foreach line [split $text "\n"] {
	if {[llength $line] == 4 && [lindex $line 0] == "cap"} {
	    set v [lindex $line 1]
	    set l [string length $v]
	    if {[regexp {p$} $v]} {
		set v [string range $v 0 [expr {$l - 2}]]e-12
	    } elseif {[regexp {f$} $v]} {
		set v [string range $v 0 [expr {$l - 2}]]e-15
	    } elseif {[regexp {a$} $v]} {
		set v [string range $v 0 [expr {$l - 2}]]e-18
	    }
	    set vd [std::validate_double $v 1]
##  puts stderr "v=$v vd=$vd"
	    set n1 [lindex $line 2]
	    set n2 [lindex $line 3]
	    if {$n1 < $n2} {set caps($n1,$n2) $vd} else {set caps($n2,$n1) $vd}
	}
    }
##  foreach n [lsort [array names caps]] {
##	puts stderr "cap($n) = $caps($n)"
##  }
}

#===============================================================================
#  Encode a name.
#  For example: 23 and -12 are encoded into p23n12.
#===============================================================================

proc encode_name {index0 index1} {
    return "[encode_number $index0][encode_number $index1]"
}

proc encode_number {index} {

    if {$index < 0} {
        return "n[expr -int($index)]"
    }
    if {$index > 0} {
        return "p[expr int($index)]"
    }
    return "c"
}

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

