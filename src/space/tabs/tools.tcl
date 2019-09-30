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

variable project_directory
catch {unset project_directory}

variable default_lambda
catch {unset default_lambda}

variable __is_initialized
set __is_initialized 0

proc initialize {} {

    std::invariant !$::tabs::__is_initialized

    set ::tabs::project_directory [std::tmppath "tabs"]
    if ![file exists $::tabs::project_directory] {
        ::std::verbose "Creating project directory $::tabs::project_directory ..."
        file mkdir $::tabs::project_directory
    }

    set ::tabs::default_lambda 0.0001

    set ::tabs::__is_initialized 1
}

#===============================================================================
#  Cell names.
#===============================================================================

variable last_cell_id
set last_cell_id 0

proc get_unique_cell_name {} {
    std::invariant $::tabs::__is_initialized
    return [format cell%04d [incr ::tabs::last_cell_id]]
}

#===============================================================================
#  Create a project.
#===============================================================================

proc create_project {process_path} {

    std::invariant $::tabs::__is_initialized

    set project_path $::tabs::project_directory

    if ![file exists $project_path/.dmrc] {
	::std::verbose "Making project directory $project_path ..."
	::std::quiet_execute mkpr -p $process_path -l $::tabs::default_lambda $project_path
    }
}

proc get_tecc_color {i} {

    set color_list {1 4 3 2 5 6 7}
    set color [lindex $color_list [expr $i%[llength $color_list]]]

    set red   [format %02x [expr (($color>>0)&1)*255]]
    set green [format %02x [expr (($color>>1)&1)*255]]
    set blue  [format %02x [expr (($color>>2)&1)*255]]

    return "@$red$green$blue"
}

#===============================================================================
#  Ideal plate capacitance.
#===============================================================================

proc get_ideal_plate_capacitance_between_layers {h1 h2 _technology_table} {

    upvar $_technology_table technology_table

    std::invariant $::tabs::__is_initialized

    # Use relative permittivity of silicon as start value.
    set previous_permittivity 11.7
    set previous_z $h1
    set list [list]

    for {set j 0} {$j < $technology_table(dielectrics:size)} {incr j} {

        set z $technology_table(dielectrics:$j:z)
        set permittivity $technology_table(dielectrics:$j:permittivity)

        if {$z >= $h1 && $z < $h2} {
            if {$z > $previous_z} {
                lappend list [list [expr $z - $previous_z] $previous_permittivity]
                set previous_z $z
            }
        } elseif {$z >= $h2} {
            break
        }

        set previous_permittivity $permittivity
    }

    lappend list [list [expr $h2 - $previous_z] $previous_permittivity]

    set tmp 0.0

    foreach tuple $list {
        set h  [expr [lindex $tuple 0]/1e10]
        set er [lindex $tuple 1]
        set cpp [expr $er*$::std::constants::epsilon_0/$h]
        set tmp [expr $tmp + 1/$cpp]
    }

    return [expr 1/$tmp]
}

proc edge_condition {condition {type "-"}} {

    set condition [exec minimize $condition]

    if {$condition == "0"} {return "0"}
    if {$condition == "1"} {return "1"}

    set list [list]
    foreach w [split $condition " "] {
        if {$w == "|"} {
            lappend list $w
            continue
        }

        set prefix ""
        if {[string index $w 0] == "!"} {
            set prefix "!"
            set w [string range $w 1 end]
        }

        if {[string index $w 0] == "-" || [string index $w 0] == "="} {
            # This case should be an error, actually.
            lappend list "${prefix}${w}"
            continue
        }

        lappend list "${prefix}${type}${w}"
    }

    return [join $list " "]
}

#===============================================================================
#  Progress computation.
#===============================================================================

set ::progress_enabled 0
set ::progress_priming 0
set ::progress 0
set ::progress_total 0

proc progress_enable {} {
    set ::progress_enabled 1
}

proc progress_prime {script} {
    if !$::progress_enabled return
    set ::progress_priming 1

    uplevel #1 $script

    set ::progress_priming 0
    set ::progress_total $::progress
    set ::progress 0
}

proc progress_add {} {
    if !$::progress_enabled { return 0 }
    incr ::progress

    if !$::progress_priming {
        set msg "$::progress/$::progress_total"
        puts stderr "progress: [std::colorize $msg blue]"
    }
    return $::progress_priming
}

proc progress_finish {} {
    if !$::progress_enabled return

    std::invariant !$::progress_priming

    if {$::progress < $::progress_total} {
        set msg "$::progress_total/$::progress_total"
        puts stderr "progress: [std::colorize $msg blue]"
    }
}

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

