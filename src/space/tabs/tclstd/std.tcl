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

#===============================================================================
#  Tcl settings.
#===============================================================================

# Full IEEE double precision; no loss of information.
#
set ::tcl_precision 17

# Make sure that random numbers come out the same each time.
expr srand(1)

#===============================================================================
#  Make sure that we have the proper version of Tcl.
#===============================================================================

eval {
    global tcl_version
    set major [lindex [split $tcl_version "."] 0]
    set minor [lindex [split $tcl_version "."] 1]

    if {$major < 8 || ($major == 8 && $minor < 3)} {
        puts stderr "fatal: version of tclsh should be at least 8.3"
        puts stderr "fatal: (current version of [info nameofexecutable] is $tcl_version)"
        exit 1
    }
}

#===============================================================================
#  Make sure that the `cd' command also sets env(CWD).
#===============================================================================

rename ::cd ::__std_cd

proc ::cd {args} {
    set result [eval ::__std_cd $args]
    global env
    set ::env(CWD) [pwd]
    return $result
}

#===============================================================================
#  Miscellaneous procedures.
#===============================================================================

proc not_implemented {msg} {
    error "error fatal: not implemented: $msg"
}

proc invariant {expr} {
    set cmd "global __std_invariant; set __std_invariant \[expr [list $expr]\]"
    uplevel 1 $cmd
    global __std_invariant
    if !$__std_invariant {
        unset __std_invariant
        error "error: invariant \"$expr\" failed"
    }
    unset __std_invariant
}

################################################################################
}
################################################################################

