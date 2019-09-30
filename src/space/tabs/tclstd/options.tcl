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

proc options_parse {format_list arguments _table {behavior ""}} {

	upvar $_table table

        set auto_separator 0
        foreach s $behavior {
            if {$s == "auto_separator"} {
                set auto_separator 1
            }
        }

	catch {unset alias_table}
	catch {unset format_table}
	foreach format $format_list {
		set option [lindex $format 0]
		set option_count [lindex $format 1]
                if {[string compare $option_count "*"] == 0} {
                    set option_count -1
                }
		if {[lindex $option_count 0] == "alias"} {
			set alias_table($option) [lindex $option_count 1]
                        continue
		}
		set format_table($option) $option_count
	}
	set format_table(--) -1

	foreach alias [array names alias_table] {
		set option $alias_table($alias)
		set format_table($alias) $format_table($option)
	}

	set current_option ""
	set got_separator 0
	foreach arg $arguments {
		if {!$got_separator && [string index $arg 0] == "-"} {
			if ![info exists format_table($arg)] {
				std::throw user_exception "error: unknown option $arg"
			}
			if {$arg == "--"} {set got_separator 1}
			set option_count [lindex $format_table($arg) 0]
			set table($arg) [list]
			if {$option_count == 0} {
				set current_option ""
			} else {
				set current_option $arg
			}
			continue
		}
		if {$current_option == ""} {
			set current_option "--"
			set table($current_option) [list]
			lappend table($current_option) $arg

                        if $auto_separator {
                            set got_separator 1
                        }

			continue
		}
		lappend table($current_option) $arg
		set n [llength $table($current_option)]
		set option_count [lindex $format_table($current_option) 0]
		if {$option_count >= 0 && $n >= $option_count} {
			set current_option ""
		}
	}

	if ![info exists table(--)] {
		set table(--) [list]
	}

	foreach option [array names table] {
		# puts stderr "debug: $option: $table($option)"
		set option_count [lindex $format_table($option) 0]
		set n [llength $table($option)]
		if {$option_count >= 0 && $option_count != $n} {
			std::throw user_exception "error: invalid # args for option $option"
		}
	}

	foreach alias [array names alias_table] {
		set option $alias_table($alias)
		if [info exists table($alias)] {
			set table($option) $table($alias)
		}
	}
}

################################################################################
}
################################################################################

