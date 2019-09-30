/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to handle "more"d output.  There are some serious system
 * dependencies in here, and it isn't clear that versions of this stuff
 * can be written for every possible machine...
 */

#include "spice.h"
#include "cpdefs.h"

//#ifdef HAS_BSDTTY
#include <sgtty.h>
//#endif

bool out_moremode = true;
bool out_isatty = true;

static int xsize, ysize, xpos, ypos;
static bool noprint, nopause;

/* out_printf doesn't handle double arguments correctly, so we
    sprintf into this buf and call out_send with it */
char out_pbuf[BSIZE_SP];

/* Start output... */

void out_init()
{
    noprint = false;
    nopause = true;

    if (out_isatty && out_moremode && cp_interactive) {
	bool m;
	if (cp_getvar("nomoremode", VT_BOOL, (char *) &m)) return;
	nopause = false;
    }
    else return;

    xsize = ysize = 0;
    (void) cp_getvar("width",  VT_NUM, (char *) &xsize);
    (void) cp_getvar("height", VT_NUM, (char *) &ysize);

#ifdef TIOCGWINSZ
    /* Try to figure out the screen size.
     */
    if (!xsize || !ysize) {
	struct winsize ws;
        (void) ioctl(fileno(stdout), TIOCGWINSZ, (char *) &ws);
	if (!xsize) xsize = ws.ws_col;
	if (!ysize) ysize = ws.ws_row;
    }
#endif

    if (!xsize) xsize = 80; /* default */
    if (!ysize) ysize = 24; /* default */

    ysize -= 2; /* Fudge room... */
    xpos = ypos = 0;
}

/* Putc may not be buffered (sp?), so we do it ourselves. */

static char staticbuf[BUFSIZ];
struct {
    int count;
    char *ptr;
} ourbuf = { BUFSIZ, staticbuf };

static void outbufputc() /* send buffer out */
{
    if (ourbuf.count != BUFSIZ) {
	fputs(staticbuf, cp_out);
	bzero(staticbuf, BUFSIZ-ourbuf.count);
	ourbuf.count = BUFSIZ;
	ourbuf.ptr = staticbuf;
    }
}

#define bufputc(c) \
if (--ourbuf.count >= 0) *ourbuf.ptr++ = (unsigned)c; \
else fbufputc((unsigned)c)

static void fbufputc (unsigned char c)
{
    ourbuf.count = 0;
    outbufputc();
    ourbuf.count = BUFSIZ;
    ourbuf.ptr = staticbuf;
  --ourbuf.count;
   *ourbuf.ptr++ = c;
}

static void promptreturn() /* prompt for a return */
{
    char buf[16];
ask:
    fputs("\n\t-- hit Enter for more, ? for help -- ", cp_out);
    fflush(cp_out);
    if (!fgets(buf, 16, cp_in)) {
        clearerr(cp_in);
        *buf = 'q';
    }
    switch (*buf) {
    case ' ':
    case '\n': break;
    case 'q': noprint = true; break;
    case 'c': nopause = true; break;
    case '?':
	fputs("\nPossible responses:\n", cp_out);
	fputs("\t<Enter>  : more, print another screenful\n", cp_out);
	fputs("\tq<Enter> : discard the rest of the output\n", cp_out);
	fputs("\tc<Enter> : continuously print the rest\n", cp_out);
	fputs("\t?<Enter> : print this help message\n", cp_out);
	goto ask;
    default:
	fputs("No good response\n", cp_out);
	goto ask;
    }
}

/* Print a string to the output.  If this would cause the screen to scroll,
 * print "more".
 */
void out_send (char *string)
{
    if (noprint) return;
    if (nopause || !out_isatty) {
        fputs(string, cp_out);
        return;
    }
    while (*string) {
        switch (*string) {
            case '\n':
                xpos = 0;
                ypos++;
                break;
            case '\f':
                ypos = ysize;
                xpos = 0;
                break;
            case '\t':
                xpos = xpos / 8 + 1;
                xpos *= 8;
                break;
            default:
                xpos++;
                break;
        }
        while (xpos >= xsize) {
            xpos -= xsize;
            ypos++;
        }
        if (ypos >= ysize) {
            outbufputc();       /* out goes buffer */
            promptreturn();
            ypos = xpos = 0;
        }
        bufputc(*string);   /* we need to buffer these */
        string++;
    }
    outbufputc();
}

/* Print some stuff using more mode. */

void out_printf(char *fmt, char *s1, char *s2, char *s3)
{
    char buf[BSIZE_SP+BSIZE_SP];
    sprintf(buf, fmt, s1, s2, s3);
    out_send(buf);
}
