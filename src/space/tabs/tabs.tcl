#
# ISC License
# Copyright (C) 2004-2018 by
#	Kees-Jan van der Kolk
#	Simon de Graaf
#	Nick van der Meijs
# Delft University of Technology
#

set tabsdir [file dirname $argv0]

source $tabsdir/tclstd/std.tcl
source $tabsdir/tclstd/colors.tcl
source $tabsdir/tclstd/constants.tcl
source $tabsdir/tclstd/environment.tcl
source $tabsdir/tclstd/algorithm.tcl
source $tabsdir/tclstd/array.tcl
source $tabsdir/tclstd/validate.tcl
source $tabsdir/tclstd/exception.tcl
source $tabsdir/tclstd/format.tcl
source $tabsdir/tclstd/options.tcl
source $tabsdir/tclstd/message.tcl
source $tabsdir/tclstd/exec.tcl
source $tabsdir/tclstd/path.tcl
source $tabsdir/tclstd/file.tcl
source $tabsdir/tclstd/math.tcl

source $tabsdir/project_tools.tcl
source $tabsdir/tools.tcl
source $tabsdir/basic_layout.tcl
source $tabsdir/write.tcl

#===============================================================================
#  Main procedure.
#===============================================================================

proc main {args} {

    set format_list [list]
    lappend format_list [list "--help"          0]
    lappend format_list [list "--verbose"       0]
    lappend format_list [list "--verboser"      0]
    lappend format_list [list "--no-color"      0]
    lappend format_list [list "--omit-vias"     0]
    lappend format_list [list "--add-lcaps2"    0]
    lappend format_list [list "--skip-ecaps"    0]
    lappend format_list [list "--skip-lcaps"    0]
    lappend format_list [list "--skip-vcaps"    0]
    lappend format_list [list "--skip-vdim"     1]
    lappend format_list [list "--fix-vdim"      0]
    lappend format_list [list "--quiet"         0]
    lappend format_list [list "--ignore-preexisting" 0]
    lappend format_list [list "--cf-lcaps"      1]
    lappend format_list [list "--cf-lcaps2"     1]
    lappend format_list [list "--edge-ratio"    1]
    lappend format_list [list "--precision"     1]
    lappend format_list [list "--sfile"         1]
    lappend format_list [list "--maskdata"      1]
    lappend format_list [list "--z-window"      1]
    lappend format_list [list "-E"              1]
    lappend format_list [list "-L"              1]
    lappend format_list [list "-M"              1]
    lappend format_list [list "--%fake"         0]
    lappend format_list [list "--fake"          0]

    lappend format_list [list "-h" "alias --help"]
    lappend format_list [list "-v" "alias --verbose"]
    lappend format_list [list "-V" "alias --verboser"]
    lappend format_list [list "-p" "alias --precision"]
    lappend format_list [list "-m" "alias --maskdata"]
    lappend format_list [list "-s" "alias --sfile"]
    lappend format_list [list "-i" "alias --ignore-preexisting"]
    lappend format_list [list "-q" "alias --quiet"]
    lappend format_list [list "-R" "alias --edge-ratio"]
    lappend format_list [list "-Z" "alias --z-window"]

    set usage ""
    append usage "Usage: tabs \[options\]\n"
    append usage "Options:\n"
    append usage "  -h, --help                  Print this message.\n"
    append usage "  -v, --verbose               Turn on verbosity.\n"
    append usage "  -V, --verboser              Turn on space3d verbosity.\n"
    append usage "  -q, --quiet                 Don't produce informational messages.\n"
    append usage "  -s, --sfile FILE.s          Use the given .s file. By default,\n"
    append usage "                              the space.def.s file of the current project.\n"
    append usage "  -m, --maskdata FILE         Use the given maskdata file. By default,\n"
    append usage "                              the maskdata file of the current project.\n"
    append usage "  -i, --ignore-preexisting    Ignore any preexisting capacitance rules\n"
    append usage "                              in the technology file.\n"
    append usage "  -p, --precision low|medium|high\n"
    append usage "                              Use the given level of precision (default low).\n"
    append usage "  -E VALUE                    To use another error-factor (default values for\n"
    append usage "                              the precisions are: 0.1|0.05|0.05).\n"
    append usage "  -L VALUE                    Set the number of layers over which capacitive\n"
    append usage "                              coupling is considered significant (default 2).\n"
    append usage "  -R, --edge-ratio VALUE      Set l/w ratio for edge caps (default 10).\n"
    append usage "  -Z, --z-window VALUE        Set the window in the z-direction for which\n"
    append usage "                              capacitive coupling is significant (in micron).\n"
    append usage "  --fake                      Fake run.\n"
    append usage "  --fix-vdim                  Fix vdim conditions with conductor conditions.\n"
    append usage "  --no-color                  Disable colored output.\n"
    append usage "  --omit-vias                 Don't add via conditions to vertical caps.\n"
    append usage "  --skip-ecaps                Skip edge-surface capacitance part.\n"
    append usage "  --skip-lcaps                Skip lateral capacitance part.\n"
    append usage "  --add-lcaps2                Add lateral capacitance part2.\n"
    append usage "  --cf-lcaps  FACTOR          Using compensate factor for lcaps values.\n"
    append usage "  --cf-lcaps2 FACTOR          Using compensate factor for lcaps2 values\n"
    append usage "                              (values are multiplied with for example 0.9).\n"
    append usage "  --skip-vcaps                Skip vertical capacitance part. This part\n"
    append usage "                              computes the area and edge-edge capacitances.\n"
    append usage "  --skip-vdim NAME,...        Skip vdimension name(s).\n"

    std::try {
        array unset ::main_options
        std::options_parse $format_list $args ::main_options
    } when user_exception {msg} {
        puts stderr $msg
        puts stderr $usage
        exit 1
    }

    if [info exists ::main_options(--no-color)] {
        std::disable_color
    }

    if [info exists ::main_options(--verbose)] {
        std::enable_verbosity
    } elseif [info exists ::main_options(--verboser)] {
        std::enable_verbosity
    }

    if [info exists ::main_options(--help)] {
        puts stderr $usage
        exit 0
    }

    if ![info exists ::main_options(--edge-ratio)] {
	set ::main_options(--edge-ratio) 10
    } elseif {$::main_options(--edge-ratio) < 5} {
	std::error_message "Too small edge-ratio specified (must be >= 5)."
	exit 1
    } elseif {$::main_options(--edge-ratio) > 100} {
	std::error_message "Too large edge-ratio specified (must be <= 100)."
	exit 1
    }
    if ![info exists ::main_options(--skip-vdim)] {
	set ::main_options(--skip-vdim) [list]
    }

    if [std::is_verbose] {
        ::tabs::progress_enable
    } elseif ![info exists ::main_options(--quiet)] {
        ::tabs::progress_enable
    }

    if [info exists ::main_options(-E)] {
        set ::main_options(-E) [std::validate_double $::main_options(-E) 1]
	if {$::main_options(-E) < 0.001} {
	    std::warning_message "Too small error-factor specified. Using 0.001"
	    set ::main_options(-E) 0.001
	}
    }

    if [info exists ::main_options(-M)] {
        if {![string equal $::main_options(-M) "0c"] &&
            ![string equal $::main_options(-M) "0g"] &&
            ![string equal $::main_options(-M) "1c"] &&
            ![string equal $::main_options(-M) "1g"]} {
            std::error_message "Please specify 0c|0g|1c|1g as argument to -M."
            exit 1
        }
    } else {
	set ::main_options(-M) "0c"
    }

    if [info exists ::main_options(--precision)] {
        if {![string equal $::main_options(--precision) "low"] &&
            ![string equal $::main_options(--precision) "medium"] &&
            ![string equal $::main_options(--precision) "high"] &&
            ![string equal $::main_options(--precision) "veryhigh"]} {
            std::error_message "Please specify low|medium|high as argument to --precision."
            exit 1
        }
    } else {
        set ::main_options(--precision) "low"
    }

    if [info exists ::main_options(--z-window)] {
        if [info exists ::main_options(-L)] {
            std::throw user_exception "Please specify either `--z-window' OR `-L', not both."
        }
        set ::main_options(--z-window) [expr 1e-6*[std::validate_double [lindex $::main_options(--z-window) 0] 1]]
	# scale z into 0.1nm units and round to integer.
	set ::main_options(--z-window) [expr round($::main_options(--z-window)*1e10)]
    } elseif [info exists ::main_options(-L)] {
        set ::main_options(-L) [std::validate_integer [lindex $::main_options(-L) 0] 1]
    } else {
        set ::main_options(-L) 2
    }

    if {[llength $::main_options(--)] != 0} {
        puts stderr $usage
        exit 1
    }

    if [info exists ::main_options(--sfile)] {
        set sfile [lindex $::main_options(--sfile) 0]
    } else {
        set sfile [::project_tools::determine_sfile [pwd]]
    }

    if [info exists ::main_options(--maskdata)] {
        set maskdata_file [lindex $::main_options(--maskdata) 0]
    } else {
        set maskdata_file [::project_tools::determine_maskdata [pwd]]
    }

    if ![file readable $sfile] {
        std::throw user_exception "Element definition file `$sfile' does not exist or is not readable."
    }
    if ![file writable $sfile] {
        std::throw user_exception "Technology file `$sfile' is not writable."
    }
    if ![file readable $maskdata_file] {
        std::throw user_exception "Maskdata file `$maskdata_file' does not exist or is not readable."
    }

    # Check if sfile already contains capacitance rules.
    #
    eval {
        set fp [open $sfile r]
        set text [read $fp]
        regsub {[\n]$} $text "" text
        set tecc_lines [split $text "\n"]
        close $fp

	set removed_lines [list]
	set tecc_lines [::tabs::remove_rules tabs-area-capacitance $tecc_lines removed_lines]
	set tecc_lines [::tabs::remove_rules tabs-edge-edge-capacitance $tecc_lines removed_lines]
	set tecc_lines [::tabs::remove_rules tabs-edge-surface-capacitance $tecc_lines removed_lines]
	set tecc_lines [::tabs::remove_rules tabs-lateral-capacitance $tecc_lines removed_lines]
	set tecc_lines [::tabs::remove_rules tabs-lateral-capacitance2 $tecc_lines removed_lines]
	unset removed_lines

        if [::tabs::contains_capacitance_rules $tecc_lines] {
            if ![info exists ::main_options(--ignore-preexisting)] {
                std::error_message "Technology file `$sfile' already contains `capacitance:' section. Use the `-i' option to ignore."
                exit 1
            }
        }
	unset tecc_lines
    }

    set ::main_options(fp) [open "tabs_summary.txt" w]
    puts $::main_options(fp) "tabs started at: [clock format [clock seconds]]"
    puts $::main_options(fp) "using techfile: $sfile"
    puts $::main_options(fp) "using maskdata: $maskdata_file"

    ::tabs::initialize

    array unset technology_table
    array unset precision_table

    # Initialize tables with information read from the technology file and
    # maskdata file.
    #
    ::project_tools::read_technology $sfile $maskdata_file technology_table

    set precision_table(be_window) "inf"

    if [info exists ::main_options(-E)] {
	set precision_table(error) $::main_options(-E)
    } elseif {$::main_options(--precision) == "low"} {
	set precision_table(error) 0.1
    } elseif {$::main_options(--precision) == "medium"} {
	set precision_table(error) 0.05
    } elseif {$::main_options(--precision) == "high"} {
	set precision_table(error) 0.05
    } else {
	set precision_table(error) 0.01
    }

    # Adjust the conditions of vdimensions in the technology table, so that the
    # conditions are true only when there is a conductor present with the same mask.
    #
    if [info exists ::main_options(--fix-vdim)] {
	fix_conditions_of_vdimensions technology_table
    }

    # The added contact condition for vertical capacitances must be fixed.
    #
    if [info exists technology_table(contacts:size)] {
	if [info exists ::main_options(--omit-vias)] {
	    set technology_table(contacts:size) 0
	} else {
	    fix_conditions_of_contacts technology_table
	}
    } else {
	set technology_table(contacts:size) 0
    }

    # We scale z, thickness and spacing into 0.1nm units and round to integer.
    #
    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
	set technology_table(vdimensions:$i:z) [expr round($technology_table(vdimensions:$i:z)*1e10)]
	set technology_table(vdimensions:$i:thickness) [expr round($technology_table(vdimensions:$i:thickness)*1e10)]
	if [info exists technology_table(vdimensions:$i:spacing)] {
	    set technology_table(vdimensions:$i:spacing) [expr round($technology_table(vdimensions:$i:spacing)*1e10)]
	} else {
	    set technology_table(vdimensions:$i:spacing) [expr $technology_table(vdimensions:$i:thickness)/2]
	}
    }

    # We scale the dielectrics data of the technology_table and round to integer.
    #
    array unset diel_table
    for {set i 0} {$i < $technology_table(dielectrics:size)} {incr i} {
	set diel_table($i:z) [expr round($technology_table(dielectrics:$i:z)*1e10)]
	set technology_table(dielectrics:$i:z) $diel_table($i:z)
	set diel_table($i:p) [expr round($technology_table(dielectrics:$i:permittivity)*1e3)]
	set technology_table(dielectrics:$i:permittivity) [expr $diel_table($i:p)/1e3]
    }
    set diel_table(size) $i

    # Add a virtual ground conductor to the technology table.
    #
    eval {
	set i $technology_table(vdimensions:size)
	set technology_table(vdimensions:$i:name) "@gnd"
	set technology_table(vdimensions:$i:mask) "@gnd"
	set technology_table(vdimensions:$i:condition) "1"
	set technology_table(vdimensions:$i:z) 0
	set technology_table(vdimensions:$i:thickness) 0
	incr technology_table(vdimensions:size)
	set technology_table(mask_order) [concat [list @gnd] $technology_table(mask_order)]
    }

    # Make `mask_index'.
    #
    eval {
	set index -1
	foreach mask $technology_table(mask_order) {
	    set technology_table(mask_index:$mask) [incr index]
	}
    }

    # Test for the --%fake debug flag. (Using it causes the actual capacitance
    # computations to be `faked', so that the control flow in the script can
    # be tested much faster.)
    #
    if [info exists ::main_options(--%fake)] {
	proc ::tabs::basic_layout::compute_capacitances {basic_layout_table diel_table precision_table _caps} {
	    puts stderr "USING OVERRIDDEN compute_capacitances"
	    upvar $_caps cap
	    set v [expr {1e-15*rand()}]
	    set cap(GND,tp1c) $v
	    set cap(tcc,tp1c) $v
	    set cap(tp1c,tp2c) $v
	    set cap(tp1c,tp1n1) $v
	    set cap(tp1c,tp1p1) $v
	}
    }

    ::tabs::create_project [file dirname $technology_table(tecc_path)]

    # Check lambda, must be 0.1 nm
    #
    std::invariant {$::tabs::default_lambda == 0.0001}

    # "Prime" the progress calculation system. This simply performs the
    # computations we are about to do, but with a special flag set in the
    # main_options array. The `::tabs::progress_prime' handles this for us.
    #
    ::tabs::progress_prime {
	set dummy [list]
	if ![info exists ::main_options(--skip-vcaps)] {
	    compute_vertical_capacitances diel_table technology_table precision_table dummy dummy
	}
	if ![info exists ::main_options(--skip-ecaps)] {
	    compute_edge_surface_capacitances diel_table technology_table precision_table dummy
	}
	if ![info exists ::main_options(--skip-lcaps)] {
	    compute_lateral_capacitances diel_table technology_table precision_table dummy
	}
	if [info exists ::main_options(--add-lcaps2)] {
	    compute_lateral_capacitances2 diel_table technology_table precision_table dummy
	}
	unset dummy
    }

    # Compute vertical capacitances.
    #
    if ![info exists ::main_options(--skip-vcaps)] {
	set area_capacitance_rules [list]
	set edge_edge_capacitance_rules [list]
	std::enable_verbosity
	std::verbose "Computing vertical capacitances ..."
	std::lower_verbosity
	compute_vertical_capacitances diel_table technology_table precision_table area_capacitance_rules edge_edge_capacitance_rules
	std::enable_verbosity
	std::verbose "Adding vertical capacitance rules to technology file ..."
	std::lower_verbosity
	::tabs::add_rules_to_file $sfile cap "tabs-area-capacitance" $area_capacitance_rules
	::tabs::add_rules_to_file $sfile cap "tabs-edge-edge-capacitance" $edge_edge_capacitance_rules
    }

    # Compute edge-surface capacitances.
    #
    if ![info exists ::main_options(--skip-ecaps)] {
	set edge_surface_capacitance_rules [list]
	std::enable_verbosity
	std::verbose "Computing edge-surface capacitances ..."
	std::lower_verbosity
	compute_edge_surface_capacitances diel_table technology_table precision_table edge_surface_capacitance_rules
	std::enable_verbosity
	std::verbose "Adding edge-surface capacitance rules to technology file ..."
	std::lower_verbosity
	::tabs::add_rules_to_file $sfile cap "tabs-edge-surface-capacitance" $edge_surface_capacitance_rules
    }

    # Compute lateral capacitances.
    #
    if ![info exists ::main_options(--skip-lcaps)] {
	set lateral_capacitance_rules [list]
	std::enable_verbosity
	std::verbose "Computing lateral capacitances ..."
	std::lower_verbosity
	compute_lateral_capacitances diel_table technology_table precision_table lateral_capacitance_rules
	std::enable_verbosity
	std::verbose "Adding lateral capacitance rules to technology file ..."
	std::lower_verbosity
	::tabs::add_rules_to_file $sfile cap "tabs-lateral-capacitance" $lateral_capacitance_rules
    }

    # Compute lateral capacitances part2.
    #
    if [info exists ::main_options(--add-lcaps2)] {
	set lateral_capacitance_rules [list]
	std::enable_verbosity
	std::verbose "Computing lateral capacitances part2 ..."
	std::lower_verbosity
	compute_lateral_capacitances2 diel_table technology_table precision_table lateral_capacitance_rules
	std::enable_verbosity
	std::verbose "Adding lateral capacitance part2 rules to technology file ..."
	std::lower_verbosity
	::tabs::add_rules_to_file $sfile cap "tabs-lateral-capacitance2" $lateral_capacitance_rules
    }

    # Finish progress tracking.
    #
    ::tabs::progress_finish

    puts $::main_options(fp) "finished at: [clock format [clock seconds]]"

    std::verbose "Done."
    exit 0
}

