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

proc protect_string {string} {
    return [split $string "\n"]
}

proc unprotect_string {string} {
    return [join $string "\n"]
}

proc array_to_string {_table} {
    upvar $_table table
    set list [list]
    foreach key [array names table] {
        lappend list [list $key $table($key)]
    }
    return [std::protect_string $list]
}

proc array_from_string {string _table} {
    set string [std::unprotect_string $string]
    upvar $_table table
    foreach pair $string {
        set table([lindex $pair 0]) [lindex $pair 1]
    }
}

# The "array get" command, but canonic. See the array manpage.
#
proc canonic_array_get {_table} {
    upvar $_table table
    set list [list]
    foreach key [lsort [array names table]] {
        lappend list $key $table($key)
    }
    return $list
}

################################################################################
}
################################################################################

