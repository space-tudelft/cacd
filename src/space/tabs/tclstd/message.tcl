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

proc message {msg {color blue}} {
    set msg [std::colorize $msg $color]
    puts stderr "message: $msg"
}

variable __verbosity
set __verbosity 0

proc verbose {msg {color blue}} {
    if [is_verbose] {
        set msg [std::colorize $msg $color]
        puts stderr "verbose: $msg"
    }
}

proc debug_message {msg {color green}} {
    if [is_debug] {
        set msg [std::colorize $msg $color]
        puts stderr "debug: $msg"
    }
}

proc is_verbose {} {
    if {$::std::__verbosity > 0} {return 1}
    return 0
}

proc enable_verbosity {} {
    incr ::std::__verbosity
}

proc lower_verbosity {} {
    incr ::std::__verbosity -1
}

proc error_message {msg {color red}} {
    set msg [std::colorize $msg $color]
    puts stderr "error: $msg"
}

proc warning_message {msg {color green}} {
    set msg [std::colorize $msg $color]
    puts stderr "warning: $msg"
}

variable __debug
set __debug -1

proc is_debug {} {
    if {$::std::__debug > 0} {return 1}
    return 0
}

proc enable_debug {} {
    set ::std::__debug 1
}

proc top_eval {body} {
    uplevel 1 [list std::try $body when tcl_exception {arg} {
        puts stderr "error: [std::colorize $arg red]"
        if [std::is_debug] {
            puts stderr $::errorInfo
        }
    } when any {arg} {
        puts stderr "error: [std::colorize $arg red]"
    }]
}

################################################################################
}
################################################################################

