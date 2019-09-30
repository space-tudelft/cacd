/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Paul Stravers
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 * Delft University of Technology
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
/*
 * Functions for accessing the image.seadif file (= image description file)
 */

#include <sys/stat.h>
#include <stdlib.h>
#include "src/ocean/libseadif/sealib.h"

#define IMAGEFN "image.seadif"
#define OCEANPROCESS_DFLT "fishbone"
#define MAXPATHLEN 512

/* This function tries to find the top of the OCEAN tree as follows:
 * First check the environment ICDPATH. If that fails return NULL.
 */
STRING sdfoceanpath ()
{
   static STRING oceanpath = NULL;
   char *p;

   /* did we compute this thing before? */
   if (oceanpath) return oceanpath;

   /* check the environment variable: */
   if ((p = getenv ("ICDPATH"))) return oceanpath = cs (p);

   fprintf (stderr, "\nCannot find top of OCEAN tree, please set the ICDPATH...\n");
   return oceanpath;
}

/* return the name of the image description file name */
STRING sdfimagefn ()
{
   static STRING imagefn = NULL;
   struct stat stat_buf;
   STRING oceanhome;
   char  *oceanprocess, path[MAXPATHLEN+1];

   /* did we compute this thing before? */
   if (imagefn) return imagefn;

   /* is there a local copy of "image.seadif"? */
   if (stat (IMAGEFN, &stat_buf) == 0) return imagefn = cs (IMAGEFN);

   if (!(oceanhome = sdfoceanpath ())) return NULL; /* cannot find an image.seadif */
   if (!(oceanprocess = getenv ("OCEANPROCESS")))
      oceanprocess = cs (OCEANPROCESS_DFLT); /* assume a default process */

   sprintf (path, "%s/share/lib/celllibs/%s/image.seadif", oceanhome, oceanprocess);
   if (stat (path, &stat_buf) == 0) return imagefn = cs (path);
   fprintf (stderr, "\nCannot access image description file %s\n", path);
   return imagefn;
}

/* return a FILE pointer to the image description file */
FILE *sdfimagefp ()
{
   FILE  *fp;
   STRING fn;

   if (!(fn = sdfimagefn ())) return NULL;
   if (!(fp = fopen (fn, "r"))) {
      fprintf (stderr, "\nCannot open image description file %s\n", fn);
   }
   return fp;
}