#===============================================================================
#  Get unique rule name.
#  This is a simple helper function used to generate unique rule names.
#===============================================================================

proc get_unique_rule_name {name} {

    regsub -all {[@<>]} $name "" name

    if [info exists ::rule_names($name)] {
        return ${name}_[incr ::rule_names($name)]
    } else {
        set ::rule_names($name) 0
        return $name
    }
}

#===============================================================================
#  Fix conditions in the vdimensions table, so that they are only true when
#  there is also a conductor of the same mask present.
#===============================================================================

proc fix_conditions_of_contacts {_technology_table} {
    upvar $_technology_table technology_table

    for {set i 0} {$i < $technology_table(contacts:size)} {incr i} {
        set msk1 $technology_table(contacts:$i:mask1)
        set msk2 $technology_table(contacts:$i:mask2)
	set list [list]
        foreach m $technology_table(contacts:$i:condition) {
	    if {$m == $msk1 || $m == $msk2} continue
	    if {[string index $m 0] == "!"} continue
	    set f 0
	    for {set j 0} {$j < $technology_table(conductors:size)} {incr j} {
		foreach p $technology_table(conductors:$j:condition) {
		    set p [string trimleft $p !]
		    if {$p == $m} {
			incr f
			break
		    }
		}
		if {$f} { break }
	    }
	    if {!$f} { lappend list $m }
	}
	set technology_table(contacts:$i:condition) ""
	set f 0
        foreach m $list {
	    if {$f} { append technology_table(contacts:$i:condition) " $m"
	    } else  { append technology_table(contacts:$i:condition) $m }
	    incr f
	}
    }
}

