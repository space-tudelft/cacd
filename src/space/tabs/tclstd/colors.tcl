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

variable __color_enabled
set __color_enabled 1

if [info exists ::env(CACD_DISABLE_COLOR)] {
    set __color_enabled 0
}

proc disable_color {} {
	variable __color_enabled
	set __color_enabled 0

        # Also make sure that child processes don't use color.
        set ::env(CACD_DISABLE_COLOR) 1
}

proc colorize {string color} {

	variable __color_enabled
	if !$__color_enabled {return $string}

	set c 0
	switch -exact -- $color {
	black {set c 50}
	gray {set c 30}
	red {set c 31}
	green {set c 32}
	yellow {set c 33}
	blue {set c 34}
        cyan {set c 36}
	white {set c 37}
	}

	set result [format "%c\[01;${c}m" 27]
	append result $string
	append result [format "%c\[00m" 27]
	return $result
}

################################################################################
}
################################################################################

