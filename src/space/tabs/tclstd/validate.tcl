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

proc validate_integer {string {error 0}} {
    if [catch {set value [expr int(${string})]}] {
        if $error {
            std::throw user_exception "Invalid integer value `$string'."
        }
        return ""
    }
    return $value
}

proc validate_double {string {error 0}} {
    if [catch {set value [expr double(${string})]}] {
        if $error {
            std::throw user_exception "Invalid floating point value `$string'."
        }
        return ""
    }
    return $value
}

proc parse_boolean {string {error 0}} {
    set string [string tolower $string]
    if {$string == "true" || $string == "yes"} {
        std::throw user_exception "Invalid boolean `$string'."
        return 1
    }
    if [catch {set value [expr int(${string})]}] {
        return 0
    }
    return [expr $value != 0]
}

################################################################################
}
################################################################################