proc fix_conditions_of_vdimensions {_technology_table} {

    upvar $_technology_table technology_table

    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
        set condition1 $technology_table(vdimensions:$i:condition)
        set mask $technology_table(vdimensions:$i:mask)

        set condition2 [determine_conductor_condition_for_mask $mask technology_table]

        set condition [exec minimize "($condition1)($condition2)"]
        set technology_table(vdimensions:$i:condition) $condition
    }
}

# Determine the condition for a conductor coupled to the given mask.
# Used only by `fix_conditions_of_vdimensions' as a helper.
#
proc determine_conductor_condition_for_mask {mask _technology_table} {

    upvar $_technology_table technology_table

    set condition ""
    for {set i 0} {$i < $technology_table(conductors:size)} {incr i} {
        if ![string equal $technology_table(conductors:$i:mask) $mask] continue

        if [string length $condition] {
            append condition "|"
        }
        append condition "($technology_table(conductors:$i:condition))"
    }

    if ![string length $condition] {
        set condition "0"
    }

    return $condition
}

#===============================================================================
#  Sorting vdimensions.
#===============================================================================

array unset ::__compare_vdimensions_table

proc __compare_vdimensions_bottom {i j} {
    set z1b $::__compare_vdimensions_table(vdimensions:$i:z)
    set z2b $::__compare_vdimensions_table(vdimensions:$j:z)
    if {$z1b > $z2b} {return 1}
    if {$z1b < $z2b} {return -1}
    return 0
}

proc __compare_vdimensions_top {i j} {
    set z1t [expr {$::__compare_vdimensions_table(vdimensions:$i:z) + $::__compare_vdimensions_table(vdimensions:$i:thickness)}]
    set z2t [expr {$::__compare_vdimensions_table(vdimensions:$j:z) + $::__compare_vdimensions_table(vdimensions:$j:thickness)}]
    if {$z1t > $z2t} {return 1}
    if {$z1t < $z2t} {return -1}
    return 0
}

# Returns all indices of vdimensions, sorted by bottom z coordinate, and
# only those which are strictly higher than the top of the vdimension
# rule with the given index i.
#
proc get_sorted_vdimensions_above {i _technology_table} {

    upvar $_technology_table technology_table

    set z1b $technology_table(vdimensions:$i:z)
    set z1t [expr {$z1b + $technology_table(vdimensions:$i:thickness)}]
    set condition1 $technology_table(vdimensions:$i:condition)
    set mask1 $technology_table(vdimensions:$i:mask)

    set list [list]

    for {set j 0} {$j < $technology_table(vdimensions:size)} {incr j} {

        set mask2 $technology_table(vdimensions:$j:mask)
        if {$mask2 == $mask1} continue

        set z2b $technology_table(vdimensions:$j:z)
        set z2t [expr {$z2b + $technology_table(vdimensions:$j:thickness)}]
        set condition2 $technology_table(vdimensions:$j:condition)

        # Conductor2 should be strictly above conductor1.
        #
        if {$z2b <= $z1t} continue

        # Conditions should be unifiable.
        #
        set condition [exec minimize "($condition1)($condition2)"]
        if [string equal $condition "0"] continue

        lappend list $j
    }

    array set ::__compare_vdimensions_table [array get technology_table]
    set list [lsort -command __compare_vdimensions_bottom $list]

    return $list
}

# Similar to get_sorted_vdimensions_above, except it works in the opposite direction.
#
proc get_sorted_vdimensions_below {i _technology_table} {

    upvar $_technology_table technology_table

    set z1b $technology_table(vdimensions:$i:z)
    set z1t [expr {$z1b + $technology_table(vdimensions:$i:thickness)}]
    set condition1 $technology_table(vdimensions:$i:condition)
    set mask1 $technology_table(vdimensions:$i:mask)

    set list [list]

    for {set j 0} {$j < $technology_table(vdimensions:size)} {incr j} {

        set mask2 $technology_table(vdimensions:$j:mask)
        if {$mask2 == $mask1} continue

        set z2b $technology_table(vdimensions:$j:z)
        set z2t [expr {$z2b + $technology_table(vdimensions:$j:thickness)}]
        set condition2 $technology_table(vdimensions:$j:condition)

        # Conductor2 should be strictly below conductor1.
        #
        if {$z2t >= $z1b} continue

        # Conditions should be unifiable.
        #
        set condition [exec minimize "($condition1)($condition2)"]
        if [string equal $condition "0"] continue

        lappend list $j
    }

    array set ::__compare_vdimensions_table [array get technology_table]
    set list [lsort -decreasing -command __compare_vdimensions_top $list]

    return $list
}

