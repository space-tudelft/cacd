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

proc read_file {path} {
    set fp [open $path r]
    set block [read $fp]
    close $fp
    return $block
}

proc file_mutex_lock {path {timeout -1}} {

    set path "${path}.#lock"

    set slept 0
    set wait 100000

    while 1 {
        if ![file exists $path] {
            set fp [open $path w]
            close $fp
            return 1
        }

        exec usleep 100000
        set slept [expr $slept + double(100000)]

        if {$timeout > 0 && $slept > $timeout} {
            return 0
        }
    }
}

proc file_mutex_unlock {path} {
    set path "${path}.#lock"
    file delete -force -- $path
}

proc backup_file {path} {

    set path [std::normalize_path $path]

    set dirname [file dirname $path]
    set backup_tail [file tail $path]
    if ![string match [string index $backup_tail 0] "."] {
        set backup_tail ".$backup_tail"
    }

    set index 0
    while 1 {
        incr index
        set backup_path ${dirname}/${backup_tail}.backup[format %04d $index]
        if ![file exists $backup_path] break
    }

    puts stderr "message: Backing up $path ..."
    exec cp -a $path $backup_path
}

################################################################################
}
################################################################################

