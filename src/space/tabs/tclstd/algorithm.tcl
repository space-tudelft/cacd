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

# Swap contents of two variables.
#
proc swap {_a _b} {
    upvar $_a a
    upvar $_b b

    set t $a
    set a $b
    set b $t
}

################################################################################
}
################################################################################