proc skip_vdim {name} {
    foreach v [split $::main_options(--skip-vdim) ,] {
	if {$v == $name} { return 1 }
    }
    return 0
}

#===============================================================================
#  Compute vertical capacitances. This includes parallel plate capacitances
#  between metal layers, plate capacitances to ground, and vertical edge-edge
#  capacitances between metal layers.
#===============================================================================

proc compute_vertical_capacitances {_diel_table _technology_table _precision_table _area_capacitance_rules _edge_edge_capacitance_rules} {

    upvar $_diel_table diel_table
    upvar $_technology_table technology_table
    upvar $_precision_table precision_table
    upvar $_area_capacitance_rules area_capacitance_rules
    upvar $_edge_edge_capacitance_rules edge_edge_capacitance_rules

    set precision_table(be_mode) "0c"

    set area_capacitance_rules [list]
    set edge_edge_capacitance_rules [list]
    lappend area_capacitance_rules "capacitances: # area-capacitances"
    lappend edge_edge_capacitance_rules "capacitances: # edge-edge capacitances"

    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {

	if [skip_vdim $technology_table(vdimensions:$i:name)] continue

	set t1 $technology_table(vdimensions:$i:thickness)
	set z1b $technology_table(vdimensions:$i:z)
	set z1t [expr {$z1b + $t1}]
        set condition1 $technology_table(vdimensions:$i:condition)
        set mask1 $technology_table(vdimensions:$i:mask)
        set mask1_index $technology_table(mask_index:$mask1)
	set mask2_index_old $mask1_index
	set out_of_reach 0

        # Consider only layers above layer i. Hence, we will see each pair of
        # layers at most once.
        #
        set jlist [get_sorted_vdimensions_above $i technology_table]
        foreach j $jlist {

	    if [skip_vdim $technology_table(vdimensions:$j:name)] continue

	    # Skip layers with both zero thickness.
	    #
	    set t2 $technology_table(vdimensions:$j:thickness)
	    if {$t1 == 0 && $t2 == 0} continue

	    set z2b $technology_table(vdimensions:$j:z)
	    set z2t [expr {$z2b + $t2}]
            set condition2 $technology_table(vdimensions:$j:condition)
            set mask2 $technology_table(vdimensions:$j:mask)
            set mask2_index $technology_table(mask_index:$mask2)

            if $out_of_reach {
		# If conductor2 was out of reach, then don't examine
		# other conductors above it.
		#
		if {$mask2_index != $mask2_index_old} break
	    }
	    set mask2_index_old $mask2_index

	    set d [expr {$z2b - $z1t}]

            # If the two conductors are too far apart, then set a flag.
            #
            if [info exists ::main_options(--z-window)] {
                if {$d > $::main_options(--z-window)} {
                    set out_of_reach 1
                }
            }
            if [info exists ::main_options(-L)] {
                if {abs($mask1_index - $mask2_index) > $::main_options(-L)} {
                    set out_of_reach 1
                }
            }

	    # PROGRESS COMPUTATION
	    if [::tabs::progress_add] continue

            # Build "nothing_in_between_condition". This is the condition
            # stating that there should be no conductor between layer i
            # and j.
            #
            eval {
                set nothing_in_between_condition ""
                foreach k $jlist {
                    if {$k == $j} break
                    set conditionk $technology_table(vdimensions:$k:condition)
                    append nothing_in_between_condition "!($conditionk)"
                }
                if ![string length $nothing_in_between_condition] {
                    set nothing_in_between_condition "1"
                }
            }

            # Create a layout of two parallel plates. Initially, the side of
            # the plates is `l'. The plates are square.
            #

	    # Note: l is an integer (lambda = 0.1 nm)
	    if {$t1 > 0} {
		if {0.9*$d > $t2} {
		    set l [expr {10*$d}]
		} else {
		    set l [expr {30*$d}]
		}
	    } else {
		# good choice for i=ground
		set l [expr {20*$d}]
	    }

	    array unset basic_layout_table

	    set basic_layout_table(size) 2

	    set basic_layout_table(0:n) 1
	    set basic_layout_table(0:mask) mask$i
	    set basic_layout_table(0:width) $l
	    set basic_layout_table(0:length) $l
	    set basic_layout_table(0:spacing) 0
	    set basic_layout_table(0:z) $z1b
	    set basic_layout_table(0:thickness) $t1

	    set basic_layout_table(1:n) 1
	    set basic_layout_table(1:mask) mask$j
	    set basic_layout_table(1:width) $l
	    set basic_layout_table(1:length) $l
	    set basic_layout_table(1:spacing) 0
	    set basic_layout_table(1:z) $z2b
	    set basic_layout_table(1:thickness) $t2

	    # set be_area in micron
	    set precision_table(be_area) [expr {1.9e-8*$l*$l}]

	    if {$z1b == 0} {set n1 "GND"} else {set n1 "tcc"}

            # Extract the capacitance between the two plates.
            #
            array unset cap
            ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
            set c1 0.0; catch {set c1 $cap($n1,tp1c)}

            # Now, double the sides of the square (gives a factor of 4 increase in area.)
            #
	    set l2 [expr {2*$l}]
            set basic_layout_table(0:width)  $l2
            set basic_layout_table(0:length) $l2
            set basic_layout_table(1:width)  $l2
            set basic_layout_table(1:length) $l2

	    set precision_table(be_area) [expr {4*$precision_table(be_area)}]

            # Extract the capacitance between the two plates.
            #
            array unset cap
            ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
            set c2 0.0; catch {set c2 $cap($n1,tp1c)}

            # Now, using these two values (c1 and c2), compute the value of
            # `cpp_3d', which is the coupling capacitance per unit area.
            # Also compute cpp_ideal, which is the ideal parallel plate capacitance.
            #
	    # set l (to double) in meter
	    set l [expr {1e-10*$l}]
            set cpp_3d [expr {($c2-2*$c1)/(2*$l*$l)}]
            set cpp_ideal [::tabs::get_ideal_plate_capacitance_between_layers $z1t $z2b technology_table]

	    set rel_error [expr {round(1e4*abs($cpp_ideal-$cpp_3d)/$cpp_ideal)}]
	    set rel_error [format %g [expr {1e-2*$rel_error}]]

            # Also compute `cee_3d', the vertical edge-edge capacitance
            # between the two conductors.
            #
	    set cee_3d [expr {(4*$c1-$c2)/(8*$l)}]

	    # Output a simple rule for the parallel plate coupling.
	    #
	    set rule_name [get_unique_rule_name cpp_${mask1}_${mask2}]
	    set condition "($condition1)($condition2)($nothing_in_between_condition)"

	    # Try to add via condition
	    #
	    for {set k 0} {$k < $technology_table(contacts:size)} {incr k} {
		set m1 $technology_table(contacts:$k:mask1)
		set m2 $technology_table(contacts:$k:mask2)
		if {($m1 == $mask1 && $m2 == $mask2) || ($m2 == $mask1 && $m1 == $mask2)} {
		    set cc $technology_table(contacts:$k:condition)
		    if {$cc != ""} { append condition "!($cc)" }
		    break
		}
	    }
	    set condition [exec minimize $condition]
	    set c "  "
	    if [string equal $condition "0"] {set c "##"}
	    lappend area_capacitance_rules "$c  $rule_name : $condition : $mask1 $mask2 : %%UNIT a_capacitance $cpp_ideal%% # %%UNIT a_capacitance $cpp_3d%% ($rel_error% error)"

	    # Skip edge caps with zero thickness.
	    #
	    if {$t1 == 0} continue

	    # Output a simple rule for the edge-edge capacitance. If the
	    # layer j is "out of reach", then layer j is considered to be
	    # omnipresent, and then there are no edge-edge capacitances.
	    #
	    set rule_name [get_unique_rule_name cee_${mask1}_${mask2}]

	    set condition "!($condition1)!($condition2)"
	    foreach c $condition1 {
		if [string equal $c $mask1] {
		    append condition "!$mask1"
		    break
		}
	    }
	    foreach c $condition2 {
		if [string equal $c $mask2] {
		    append condition "!$mask2"
		    break
		}
	    }
	    append condition "($nothing_in_between_condition)([tabs::edge_condition $condition1])([tabs::edge_condition $condition2])"
	    set condition [exec minimize $condition]

	    set c "  "
	    if {[string equal $condition "0"] || $cee_3d <= 0.0} {set c "##"}
	    lappend edge_edge_capacitance_rules "$c  $rule_name : $condition : -$mask1 -$mask2 : %%UNIT e_capacitance $cee_3d%%"
        }
    }
}

