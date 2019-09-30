/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1986 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * The main entry point for the help system.
 */

#include "spice.h"
#include "cpstd.h"
#include "hlpdefs.h"

char *hlp_directory;
extern char *hlp_filelist[];
int hlp_ftablesize = 0;

void hlp_main (char *path, wordlist *wl)
{
    topic *top;
    fplace *place;

    hlp_directory = path;

    if (wl) {
        while (wl) {
            if (!(place = findglobalsubject(wl->wl_word))) {
                fprintf(stderr, "Error: No such topic: %s\n", wl->wl_word);
                wl = wl->wl_next;
                continue;
            }
            if (!(top = hlp_read(place))) {
                fprintf(stderr, "Error: can't read topic\n");
                wl = wl->wl_next;
                continue;
            }
            hlp_provide(top);
            wl = wl->wl_next;
        }
    } else {
        if (!(place = findglobalsubject("main"))) {
          fprintf(stderr, "Error: no main topic\n");
          return;
        }
        if (!(top = hlp_read(place))) {
            fprintf(stderr, "Error: can't read topic\n");
            return;
        }
        hlp_provide(top);
    }

#ifndef HAS_X11
    hlp_free();
#endif
}

fplace * findglobalsubject (char *subject)
{
    fplace *place;
    char **dict;
    long fpos;

    place = NULL;
    for (dict = hlp_filelist; *dict && **dict; dict++) {
	fpos = findsubject(*dict, subject);
	if (fpos != -1) {
	    place = (fplace *) tmalloc(sizeof(fplace));
	    place->fpos = fpos;
	    place->filename = copy(*dict);
	    place->fp = hlp_fopen(*dict);
	    break;
	}
    }
    return(place);
}

/* see if file is on filelist */

bool hlp_approvedfile (char *filename)
{
    char **s;

    for (s = hlp_filelist; *s && **s; s++) {
      if (cieq(*s, filename)) return(true);
    }
    return(false);
}

/* keep file pointers on top level files so we don't always have to do
    fopen's */
FILE *hlp_fopen (char *filename)
{
    static struct {
        char filename[BSIZE_SP];
        FILE *fp;
    } hlp_ftable[32];
    int i;
    char buf[BSIZE_SP];

    for (i=0; i < hlp_ftablesize; i++) {
      if (cieq(filename, hlp_ftable[i].filename)) {
        return(hlp_ftable[i].fp);
      }
    }

    /* not already in table */
    sprintf(buf, "%s/%s.txt", hlp_directory, filename);
    if (!(hlp_ftable[hlp_ftablesize].fp = fopen(buf, "r"))) {
      perror(buf);
      return (NULL);
    }

    strcpy(hlp_ftable[hlp_ftablesize].filename, filename);
    hlp_ftablesize++;

    return(hlp_ftable[hlp_ftablesize - 1].fp);
}
