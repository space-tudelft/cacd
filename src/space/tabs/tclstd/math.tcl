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

# Round to nearest integer.
#
proc rint {x} {
    if {[expr $x-floor($x)] < 0.5} {
        return [expr floor($x)]
    } else {
        return [expr ceil($x)]
    }
}

proc max {args} {
    std::invariant {[llength $args] != 0}
    set v [lindex $args 0]
    foreach elem $args {
        if {$elem > $v} {set v $elem}
    }
    return $v
}

proc min {args} {
    std::invariant {[llength $args] != 0}
    set v [lindex $args 0]
    foreach elem $args {
        if {$elem < $v} {set v $elem}
    }
    return $v
}

#===============================================================================
#  Functions for complex arithmetic.
#===============================================================================

proc Plus {args} {
    set real 0.0
    set imag 0.0
    foreach elem $args {
        set r [lindex $elem 0]
        set i [lindex $elem 1]
        set real [expr $real + $r]
        set imag [expr $imag + $i]
    }
    return [list $real $imag]
}

proc Times {args} {
    set real 1.0
    set imag 0.0
    foreach elem $args {
        set r [lindex $elem 0]
        set i [lindex $elem 1]
        set real2 [expr $real*$r - $imag*$i]
        set imag2 [expr $real*$i + $imag*$r]
        set real $real2
        set imag $imag2
    }
    return [list $real $imag]
}

# x^y = e^(y * ln(x))
#     = e^(y * (ln(abs(x)) + i*(2*n*pi + arg(x)))) {n integer}
#
proc Power {x y} {
    set a [lindex $x 0]
    set b [lindex $x 1]
    set c [lindex $y 0]
    set d [lindex $y 1]

    set m [expr sqrt($a*$a+$b*$b)]
    set arg [expr atan2($b, $a)]

    set lnm [expr log($m)]

    set r [expr $lnm*$c - $arg*$d]
    set i [expr $arg*$c + $lnm*$d]

    set real [expr exp($r)*cos($i)]
    set imag [expr exp($r)*sin($i)]

    return [list $real $imag]
}

proc Modulus {x} {
    set a [lindex $x 0]
    set b [lindex $x 1]
    return [expr sqrt($a*$a+$b*$b)]
}

################################################################################
}
################################################################################