#===============================================================================
#  Compute edge-surface capacitances.
#===============================================================================

proc compute_edge_surface_capacitances {_diel_table _technology_table _precision_table _edge_surface_capacitance_rules} {

    upvar $_diel_table diel_table
    upvar $_technology_table technology_table
    upvar $_precision_table precision_table
    upvar $_edge_surface_capacitance_rules edge_surface_capacitance_rules

    set precision_table(be_mode) $::main_options(-M)

    set edge_surface_capacitance_rules [list]
    lappend edge_surface_capacitance_rules "# Precision: $::main_options(--precision) (be_mode=$precision_table(be_mode) error=$precision_table(error))"
    lappend edge_surface_capacitance_rules "# Edge-ratio: $::main_options(--edge-ratio)"
    lappend edge_surface_capacitance_rules "capacitances: # edge-surface capacitances"

    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {

	if [skip_vdim $technology_table(vdimensions:$i:name)] continue

	# Skip edge caps with zero thickness (and @gnd mask).
	#
	set t1 $technology_table(vdimensions:$i:thickness)
	if {$t1 <= 0} continue

	set spacing $technology_table(vdimensions:$i:spacing)
	if {$spacing <= 0} {
	    set spacing [expr {$t1/2}]
	    if {$spacing == 0} {incr spacing}
	}

        set z1b [expr $technology_table(vdimensions:$i:z)]
        set z1t [expr $z1b + $t1]
        set condition1 $technology_table(vdimensions:$i:condition)
	set mask1 $technology_table(vdimensions:$i:mask)
        set mask1_index $technology_table(mask_index:$mask1)

	set found 0
        foreach c $condition1 {
	    if [string equal $c $mask1] { incr found; break }
	}
	set first 1
	set prev_factor 512

        foreach direction {i_above i_below} {

            if {$direction == "i_above"} {
                set jlist [get_sorted_vdimensions_below $i technology_table]
            } else {
                set jlist [get_sorted_vdimensions_above $i technology_table]
            }
	    set mask2_index_old $mask1_index
	    set out_of_reach 0

            foreach j $jlist {

		if [skip_vdim $technology_table(vdimensions:$j:name)] continue

		set t2 $technology_table(vdimensions:$j:thickness)
                set z2b [expr $technology_table(vdimensions:$j:z)]
                set z2t [expr $z2b + $t2]
                set condition2 $technology_table(vdimensions:$j:condition)
                set mask2 $technology_table(vdimensions:$j:mask)
                set mask2_index $technology_table(mask_index:$mask2)

		if $out_of_reach {
		    # If conductor2 was out of reach, then don't examine other conductors
		    #
		    if {$mask2_index != $mask2_index_old} break
		}
		set mask2_index_old $mask2_index

                # Determine which conductor is higher.
                #
                if {$direction == "i_below"} {
                    set zdiff [expr $z2b - $z1t]
                } else {
                    set zdiff [expr $z1b - $z2t]
                }

                # If the two conductors are too far apart, then this is the
                # last pair we examine.
                #
                if [info exists ::main_options(--z-window)] {
                    if {$zdiff > $::main_options(--z-window)} {
                        set out_of_reach 1
                    }
                }
                if [info exists ::main_options(-L)] {
                    if {abs($mask1_index - $mask2_index) > $::main_options(-L)} {
                        set out_of_reach 1
                    }
                }

                # Build sub_rules
                #
                set sub_rules [list]
                set produced_sub_rules 0
		set prev_ce 0.0

		set w  [expr {4*$zdiff}]
		set w2 [expr {2*$w}]
		set l  [expr {$w * $::main_options(--edge-ratio)}]
		set lm [expr {2e-10*$l}]

		# set be_area in micron
		# the area must be < area1 and > area2
		#
		set precision $::main_options(--precision)
		if {$precision == "veryhigh"} {
		    set area1 [expr {0.25e-8 * $l * $l / 16}]
		    set area2 [expr {1e-8 * $w * $l / 4}]
		    if {$area2 >= $area1} { set precision "high" }
		}
		if {$precision == "high"} {
		    set area1 [expr {0.25e-8 * $l * $l / 4}]
		    set area2 [expr {1e-8 * $w * $l / 2}]
		    if {$area2 >= $area1} { set precision "medium" }
		}
		if {$precision == "medium"} {
		    set area1 [expr {0.25e-8 * $l * $l}]
		    set area2 [expr {1e-8 * $w * $l}]
		    if {$area2 >= $area1} { set precision "low" }
		}
		if {$precision == "low"} {
		    set precision_table(be_area) [expr {1e-8 * $l * $l}]
		} else {
		    set precision_table(be_area) [expr {($area1+$area2)/2}]
		}

		if {$first || [string equal $mask2 "@gnd"]} { set max_s_factor 512 } else { set max_s_factor $prev_factor }

		foreach s_factor {1 2 4 8 16 32 64 128 256 512} {

		    # PROGRESS COMPUTATION
		    if [::tabs::progress_add] continue

		    if {$s_factor > $max_s_factor} break

		    set s [expr $spacing * $s_factor]
		    # Note: s, w and l are integers (lambda = 0.1 nm)

		    set W [expr {3*$w + 2*$s}]

		    array unset basic_layout_table

		    set basic_layout_table(0:n) 1
		    set basic_layout_table(0:mask) mask$j
		    set basic_layout_table(0:width) $W
		    set basic_layout_table(0:length) $l
		    set basic_layout_table(0:spacing) 0
		    if {$direction == "i_below"} {
			# add dummy ground mask
			set basic_layout_table(0:z) 0
			set basic_layout_table(0:thickness) 0
		    } else {
			set basic_layout_table(0:z) $z2b
			set basic_layout_table(0:thickness) $t2
		    }

		    set basic_layout_table(1:n) 3
		    set basic_layout_table(1:mask) mask$i
		    set basic_layout_table(1:width) $w
		    set basic_layout_table(1:length) $l
		    set basic_layout_table(1:spacing) $s
		    set basic_layout_table(1:z) $z1b
		    set basic_layout_table(1:thickness) $t1

		    if {$direction == "i_below"} {
			set basic_layout_table(2:n) 1
			set basic_layout_table(2:mask) mask$j
			set basic_layout_table(2:width) $W
			set basic_layout_table(2:length) $l
			set basic_layout_table(2:spacing) 0
			set basic_layout_table(2:z) $z2b
			set basic_layout_table(2:thickness) $t2
			set basic_layout_table(size) 3
			std::verbose "Computing edge-top capacitance $mask1 $mask2 at distance [format %g [expr {$s/1e4}]] ..."
		    } else {
			set basic_layout_table(size) 2
			std::verbose "Computing edge-bottom capacitance $mask1 $mask2 at distance [format %g [expr {$s/1e4}]] ..."
		    }

		    if {$direction == "i_below"} {
			set n1 "tp1c"
			set n2 "tp2c"
		    } else {
			if {$z2b == 0} {set n1 "GND"} else {set n1 "tcc"}
			set n2 "tp1c"
		    }

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c1 0.0; catch {set c1 $cap($n1,$n2)}

		    set W [expr {3*$w2 + 2*$s}]
		    set basic_layout_table(0:width) $W
		    set basic_layout_table(1:width) $w2
		    if {$basic_layout_table(size) == 3} {
			set basic_layout_table(2:width) $W
		    }

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c2 0.0; catch {set c2 $cap($n1,$n2)}

		    if [info exists ::main_options(--fake)] {
			incr produced_sub_rules
			continue
		    }

		    # This is the main formula used to determine the edge-surface capacitance.
		    #
		    set ce [expr {(2*$c1-$c2)/$lm}]

		    set s [expr {1e-10*$s}]
		    if {$ce <= 0.0} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $ce%% (outcommented, because not greater than zero)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    # This should not happen. But if it does (most likely
		    # due to inaccuracies), then don't output the next line.
		    #
		    if {$ce <= $prev_ce} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $ce%% (outcommented, because not greater than previous value)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    set cx [expr {abs($ce - $prev_ce)/$ce}]
		    lappend sub_rules "                                      %%UNIT distance $s%% %%UNIT e_capacitance $ce%%"
		    incr produced_sub_rules
		    if {$first} { set prev_factor $s_factor }

		    if {$s_factor > 2 && $cx < $prev_cx && $cx < $precision_table(error)} {
			std::verbose "No significant change in capacitance between this and last computation; stopping."
			break
		    }

		    set prev_ce $ce
		    set prev_cx $cx
		}

		if {$first && ![string equal $mask2 "@gnd"]} { set first 0 }

                # If somehow we produced no rules, then don't do anything else here.
                # This happens in error situations or by progress computation.
                #
		if !$produced_sub_rules continue

                # Build "nothing_in_between_condition"
                #
                eval {
                    set nothing_in_between_condition ""
                    foreach k $jlist {
                        if {$k == $j} break
                        set conditionk $technology_table(vdimensions:$k:condition)
                        append nothing_in_between_condition "!($conditionk)"
                    }
                    if ![string length $nothing_in_between_condition] {
                        set nothing_in_between_condition "1"
                    }
                }

		set condition "!($condition1)"
		if {$found} { append condition "!$mask1" }
		append condition "([::tabs::edge_condition $condition1])($condition2)($nothing_in_between_condition)"
		set condition [exec minimize $condition]

		if {$condition != "0"} {
		    set rule_name [get_unique_rule_name ecap_${mask1}_${mask2}]
		    if [info exists ::main_options(--fake)] {
			lappend edge_surface_capacitance_rules "#   ${rule_name} : $condition : -$mask1 $mask2 : 0"
		    } else {
			lappend edge_surface_capacitance_rules "    ${rule_name} : $condition : -$mask1 $mask2 :"
			eval lappend edge_surface_capacitance_rules $sub_rules
			lappend edge_surface_capacitance_rules ""
		    }
		}
            }
        }
    }
}

