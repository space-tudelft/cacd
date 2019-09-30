#
# ISC License
# Copyright (C) 2004-2018 by
#	Kees-Jan van der Kolk
#	Simon de Graaf
#	Nick van der Meijs
# Delft University of Technology
#

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace eval tabs {
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

set tecc_patterns [list]
lappend tecc_patterns [list conduc {^[ \t]*conductors[ \t]*[^ \t\n]+[ \t]*[:]}]
lappend tecc_patterns [list tor {(^[ \t]*transistors[ \t]*[:])|(^[ \t]*fets[ \t]*[:])}]
lappend tecc_patterns [list bjt {(^[ \t]*bipotors[ \t]*[:])|(^[ \t]*bjts[ \t]*[:])}]
lappend tecc_patterns [list jun {^[ \t]*junctions[ \t]*[:]}]
lappend tecc_patterns [list cnt {^[ \t]*connects[ \t]*[:]}]
lappend tecc_patterns [list contact {^[ \t]*contacts.*[:]}]
lappend tecc_patterns [list cap {^[ \t]*(junction[ \t]+)?capacitances([ \t]+[a-zA-Z_][a-zA-Z0-9_]*)?[ \t]*[:]}]
lappend tecc_patterns [list vdim {^[ \t]*vdimensions[ \t]*[:]}]
lappend tecc_patterns [list eshape {^[ \t]*eshapes[ \t]*[:]}]
lappend tecc_patterns [list cshape {^[ \t]*cshapes[ \t]*[:]}]
lappend tecc_patterns [list dielec {^[ \t]*dielectrics[ \t]*[:]}]
lappend tecc_patterns [list sublay {^[ \t]*sublayers[ \t]*[:]}]
lappend tecc_patterns [list selfsub {^[ \t]*selfsubres[ \t]*[:]}]
lappend tecc_patterns [list mutsub {^[ \t]*coupsubres[ \t]*[:]}]

proc contains_capacitance_rules {tecc_lines} {
    foreach line $tecc_lines {
        if [regexp {^[ \t]*capacitances([ \t]+[a-zA-Z_][a-zA-Z0-9_]*)?[ \t]*[:]} $line] {
            return 1
        }
    }
    return 0
}

proc add_rules_to_file {path type id rules} {

    set fp [open $path r]
    set text [read $fp]
    close $fp

    regsub {[\n]$} $text "" text
    set tecc_lines [split $text "\n"]
    set tecc_lines [add_rules $type $id $tecc_lines $rules]

    set fp [open $path w]
    puts $fp [join $tecc_lines "\n"]
    close $fp
}

proc remove_rules {id tecc_lines _rem_lines} {

    upvar $_rem_lines rem_lines

    eval {
        set lines [list]
        set in_block 0
        set eat_white_lines 0

        foreach line $tecc_lines {

            set reduced_line $line
            regsub {^[ \t]+} $reduced_line "" reduced_line
            regsub -all {[ \t]+} $reduced_line " " reduced_line

            if $eat_white_lines {
                if ![string length $reduced_line] continue
                set eat_white_lines 0
            }

            if $in_block {
		lappend rem_lines $line
                if [regexp {^[#][ ]GENERATED END} $line] {
                    set in_block 0
                    set eat_white_lines 1
                    continue
                }
            }

            if [regexp {^[#][ ]GENERATED BEGIN[:]} $line] {
                set list [split $line " \t"]
                if {[lindex $list 3] == $id} {
                    set in_block 1
		    lappend rem_lines $line
                    continue
                }
            }

            if !$in_block {
                lappend lines $line
            }
        }

        set tecc_lines $lines
    }

    return $tecc_lines
}

proc add_rules {type id tecc_lines rules} {

    # First remove old rules with this id (if present).
    #
    set removed_lines [list]
    set tecc_lines [remove_rules $id $tecc_lines removed_lines]

    set units(resistance) 1
    set units(c_resistance) 1
    set units(s_resistance) 1
    set units(capacitance) 1
    set units(a_capacitance) 1
    set units(e_capacitance) 1
    set units(distance) 1
    set units(vdimension) 1
    set units(shape) 1
    set units(layerdepth) 1
    set units(resizemask) 1

    # Insert the new rules.
    #
    eval {
        set lines [list]
        set placed 0
        foreach line $tecc_lines {

            set reduced_line $line
            regsub {[#].*} $reduced_line "" reduced_line
            regsub {^[ \t]+} $reduced_line "" reduced_line
            regsub -all {[ \t]+} $reduced_line " " reduced_line

            set list [split $reduced_line " \t"]

            # Check if this is a `unit' line.
            #
            if {[lindex $list 0] == "unit"} {
                set unit_type [lindex $list 1]
                set unit_value [lindex $list 2]
                set units($unit_type) $unit_value
                lappend lines $line
                continue
            }

            # Determine if we should place the rules at this point.
            #
            if !$placed {
                set found 0
                set place 0
                foreach pattern $::tabs::tecc_patterns {
                    if {[lindex $pattern 0] == $type} {
                        set found 1
                        continue
                    }
                    if [regexp [lindex $pattern 1] $reduced_line] {
                        if $found {
                            set place 1
                        }
                    }
                }

                # Place the rules.
                #
                if $place {
                    add_separating_line lines
		    if {$rules != ""} {
			eval lappend lines [patch_rules $rules $id units]
		    } elseif {$removed_lines != ""} {
			eval lappend lines $removed_lines
		    }
                    lappend lines ""
                    set placed 1
                }
            }

            lappend lines $line
        }
    }

    if !$placed {
        add_separating_line lines
	if {$rules != ""} {
	    eval lappend lines [patch_rules $rules $id units]
	} elseif {$removed_lines != ""} {
	    eval lappend lines $removed_lines
	}
        lappend lines ""
        set placed 1
    }

    return $lines
}

proc add_separating_line {_lines} {
    upvar $_lines lines
    set last_line [lindex $lines end]
    regsub {^[ \t]+} $last_line "" last_line
    if [string length $last_line] {
        lappend lines ""
    }
}

proc patch_rules {rules id _units} {

    upvar $_units units

    set list [list]
    foreach rule $rules {

        set string ""
        set pos 0
        while 1 {
            if [regexp -start $pos -indices -- {%%([^%]|[%][^%])*%%} $rule indices] {
                append string [string range $rule $pos [expr [lindex $indices 0]-1]]
                set match [string range $rule [lindex $indices 0] [lindex $indices 1]]
                set pos [expr [lindex $indices 1]+1]

                #puts stderr "MATCH $match"
                regsub {^%%} $match "" match
                regsub {%%$} $match "" match

                if {[lindex $match 0] == "UNIT"} {
                    set unit_type [lindex $match 1]
                    set value [lindex $match 2]
                    append string [format "%.5g" [expr $value/$units($unit_type)]]
                } else {
                    std::invariant 0
                }

            } else {
                append string [string range $rule $pos end]
                break
            }
        }

        lappend list $string
    }

    # Remove empty lines at the beginning and end of the rule list.
    #
    eval {
        set rules $list
        set list [list]
        set buffer [list]
        foreach rule $rules {
            set reduced_rule $rule
            regsub -all {[ \t]} $reduced_rule "" reduced_rule
            if ![string length $reduced_rule] {
                lappend buffer $rule
            } else {
                if {[llength $list]} {
                    eval lappend list $buffer
                }
                set buffer [list]
                lappend list $rule
            }
        }
        set rules $list
    }

    set header [list]
    lappend header "# GENERATED BEGIN: $id (edit this block at your own risk)"
    lappend header "# Added: [clock format [clock seconds]]"

    set footer [list]
    lappend footer "# GENERATED END"

    return [concat $header $rules $footer]
}

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

