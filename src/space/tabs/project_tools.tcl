#
# ISC License
# Copyright (C) 2004-2018 by
#	Kees-Jan van der Kolk
#	Simon de Graaf
#	Nick van der Meijs
# Delft University of Technology
#

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
namespace eval project_tools {
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#===============================================================================
#  Determine technology description file (.s file).
#===============================================================================

proc determine_sfile {user_project_path} {

    set process_path [determine_process $user_project_path]

    set sfile $process_path/space.def.s
    if ![file exists $sfile] {
        std::throw user_exception "Unable to locate technology description (space.def.s) in process directory `$process_path'."
    }

    return $sfile
}

#===============================================================================
#  Determine maskdata file.
#===============================================================================

proc determine_maskdata {user_project_path} {

    set process_path [determine_process $user_project_path]

    set maskdata_file $process_path/maskdata
    if ![file exists $maskdata_file] {
        std::throw user_exception "Unable to locate file `maskdata' in process directory `$process_path'."
    }

    return $maskdata_file
}

#===============================================================================
#  Determine process.
#===============================================================================

proc determine_process {user_project_path} {

    if ![file exists $user_project_path/.dmrc] {
        std::throw user_exception "Directory `$user_project_path' is not a project."
    }

    std::auto_chdir $user_project_path {
        set fp [open "./.dmrc" r]
        gets $fp line
        gets $fp process_path
        close $fp

        if [regexp {^[ \t]*[0-9]+[ \t]*$} $process_path] {
            set process_number [expr int($process_path)]

            set fp [open $::env(ICDPATH)/share/lib/process/processlist r]
            set process_name ""
            while {[gets $fp line] >= 0} {
                regsub -all {#.*} $line "" line
                regsub -all {^[ \t]+} $line "" line
                regsub -all {[ \t]+$} $line "" line
                regsub -all {[ \t]+} $line " " line
                set list [split $line " "]
                if {[llength $list] != 2} continue
                set n [lindex $list 0]
                if {$n == $process_number} {
                    set process_name [lindex $list 1]
                    break
                }
            }
            close $fp

            if {$process_name == ""} {
                std::throw user_exception "Process number `$process_number' is invalid."
            }

            set process_path $::env(ICDPATH)/share/lib/process/$process_name
            if ![file isdirectory $process_path] {
                std::throw user_exception "Could not find process directory `$process_path' for process number `$process_number'."
            }
        }

        set process_path [std::normalize_path $process_path]
    }

    return $process_path
}

#===============================================================================
#  Determine lambda.
#===============================================================================

proc determine_lambda {user_project_path} {

    if ![file exists $user_project_path/.dmrc] {
        std::throw user_exception "Directory `$user_project_path' is not a project."
    }

    std::auto_chdir $user_project_path {
        set fp [open "./.dmrc" r]
        gets $fp line
        gets $fp line
        gets $fp lambda
        close $fp

        set lambda [std::validate_double $lambda 1]
    }

    return $lambda
}

#===============================================================================
#  Reads the given technology file (and maskdata file) and stores the result
#  in the given table.
#
#  This procedure uses the `-T' option of tecc to extract the technology
#  information from a `.s' file.
#
#  Note that vdimensions and dielectrics are re-ordered according to increasing
#  z-coordinates.
#===============================================================================

proc read_technology {sfile maskdata_file _technology_table} {

    upvar $_technology_table technology_table

    std::verbose "Creating process directory to read technology file ..."

    eval {
        set initdir [std::tmppath "process"]

        file mkdir $initdir
        file copy $sfile $initdir/space.def1.s
        file copy $maskdata_file $initdir/maskdata1

        std::auto_chdir $initdir {
            std::quiet_execute tecc -T space.def.tcl -m maskdata1 space.def1.s
            source ./space.def.tcl

    # Sort dielectrics in the technology_table, according to increasing z-coordinate.
    #
    eval {
        array set ::project_tools::__sort_tmp [array get technology_table]

        proc __compare_dielectrics {n m} {
            if {$::project_tools::__sort_tmp(dielectrics:$n:z) < $::project_tools::__sort_tmp(dielectrics:$m:z)} {return -1}
            if {$::project_tools::__sort_tmp(dielectrics:$n:z) > $::project_tools::__sort_tmp(dielectrics:$m:z)} {return 1}
            return 0
        }

        set list [list]
        for {set i 0} {$i < $technology_table(dielectrics:size)} {incr i} {
            lappend list $i
        }
        set list [lsort -command __compare_dielectrics $list]

        for {set i 0} {$i < $technology_table(dielectrics:size)} {incr i} {
            set j [lindex $list $i]
            foreach key {name z permittivity} {
                set technology_table(dielectrics:$i:$key) $::project_tools::__sort_tmp(dielectrics:$j:$key)
            }
        }

        array unset ::project_tools::__sort_tmp
    }

    # Sort vdimensions in the technology table, according to increasing z-coordinate.
    #
    eval {
        array set ::project_tools::__sort_tmp [array get technology_table]

        proc __compare_vdimensions {n m} {
            if {$::project_tools::__sort_tmp(vdimensions:$n:z) < $::project_tools::__sort_tmp(vdimensions:$m:z)} {return -1}
            if {$::project_tools::__sort_tmp(vdimensions:$n:z) > $::project_tools::__sort_tmp(vdimensions:$m:z)} {return 1}
            return 0
        }

        set list [list]
        for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
            lappend list $i
        }
        set list [lsort -command __compare_vdimensions $list]

        for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
            set j [lindex $list $i]
            foreach key {name condition mask z thickness} {
                set technology_table(vdimensions:$i:$key) $::project_tools::__sort_tmp(vdimensions:$j:$key)
            }
        }

        array unset ::project_tools::__sort_tmp
    }

	    set fp [open maskdata w]
	    puts $fp "\"<None>\" \"<None>\""
	    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
		puts $fp "mask$i 1 0 0 0 0 0 0 0 0 \"mask$i\""
	    }
	    close $fp

	    set fp [open space.def.s w]
	    puts $fp "\nunit vdimension 1e-6 # micron"
	    puts $fp "\ncolors:"
	    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
		puts $fp "    mask$i white"
	    }
	    puts $fp "\nconductors:"
	    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
		puts $fp "    cond_$i : mask$i : mask$i : 1"
	    }
	    puts $fp "\nvdimensions:"
	    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
		puts $fp "    vdim_$i : mask$i : mask$i : [format %7g [expr $technology_table(vdimensions:$i:z) * 1e6]]\t[format %g [expr $technology_table(vdimensions:$i:thickness) * 1e6]]"
	    }
	    puts $fp "\ndielectrics:"
	    for {set i 0} {$i < $technology_table(dielectrics:size)} {incr i} {
		puts $fp "    [format %-10s $technology_table(dielectrics:$i:name)] $technology_table(dielectrics:$i:permittivity)\t[format %g [expr $technology_table(dielectrics:$i:z) * 1e6]]"
	    }
	    puts $fp "\n#"
	    close $fp

	    if {$technology_table(dielectrics:size) > 3} {
                std::throw user_exception "Too many dielectrics (> 3), no unigreen support."
	    } else {
		std::quiet_execute tecc -m maskdata space.def.s
	    }

	    # Create the space.def.p (space default parameter) file.
	    eval {
		set fp [open space.def.p w]
		puts $fp "compression           off"
		puts $fp "min_art_degree          3"
		puts $fp "min_degree              4"
		puts $fp "min_res               100      # ohm"
		puts $fp "max_par_res            20"
		puts $fp "no_neg_res             on"
		## puts $fp "min_coup_cap            0.05"
		puts $fp "lat_cap_window          6.0    # micron"
		puts $fp "max_obtuse            110.0    # degrees"
		puts $fp "equi_line_ratio         1.0"
		puts $fp ""
		puts $fp "BEGIN cap3d  # Data for 3D capacitance extraction"
		puts $fp "be_mode                 0c"
		puts $fp "be_window               2.0"
		puts $fp "max_be_area             1.0"
		puts $fp "omit_gate_ds_cap        on"
		## puts $fp "mp_min_dist           2"
		## puts $fp "green_eps          .001"
		puts $fp "END cap3d"
		puts $fp ""
		puts $fp "BEGIN disp  # Data for Xspace"
		puts $fp "be_mesh_only"
		puts $fp "draw_be_mesh"
		puts $fp "save_prepass_image"
		puts $fp "cam_longitude 300"
		puts $fp "END disp"
		close $fp
	    }
        }

        set technology_table(tecc_path) $initdir/space.def.t
    }

    # Save the order of the masks specified in the vdimensions section.
    #
    set technology_table(mask_order) [list]
    array unset table
    for {set i 0} {$i < $technology_table(vdimensions:size)} {incr i} {
        set mask $technology_table(vdimensions:$i:mask)
        if ![info exists table($mask)] {
            lappend technology_table(mask_order) $mask
            set table($mask) 1
        }
    }

}

#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}
#+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

