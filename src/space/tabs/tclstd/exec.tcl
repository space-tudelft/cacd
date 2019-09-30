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

proc exec_fetch {cmd args} {

    if [std::is_verbose] {
        puts stderr "+ $cmd $args"
    }

    if [catch {
        set fp [open "|$cmd $args" "r"]
        set result [read $fp]
        close $fp
    } msg] {
        if {[lindex $::errorCode 0] == "CHILDSTATUS"} {
            set status [lindex $::errorCode 2]
            std::throw child_exception "error: Exit code $status."
        }
        error $msg $::errorInfo $::errorCode
    }
    return $result
}

proc execute {cmd args} {

    if [std::is_verbose] {
        puts stderr "+ $cmd $args"
    }

    if ![regexp ">" $args] {
        lappend args >@ stderr 2>@ stderr
    } elseif ![regexp "2>" $args] {
        lappend args 2>@ stderr
    }
    if [catch {eval ::exec [list $cmd] $args} msg] {
        if {[lindex $::errorCode 0] == "CHILDSTATUS"} {
            set status [lindex $::errorCode 2]
            std::throw child_exception "error: Exit code $status."
        } else {
            error $msg $::errorInfo $::errorCode
        }
    }
}

proc quiet_execute {cmd args} {

    if [std::is_verbose] {
        return [eval std::execute [list $cmd] $args]
    }

    if [catch {eval ::exec [list $cmd] $args} msg] {
        if {[lindex $::errorCode 0] == "CHILDSTATUS"} {
            puts stderr "+ $cmd $args"
            puts stderr $msg
            set status [lindex $::errorCode 2]
            std::throw child_exception "error: Exit code $status."
        }
    }
}


#===============================================================================
#
#  flatten_shell_command
#
#  Print a shell command (a list) so that it can be understood by /bin/sh (for
#  pretty printing, and so that the user can cut a command from the output,
#  and paste it into his/her shell.)
#
#===============================================================================

proc flatten_shell_command {cmd args} {
    set string ""
    foreach elem [concat [list $cmd] $args] {
        if {$string != ""} {append string " "}
        if [regexp " " $elem] {
            set elem "\"$elem\""
        }
        append string $elem
    }
    return $string
}

#===============================================================================
#  process_still_running
#===============================================================================

proc process_still_running {pid} {

    set path /proc/$pid/stat
    if [file exists $path] {
        set code [catch {
            set fp [open $path r]
            set line [split [read $fp] " "]
            close $fp
            if {[lindex $line 2] == "Z"} {
                return 0
            }
            return 1
        } retvalue]

        # Check for return (code=2, see manpage of catch(n)).
        if {$code == 2} {return $retvalue}
    }

    if [catch {exec ps -h $pid}] {
        return 0
    }

    return 1
}

################################################################################
}
################################################################################

