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

proc mulstr {string times} {
    set result ""
    while {$times > 0} {
        append result $string
        incr times -1
    }
    return $result
}

#===============================================================================
#  comma_number -- Put comma's in a number.
#
#  Example input: 4387234
#	output: 4,387,234
#===============================================================================

proc comma_number {number} {
    while 1 {
        set pos [string first "," $number]
        if {$pos < 0} {set pos [string length $number]}
        if {$pos < 4} break
        set head [string range $number 0 [expr $pos-4]]
        set tail [string range $number [expr $pos-3] end]
        set number "${head},${tail}"
    }
    return $number
}

#===============================================================================
#  format_complex -- Format a complex number.
#
#  Example input: {1 2}
#	output: 1 + 2*I
#===============================================================================

proc format_complex {c} {
    set x [lindex $c 0]
    set y [lindex $c 1]
    if ![string length $y] {set y 0}

    catch {
        if {abs($y/$x) < 1e-15} {
            set y 0
        }
    }

    catch {
        if {abs($x/$y) < 1e-15} {
            set x 0
        }
    }

    if {$y == 0} {
        return [format %.5g $x]
    } elseif {$x == 0} {
        return "0 + [format %.5g $y] * I"
    } else {
        return "[format %.5g $x] + [format %.5g $y] * I"
    }
}

################################################################################
}
################################################################################

