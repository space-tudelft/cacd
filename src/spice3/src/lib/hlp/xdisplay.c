/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

#include "spice.h"
#include "cpstd.h"
#include "hlpdefs.h"

char *hlp_boldfontname = BOLD_FONT;
char *hlp_regfontname = REG_FONT;
char *hlp_italicfontname = ITALIC_FONT;
char *hlp_titlefontname = TITLE_FONT;
char *hlp_buttonfontname = BUTTON_FONT;
char *hlp_displayname = NULL;
int hlp_initxpos = START_XPOS;
int hlp_initypos = START_YPOS;
int hlp_buttonstyle = BS_LEFT;

#ifndef HAS_X11
bool hlp_xdisplay(topic *top) { return (false); }
void hlp_xkillwin(topic *top) { }
#endif

void hlp_xwait(topic *top, bool on) { }
void hlp_xclosedisplay() {}
toplink * hlp_xhandle(topic **pp) { *pp = NULL; return (NULL); }

