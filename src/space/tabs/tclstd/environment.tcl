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

proc get_environment_flag {flag} {
    global env
    if ![info exists env($flag)] {
        return 0
    }
    set string $env($flag)
    if [catch {set value [expr int(${string})]}] {
        return 0
    }
    return [expr $value != 0]
}

proc save_environment {_table} {
    upvar $_table table
    global env
    catch {unset table}
    foreach key [array names env] {
        set table($key) $env($key)
    }
}

proc restore_environment {_table} {
    upvar $_table table
    global env
    foreach key [array names table] {
        set env($key) $table($key)
    }
    foreach key [array names env] {
        if ![info exists table($key)] {
            unset env($key)
        }
    }
}

################################################################################
}
################################################################################