#===============================================================================
#  Compute lateral capacitances.
#===============================================================================

proc compute_lateral_capacitances {_diel_table _technology_table _precision_table _lateral_capacitance_rules} {

    upvar $_diel_table diel_table
    upvar $_technology_table technology_table
    upvar $_precision_table precision_table
    upvar $_lateral_capacitance_rules lateral_capacitance_rules

    set precision_table(be_mode) $::main_options(-M)

    set lateral_capacitance_rules [list]
    lappend lateral_capacitance_rules "# Precision: $::main_options(--precision) (be_mode=$precision_table(be_mode) error=$precision_table(error))"
    lappend lateral_capacitance_rules "# Edge-ratio: $::main_options(--edge-ratio)"
if [info exists ::main_options(--cf-lcaps)] {
    lappend lateral_capacitance_rules "# Using compensate factor: $::main_options(--cf-lcaps)"
}
    lappend lateral_capacitance_rules "capacitances: # lateral capacitances"

    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {

	if [skip_vdim $technology_table(vdimensions:$i:name)] continue

	# Skip lateral caps with zero thickness (and @gnd mask).
	#
	set t1 $technology_table(vdimensions:$i:thickness)
	if {$t1 <= 0} continue

	set spacing $technology_table(vdimensions:$i:spacing)
	if {$spacing <= 0} {
	    set spacing [expr $t1/2]
	    if {$spacing == 0} {incr spacing}
	}

	set w [expr $spacing * 8]
	set l [expr $spacing * 4 * $::main_options(--edge-ratio)]
	set l2 [expr 2*$l]
	set lm [expr 2e-10*$l]

	# set be_area in micron
	# the area must be < area1 and > area2
	#
	set precision $::main_options(--precision)
	if {$precision == "veryhigh"} {
	    set area1 [expr 0.25e-8 * $l * $l / 64]
	    set area2 [expr 1e-8 * $w * $l / 16]
	    if {$area2 >= $area1} { set precision "high" }
	}
	if {$precision == "high"} {
	    set area1 [expr 0.25e-8 * $l * $l / 16]
	    set area2 [expr 1e-8 * $w * $l / 8]
	    if {$area2 >= $area1} { set precision "medium" }
	}
	if {$precision == "medium"} {
	    set area1 [expr 0.25e-8 * $l * $l / 4]
	    set area2 [expr 1e-8 * $w * $l / 4]
	    if {$area2 >= $area1} { set precision "low" }
	}
	if {$precision == "low"} {
	    set area1 [expr 0.25e-8 * $l * $l]
	    set area2 [expr 1e-8 * $w * $l / 2]
	}
	set precision_table(be_area) [expr ($area1+$area2)/2]

        set z1b [expr $technology_table(vdimensions:$i:z)]
        set z1t [expr $z1b + $t1]
        set condition1 $technology_table(vdimensions:$i:condition)
	set mask1 $technology_table(vdimensions:$i:mask)
        set mask1_index $technology_table(mask_index:$mask1)

	set found 0
        foreach c $condition1 {
	    if [string equal $c $mask1] { incr found; break }
	}

        set jlist [list]
	if [info exists ::main_options(-L)] {
	    foreach j [get_sorted_vdimensions_below $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set mask2 $technology_table(vdimensions:$j:mask)
		if [string equal $mask2 "@gnd"] continue
		set mask2_index $technology_table(mask_index:$mask2)
                if {abs($mask1_index - $mask2_index) > $::main_options(-L)} break
		lappend jlist $j
	    }
	}
	if [info exists ::main_options(--z-window)] {
	    foreach j [get_sorted_vdimensions_below $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set z2b [expr $technology_table(vdimensions:$j:z)]
		if {$z2b == 0} continue
		set z2t [expr $z2b + $technology_table(vdimensions:$j:thickness)]
		if {$z1b - $z2t > $::main_options(--z-window)} break
		lappend jlist $j
	    }
	}
	lappend jlist 99

        set klist [list]
	if [info exists ::main_options(-L)] {
	    foreach j [get_sorted_vdimensions_above $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set mask3 $technology_table(vdimensions:$j:mask)
		set mask3_index $technology_table(mask_index:$mask3)
                if {abs($mask1_index - $mask3_index) > $::main_options(-L)} break
		lappend klist $j
	    }
	}
	if [info exists ::main_options(--z-window)] {
	    foreach j [get_sorted_vdimensions_above $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set zdelta [expr $technology_table(vdimensions:$j:z) - $z1t]
		if {$zdelta > $::main_options(--z-window)} break
		lappend klist $j
	    }
	}
	lappend klist 99

        foreach j $jlist {
	    if {$j < 99} {
		set condition2 $technology_table(vdimensions:$j:condition)
		set mask2 $technology_table(vdimensions:$j:mask)
	    } else { set mask2 "<none>" }

            foreach k $klist {
		if {$k < 99} {
		    set condition3 $technology_table(vdimensions:$k:condition)
		    set mask3 $technology_table(vdimensions:$k:mask)
		} else { set mask3 "<none>" }

                # Produce sub-rules.
                #
		set prev_cl 0.0
		set sub_rules [list]
		set produced_sub_rules 0

		foreach s_factor {1 2 4 8 16 32} {

		    # PROGRESS COMPUTATION
		    if [::tabs::progress_add] continue

		    set s [expr $spacing * $s_factor]
		    # Note: s, w and l are integers (lambda = 0.1 nm)

		    array unset basic_layout_table

		    # Create a layer below the victim layer.
		    set basic_layout_table(0:n) 1
		    set basic_layout_table(0:mask) mask$j
		    set basic_layout_table(0:width) [expr 3*$w + 2*$s]
		    set basic_layout_table(0:length) $l
		    set basic_layout_table(0:spacing) 0
		    if {$j < 99} {
			set basic_layout_table(0:z) $technology_table(vdimensions:$j:z)
			set basic_layout_table(0:thickness) $technology_table(vdimensions:$j:thickness)
		    } else {
			set basic_layout_table(0:z) 0
			set basic_layout_table(0:thickness) 0
		    }

		    eval {
			set basic_layout_table(1:n) 3
			set basic_layout_table(1:mask) mask$i
			set basic_layout_table(1:width) $w
			set basic_layout_table(1:length) $l
			set basic_layout_table(1:spacing) $s
			set basic_layout_table(1:z) $z1b
			set basic_layout_table(1:thickness) $t1
		    }

		    # Create a layer above the victim layer.
		    if {$k < 99} {
			set basic_layout_table(2:n) 1
			set basic_layout_table(2:mask) mask$k
			set basic_layout_table(2:width) [expr 3*$w + 2*$s]
			set basic_layout_table(2:length) $l
			set basic_layout_table(2:spacing) 0
			set basic_layout_table(2:z) $technology_table(vdimensions:$k:z)
			set basic_layout_table(2:thickness) $technology_table(vdimensions:$k:thickness)
			set basic_layout_table(size) 3
		    } else {
			set basic_layout_table(size) 2
		    }

		    std::verbose "Computing lateral capacitance for $mask1 at distance [format %g [expr $s/1e4]], with $mask2 below and $mask3 above ..."

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c1 0.0; catch {set c1 [expr {$cap(tp1c,tp1n1) + $cap(tp1c,tp1p1)}]}

		    set basic_layout_table(0:length) $l2
		    set basic_layout_table(1:length) $l2
		    if {$k < 99} { set basic_layout_table(2:length) $l2 }

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c2 0.0; catch {set c2 [expr {$cap(tp1c,tp1n1) + $cap(tp1c,tp1p1)}]}

		    if [info exists ::main_options(--fake)] {
			incr produced_sub_rules
			continue
		    }

		    # This is the main formula used to determine the lateral capacitance.
		    #
		    set cl [expr ($c2-$c1)/$lm]

		    set s [expr $s/1e10]
		    if {$cl <= 0.0} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $cl%% (outcommented, because not greater than zero)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    # This should not happen. But if it does (most likely
		    # due to inaccuracies), then don't output the next line.
		    #
		    if {$s_factor > 1 && $cl >= $prev_cl} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $cl%% (outcommented, because not smaller than previous value)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    set cx $cl
		    if [info exists ::main_options(--cf-lcaps)] { set cx [expr {$cl * $::main_options(--cf-lcaps)}] }

		    lappend sub_rules "                                      %%UNIT distance $s%% %%UNIT e_capacitance $cx%%"
		    incr produced_sub_rules

		    set cx [expr {abs($cl - $prev_cl)/$cl}]
		    if {$s_factor > 2 && $cx < $prev_cx && $cx < $precision_table(error)} {
			std::verbose "No significant change in capacitance between this and last computation; stopping."
			break
		    }

		    set prev_cl $cl
		    set prev_cx $cx
		}

		# Output the sub-rules.
		#
		if {$produced_sub_rules} {

		    # Build "condition"
		    #
		    eval {
			set condition "!($condition1)"
			if {$found} { append condition "!$mask1" }
			append condition "([::tabs::edge_condition $condition1 -])([::tabs::edge_condition $condition1 =])"

			foreach j2 $jlist {
			    if {$j2 == $j} break
			    set condition_j2 $technology_table(vdimensions:$j2:condition)
			    append condition "!($condition_j2)"
			}
			if {$j < 99} { append condition "($condition2)" }

			foreach k2 $klist {
			    if {$k2 == $k} break
			    set condition_k2 $technology_table(vdimensions:$k2:condition)
			    append condition "!($condition_k2)"
			}
			if {$k < 99} { append condition "($condition3)" }

			set condition [exec minimize $condition]
		    }

		    if {$condition != "0"} {
			set rule_name [get_unique_rule_name lcap_${mask1}_${mask2}_${mask3}]
			if [info exists ::main_options(--fake)] {
			    lappend lateral_capacitance_rules "#   ${rule_name} : $condition : -$mask1 =$mask1 : 0"
			} else {
			    lappend lateral_capacitance_rules "    ${rule_name} : $condition : -$mask1 =$mask1 :"
			    eval lappend lateral_capacitance_rules $sub_rules
			    lappend lateral_capacitance_rules ""
			}
		    }
		}
            }
        }
    }
}

