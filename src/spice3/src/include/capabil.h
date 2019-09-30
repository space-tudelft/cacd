/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
**********/

/*
 *  #define-s that are always on
 */

#define CAPZEROBYPASS
#define NEWCONV
/* #define CAPBYPASS	Internal use only */

/*
 *  #define-s to identify common capabilities
 */

#ifdef WANT_X11
#define HAS_X11
#define HAS_X_
#endif

#ifndef SIGNAL_TYPE
#define SIGNAL_TYPE void
#endif

