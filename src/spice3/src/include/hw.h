/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *  Hardware specific machine limits
 */

#include <limits.h>

#ifdef HAS_FLOAT_H
#include <float.h>
#endif

#include "hw_ieee.h"

#ifndef HUGE_VAL
#define HUGE_VAL 1.8e+308
#endif

#ifndef HUGE
#define HUGE HUGE_VAL
#endif

