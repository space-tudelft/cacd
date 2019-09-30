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

#  This file defines the procedures "std::throw" and "std::try", which can be
#  used for exception handling.
#
#  Example of usage:
#
# 	std::try {
#		puts stderr "HELLO"
#		std::throw io_exception "READ ERROR"
#	} when io_exception {arg} {
#		puts stderr "got io_exception: $arg"
#	} when any {arg} {
#		puts stderr "got other exception: $arg"
#	} else {
#		puts stderr "got no exception"
#	}

#===============================================================================
#  proc throw -- Throw an exception.
#===============================================================================

proc throw {exception {argument ""}} {
	return -errorcode [list throw $exception $argument] -code error "Exception of type `$exception' not caught."
}

#===============================================================================
#  proc try -- Try to execute a block and catch exceptions.
#===============================================================================

proc try {args} {

	set body [lindex $args 0]

	set status1 [catch {uplevel 1 $body} msg1]
	if {$status1 == 1} {

                set error_code $::errorCode
                set error_info $::errorInfo

		if [regexp "^throw " $::errorCode] {
			set exception [lindex $error_code 1]
			set argument [lindex $error_code 2]
		} else {
			set exception "tcl_exception"
			set argument "$msg1"
		}

	} else {
                set error_code ""
                set error_info ""

		set exception ""
		set argument ""
	}

	set caught 0
	for {set i 1} {$i < [llength $args]} {} {

            set keyword [lindex $args $i]

            if {$keyword == "when"} {

                if {$i+3 > [llength $args]} {
                        return -code error "invalid number of arguments for `when'"
                }

                if !$caught {
                    set matchcode [lindex $args [expr $i + 1]]

                    if {$matchcode == $exception || ($matchcode == "any" && $exception != "") || ($matchcode == "none" && $exception == "")} {

                        set varname [lindex $args [expr $i + 2]]
                        set body    [lindex $args [expr $i + 3]]

                        upvar 1 $varname variable
                        set variable $argument

                        set status [catch {uplevel 1 $body} msg2]
                        switch -exact -- $status {
                            0 { set caught 1 }
                            1 { return -code error -errorcode $::errorCode -errorinfo $::errorInfo }
                            2 { return -code return $msg2 }
                            3 { return -code break }
                            4 { return -code continue }
                        }
                    }
                }
                incr i 4

            } elseif {$keyword == "else"} {

                if ![info exists ::std::__try_else_deprecated] {
                    puts stderr "INTERNAL: `else' keyword is deprecated; use `when none' instead. See file `exception.tcl'."
                    set ::std::__try_else_deprecated 1
                }

                if {$i+1 > [llength $args]} {
                        return -code error "invalid number of arguments for `else'"
                }

                if !$caught {
                    if {$exception == ""} {
                        set body [lindex $args [expr $i + 1]]

                        set status [catch {uplevel 1 $body} msg2]
                        switch -exact -- $status {
                            0 { set caught 1 }
                            1 { return -code error -errorcode $::errorCode -errorinfo $::errorInfo }
                            2 { return -code return $msg2 }
                            3 { return -code break }
                            4 { return -code continue }
                        }
                    }
                }
                incr i 2

            } elseif {$keyword == "always"} {

                if {$i+1 > [llength $args]} {
                        return -code error "invalid number of arguments for `always'"
                }

                eval {
                    set body [lindex $args [expr $i + 1]]

                    set status [catch {uplevel 1 $body} msg2]
                    switch -exact -- $status {
                        0 { }
                        1 { return -code error -errorcode $::errorCode -errorinfo $::errorInfo }
                        2 { return -code return $msg2 }
                        3 { return -code break }
                        4 { return -code continue }
                    }
                }
                incr i 2

            } else {
                return -code error "Invalid keyword after body; should be: when, else or always."
            }
	}

	if {$exception != "" && !$caught} {
            return -errorcode $error_code -code error -errorinfo $error_info "$msg1"
	} elseif {$status1 == 2} {
            return -code return $msg1
        }
}

################################################################################
}
################################################################################