#===============================================================================
#  Compute lateral capacitances part2.
#===============================================================================

proc compute_lateral_capacitances2 {_diel_table _technology_table _precision_table _lateral_capacitance_rules} {

    upvar $_diel_table diel_table
    upvar $_technology_table technology_table
    upvar $_precision_table precision_table
    upvar $_lateral_capacitance_rules lateral_capacitance_rules

    set precision_table(be_mode) $::main_options(-M)

    set lateral_capacitance_rules [list]
    lappend lateral_capacitance_rules "# Precision: $::main_options(--precision) (be_mode=$precision_table(be_mode) error=$precision_table(error))"
    lappend lateral_capacitance_rules "# Edge-ratio: $::main_options(--edge-ratio)"
if [info exists ::main_options(--cf-lcaps2)] {
    lappend lateral_capacitance_rules "# Using compensate factor: $::main_options(--cf-lcaps2)"
}
    lappend lateral_capacitance_rules "capacitances: # lateral capacitances2"

    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {

	if [skip_vdim $technology_table(vdimensions:$i:name)] continue

	# Skip lateral caps with zero thickness (and @gnd mask).
	#
	set t1 $technology_table(vdimensions:$i:thickness)
	if {$t1 <= 0} continue

	set spacing $technology_table(vdimensions:$i:spacing)
	if {$spacing <= 0} {
	    set spacing [expr $t1/2]
	    if {$spacing == 0} {incr spacing}
	}

	set w [expr $spacing * 8]
	set l [expr $spacing * 4 * $::main_options(--edge-ratio)]
	set l2 [expr 2*$l]
	set lm [expr 2e-10*$l]

	# set be_area in micron
	# the area must be < area1 and > area2
	#
	set precision $::main_options(--precision)
	if {$precision == "veryhigh"} {
	    set area1 [expr 0.25e-8 * $l * $l / 64]
	    set area2 [expr 1e-8 * $w * $l / 16]
	    if {$area2 >= $area1} { set precision "high" }
	}
	if {$precision == "high"} {
	    set area1 [expr 0.25e-8 * $l * $l / 16]
	    set area2 [expr 1e-8 * $w * $l / 8]
	    if {$area2 >= $area1} { set precision "medium" }
	}
	if {$precision == "medium"} {
	    set area1 [expr 0.25e-8 * $l * $l / 4]
	    set area2 [expr 1e-8 * $w * $l / 4]
	    if {$area2 >= $area1} { set precision "low" }
	}
	if {$precision == "low"} {
	    set area1 [expr 0.25e-8 * $l * $l]
	    set area2 [expr 1e-8 * $w * $l / 2]
	}
	set precision_table(be_area) [expr ($area1+$area2)/2]

        set z1b [expr $technology_table(vdimensions:$i:z)]
        set z1t [expr $z1b + $t1]
        set condition1 $technology_table(vdimensions:$i:condition)
	set mask1 $technology_table(vdimensions:$i:mask)
        set mask1_index $technology_table(mask_index:$mask1)

	set found 0
        foreach c $condition1 {
	    if [string equal $c $mask1] { incr found; break }
	}

        set jlist [list]
	if [info exists ::main_options(-L)] {
	    foreach j [get_sorted_vdimensions_below $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set mask2 $technology_table(vdimensions:$j:mask)
		if [string equal $mask2 "@gnd"] continue
		set mask2_index $technology_table(mask_index:$mask2)
                if {abs($mask1_index - $mask2_index) > $::main_options(-L)} break
		lappend jlist $j
	    }
	}
	if [info exists ::main_options(--z-window)] {
	    foreach j [get_sorted_vdimensions_below $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set z2b [expr $technology_table(vdimensions:$j:z)]
		if {$z2b == 0} continue
		set z2t [expr $z2b + $technology_table(vdimensions:$j:thickness)]
		if {$z1b - $z2t > $::main_options(--z-window)} break
		lappend jlist $j
	    }
	}
	lappend jlist 99

        set klist [list]
	if [info exists ::main_options(-L)] {
	    foreach j [get_sorted_vdimensions_above $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set mask3 $technology_table(vdimensions:$j:mask)
		set mask3_index $technology_table(mask_index:$mask3)
                if {abs($mask1_index - $mask3_index) > $::main_options(-L)} break
		lappend klist $j
	    }
	}
	if [info exists ::main_options(--z-window)] {
	    foreach j [get_sorted_vdimensions_above $i technology_table] {
		if [skip_vdim $technology_table(vdimensions:$j:name)] continue
		set zdelta [expr $technology_table(vdimensions:$j:z) - $z1t]
		if {$zdelta > $::main_options(--z-window)} break
		lappend klist $j
	    }
	}
	lappend klist 99

        foreach j $jlist {
	    if {$j < 99} {
		set condition2 $technology_table(vdimensions:$j:condition)
		set mask2 $technology_table(vdimensions:$j:mask)
	    } else { set mask2 "<none>" }

            foreach m $klist {
                if {$m == 99} break
		set cond4 $technology_table(vdimensions:$m:condition)
		set mask4 $technology_table(vdimensions:$m:mask)

            foreach k $klist {
		if {$k <= $m} continue
		if {$k < 99} {
                    set condition3 $technology_table(vdimensions:$k:condition)
                    set mask3 $technology_table(vdimensions:$k:mask)
                } else { set mask3 "<none>" }

                # Produce sub-rules.
                #
		set prev_cl 0.0
		set sub_rules [list]
		set produced_sub_rules 0

		foreach s_factor {1 2 4 8 16 32} {

		    # PROGRESS COMPUTATION
		    if [::tabs::progress_add] continue

		    set s [expr $spacing * $s_factor]
		    # Note: s, w and l are integers (lambda = 0.1 nm)

		    array unset basic_layout_table

		    # Create a layer below the victim layer.
		    set basic_layout_table(0:n) 1
		    set basic_layout_table(0:mask) mask$j
		    set basic_layout_table(0:width) [expr 3*$w + 2*$s]
		    set basic_layout_table(0:length) $l
		    set basic_layout_table(0:spacing) 0
		    if {$j < 99} {
			set basic_layout_table(0:z) $technology_table(vdimensions:$j:z)
			set basic_layout_table(0:thickness) $technology_table(vdimensions:$j:thickness)
		    } else {
			set basic_layout_table(0:z) 0
			set basic_layout_table(0:thickness) 0
		    }

		    set basic_layout_table(1:n) 1
		    set basic_layout_table(1:mask) mask$i
		    set basic_layout_table(1:x1) [expr $w + $s]
		    set basic_layout_table(1:width) $w
		    set basic_layout_table(1:length) $l
		    set basic_layout_table(1:spacing) 0
		    set basic_layout_table(1:z) $z1b
		    set basic_layout_table(1:thickness) $t1

		    set basic_layout_table(2:n) 2
		    set basic_layout_table(2:mask) mask$m
		    set basic_layout_table(2:width) $w
		    set basic_layout_table(2:length) $l
		    set basic_layout_table(2:spacing) [expr $w + 2*$s]
		    set basic_layout_table(2:z) $technology_table(vdimensions:$m:z)
		    set basic_layout_table(2:thickness) $technology_table(vdimensions:$m:thickness)

		    # Create a layer above the victim layer.
		    if {$k < 99} {
			set basic_layout_table(3:n) 1
			set basic_layout_table(3:mask) mask$k
			set basic_layout_table(3:width) [expr 3*$w + 2*$s]
			set basic_layout_table(3:length) $l
			set basic_layout_table(3:spacing) 0
			set basic_layout_table(3:z) $technology_table(vdimensions:$k:z)
			set basic_layout_table(3:thickness) $technology_table(vdimensions:$k:thickness)
			set basic_layout_table(size) 4
		    } else {
			set basic_layout_table(size) 3
		    }

		    std::verbose "Computing lateral capacitance for $mask1 at distance [format %g [expr $s/1e4]], with $mask2 below and $mask3 above ..."

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c1 0.0; catch {set c1 [expr {$cap(tp1c,tp2c) + $cap(tp1c,tp2p1)}]}

		    set basic_layout_table(0:length) $l2
		    set basic_layout_table(1:length) $l2
		    set basic_layout_table(2:length) $l2
		    if {$k < 99} { set basic_layout_table(3:length) $l2 }

		    array unset cap
		    ::tabs::basic_layout::compute_capacitances basic_layout_table diel_table precision_table cap
		    set c2 0.0; catch {set c2 [expr {$cap(tp1c,tp2c) + $cap(tp1c,tp2p1)}]}

		    if [info exists ::main_options(--fake)] {
			incr produced_sub_rules
			continue
		    }

		    # This is the main formula used to determine the lateral capacitance.
		    #
		    set cl [expr ($c2-$c1)/$lm]

		    set s [expr $s/1e10]
		    if {$cl <= 0.0} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $cl%% (outcommented, because not greater than zero)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    # This should not happen. But if it does (most likely
		    # due to inaccuracies), then don't output the next line.
		    #
		    if {$s_factor > 1 && $cl >= $prev_cl} {
			#lappend sub_rules "                                    # %%UNIT distance $s%% %%UNIT e_capacitance $cl%% (outcommented, because not smaller than previous value)"
			std::verbose "No increase in capacitance wrt. previous computation; stopping."
			break
		    }

		    set cx $cl
		    if [info exists ::main_options(--cf-lcaps2)] { set cx [expr {$cl * $::main_options(--cf-lcaps2)}] }

		    lappend sub_rules "                                      %%UNIT distance $s%% %%UNIT e_capacitance $cx%%"
		    incr produced_sub_rules

		    set cx [expr {abs($cl - $prev_cl)/$cl}]
		    if {$s_factor > 2 && $cx < $prev_cx && $cx < $precision_table(error)} {
			std::verbose "No significant change in capacitance between this and last computation; stopping."
			break
		    }

		    set prev_cl $cl
		    set prev_cx $cx
		}

		# Output the sub-rules.
		#
		if {$produced_sub_rules} {

		    # Build "condition"
		    #
		    eval {
			set condition "!($condition1)"
			if {$found} { append condition "!$mask1" }
			append condition "([::tabs::edge_condition $condition1 -])([::tabs::edge_condition $cond4 =])"

			foreach j2 $jlist {
			    if {$j2 == $j} break
			    set condition_j2 $technology_table(vdimensions:$j2:condition)
			    append condition "!($condition_j2)"
			}
			if {$j < 99} { append condition "($condition2)" }

			foreach k2 $klist {
			    if {$k2 == $k} break
			    set condition_k2 $technology_table(vdimensions:$k2:condition)
			    append condition "!($condition_k2)"
			}
			if {$k < 99} { append condition "($condition3)" }

			set condition [exec minimize $condition]
		    }

		    if {$condition != "0"} {
			set rule_name [get_unique_rule_name lcap_${mask1}_${mask4}_${mask2}_${mask3}]
			if [info exists ::main_options(--fake)] {
			    lappend lateral_capacitance_rules "#   ${rule_name} : $condition : -$mask1 =$mask4 : 0"
			} else {
			    lappend lateral_capacitance_rules "    ${rule_name} : $condition : -$mask1 =$mask4 :"
			    eval lappend lateral_capacitance_rules $sub_rules
			    lappend lateral_capacitance_rules ""
			}
		    }
		}
            }
            }
        }
    }
}

#===============================================================================
#  Invoke main.
#===============================================================================

std::top_eval {
    eval main $argv
}
