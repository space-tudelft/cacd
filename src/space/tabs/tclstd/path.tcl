#
# ISC License
# Copyright (C) 2004-2018 by
#	Kees-Jan van der Kolk
#	Simon de Graaf
#	Nick van der Meijs
# Delft University of Technology
#

################################################################################
namespace eval std {
################################################################################

proc is_absolute_path {path} {

    if {[string index $path 0] == "/"} {
        return 1
    }
    return 0
}

proc normalize_path {path} {

    global tcl_platform

    if {[string index $path 0] != "/"} {
        set path [pwd]/$path
    }

    set list [split $path /]

    set newlist ""
    foreach elem $list {
        if {$elem == "."} continue
        if {$elem == ".."} {
            if {$newlist != ""} {
                set newlist [lrange $newlist 0 [expr [llength $newlist]-2]]
                continue
            }
        } elseif {$elem == ""} {
            if {$newlist != ""} {
                continue
            }
        }
        lappend newlist $elem
    }

    set new_path [join $newlist "/"]
    if {$new_path == ""} {set new_path "/"}
    return ${new_path}
}

proc make_relative_path {path {curpath "."}} {

    set path    [normalize_path $path]
    set curpath [normalize_path $curpath]

    set list    [split $path /]
    set curlist [split $curpath /]

    set remainder  ""
    set difference 0

    for {set i 0} {1} {incr i} {

        set elem    [lindex $list $i]
        set curelem [lindex $curlist $i]

        if 0 {
            puts stderr "elem:    $elem"
            puts stderr "curelem: $curelem"
            puts stderr "remain:  $remainder"
        }

        if {$i >= [llength $list] && $i >= [llength $curlist]} {
            break
        }

        if {$difference} {
            if {$curelem != ""} {
                set remainder [concat [list ..] $remainder]
            }
            if {$elem != ""} {
                lappend remainder $elem
            }
            continue
        }

        if {$i >= [llength $curlist]} {
            lappend remainder $elem
            continue
        }

        if {$i >= [llength $list]} {
            set remainder [concat [list ..] $remainder]
            continue
        }

        if {$elem != $curelem} {
            set remainder [concat [list ..] $remainder $elem]
            set difference 1
        }
    }

    if {$remainder == ""} {
        set remainder [list .]
    }

    return [join $remainder "/"]
}

proc straighten_path {path} {

    set list [split $path /]
    set newlist [list]
    foreach elem $list {
        if {$elem == "."} continue
        if {$elem == ""} continue
        if {$elem == ".."} {
            if {[llength $newlist] != 0} {
                set newlist [lrange $newlist 0 [expr [llength $newlist]-2]]
                continue
            }
        }
        lappend newlist $elem
    }

    set result [join $newlist "/"]
    if {[lindex $path 0] == "/"} {set result "/$result"}

    return $result
}

#===============================================================================
#  Temporary files and directories.
#===============================================================================

variable global_tmpdir
set global_tmpdir ""

variable tmppath_id
set tmppath_id 0

proc tmppath {{tail ""}} {

    variable global_tmpdir
    if ![string length $global_tmpdir] {
        set global_tmpdir [tmpdir]
    }

    variable tmppath_id
    if {$tail == ""} {
        set tail "tmp"
    }

    while 1 {
        set path $global_tmpdir/$tail-[incr tmppath_id]
        if ![file exists $path] {
            return $path
        }
    }
}

proc _get_tmpbase {} {

    global tcl_platform
    set found 0
    if [info exists ::env(ICD_TMPDIR)] {
        set path $::env(ICD_TMPDIR)
        if [file exists $path] {return $path}
    }
    if [info exists ::env(TMP)] {
        set path $::env(TMP)
        if [file exists $path] {return $path}
    }
    if [info exists ::env(TEMP)] {
        set path $::env(TEMP)
        if [file exists $path] {return $path}
    }

    set path "/tmp"
    if ![file exists $path] {
	puts stderr "$path: directory not found"
	puts stderr "please set environment ICD_TMPDIR"
	exit 1
    }

    return $path
}

variable cleanup_necessary
set cleanup_necessary 1

proc tmpdir {} {

    variable tmppath_id
    variable cleanup_necessary

    if {$cleanup_necessary} {
        cleanup_tmpdirs
        set cleanup_necessary 0
    }

    set path [_get_tmpbase]

    while 1 {
        set path2 $path/cacdtmp-[pid]-[incr tmppath_id]
        if ![file exists $path2] {
            set path $path2
            break
        }
    }

    file mkdir $path

    set fp [open $path/.tmp_pid w]
    puts $fp [pid]
    close $fp

    return $path
}

proc cleanup_tmpdirs {} {

    set tmpbase [_get_tmpbase]
    set cwd [pwd]
    cd $tmpbase
    foreach dir [glob -nocomplain cacdtmp-*] {
        if ![file readable $dir/.tmp_pid] continue
        set fp [open $dir/.tmp_pid r]
        gets $fp pid
        close $fp

        if ![::std::process_still_running $pid] {
            catch {file delete -force -- $dir}
        }
    }
    cd $cwd
}

#===============================================================================
#  auto_chdir: perform a script in a working directory, and automatically
#  change back to the previous working directory (even if an exception occurs
#  in the script, or if the script returns).
#===============================================================================

proc auto_chdir {path script} {

    set cwd [pwd]
    cd $path

    std::try {
        uplevel 1 $script
    } always {
        cd $cwd
    }
}

################################################################################
}
################################################################################

