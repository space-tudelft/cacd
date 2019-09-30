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
 * This defines a function tryNelsisSealib() that builds a SEALIB environment
 * containing all directories in the projlist (this is a recursive thing,
 * because these imported projects can have their own projlist, etc.).
 * The function tryNelsisSealib() assumes that the seadif files are in
 * a subdirectory "seadif", below each project directory.
 */

#include "src/ocean/libseadif/systypes.h"
#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/libseadif/sea_decl.h"
#include "src/ocean/libseadif/namelist.h"
#include <string.h>             /* strcpy() etc. */
#include <ctype.h>              /* isspace() */
#include <unistd.h>             /* sometimes needed for sys/stat.h */
#include <sys/stat.h>           /* stat() */

static void addproj (STRING dir);
static void makeSealibPath (void);
static void cleanListOfProjects (void);
static void cleanListOfInodes (void);

#define MAXPATHLENGTH 200
#define MAXENV       8000

static NAMELISTPTR listofprojs;
static NUM2LISTPTR listofinodes;
static char sealibpath[MAXENV+1];

char *tryNelsisSealib ()
{
   listofprojs = NULL;
   listofinodes = NULL;
   addproj (".");		/* recursively adds projlist */
   makeSealibPath ();
   cleanListOfProjects ();
   cleanListOfInodes ();
   if (!sealibpath[0]) return NULL;
   return sealibpath;
}

/* Add the project "DIR" to the namelist if it's not already there.
 * Then recursively do this for all imported projects.
 *
 * The tricky part here is to avoid inserting DIR if it already is in the
 * namelist. Suppose a project is NFS-mounted on two different mount points:
 *
 *        % mnt -t nfs muresh:/usr1 /usr1
 *        % mnt -t nfs muresh:/usr1 /network/muresh/usr1
 *
 * then the project muresh:/usr1/stravers/foobar has 2 canonical names. So
 * instead of using the canonical name we use the inode number of "foobar" to
 * recognize that we've already processed this project. Of course, the inode
 * number is not unique, because there will probably be more than one file
 * system. To decrease the probability, we match a inode number pair of
 * "foobar" and "foobar/.dmrc". The chances that both files have the same
 * inode number in different file systems are very close to 0 (at least
 * 4*10^-9 if inodes are at least 16 bits).
 *
 * And if you think we should use the device number: NOPE, the device number
 * for /usr1 is different from the device number for /network/muresh/usr1 ...
 */
static void addproj (STRING dir)
{
   long proj_ino, dmrc_ino;
   struct stat stat_buf;
   char path[MAXPATHLENGTH+1];

   if (!dir) return;

   while (isspace ((int)*dir)) ++dir; /* get rid of initial white spaces */

   if (stat (dir, &stat_buf)) return; /* cannot stat project dir */

   proj_ino = (long)stat_buf.st_ino;
   sprintf (path, "%s/.dmrc", dir);
   if (stat (path, &stat_buf)) return; /* not a nelsis3 project */

   dmrc_ino = (long)stat_buf.st_ino;
   /* we assume that the combination of proj_ino and dmrc_ino is unique */
   if (!isinnum2list (listofinodes, (long)proj_ino, (long)dmrc_ino))
   {
      FILE *fp;
      char *p = abscanonicpath (dir); /* remove symlinks and make canonic */
      tonamelist (&listofprojs, p);  /* add it if its not already there */
      tonum2list (&listofinodes, (long)proj_ino, (long)dmrc_ino);
      sprintf (path, "%s/projlist", dir);
      if (stat (path, &stat_buf)) return; /* no project list */
      if (!(fp = fopen (path, "r"))) return; /* cannot read project list */
      while (fgets (path, MAXPATHLENGTH, fp))
      {
	 int len = strlen (path);
	 if (len > 0) {
	    path[len - 1] = '\0'; /* get rid of trailing newline */
	    addproj (path);	  /* recursive call */
	 }
      }
      fclose (fp);
   }
}

/* add the "/seadif" subdirectory to the project directories and put
 * the resulting environment string in the global sealibpath.
 */
static void makeSealibPath ()
{
   NAMELISTPTR p = listofprojs;
   int l2, l3, len = 0;
   *sealibpath = 0;
   for (; p; p = p->next)
   {
      l2 = strlen (p->name);
      l3 = p->next ? 8 : 7;
      if (len + l2 + l3 > MAXENV) sdfreport (Fatal, "too long env");
      strcpy (sealibpath + len, p->name);
      len += l2;
      if (p->next)
	 strcpy (sealibpath + len, "/seadif:");
      else
	 strcpy (sealibpath + len, "/seadif");
      len += l3;
   }
}

/* clean up the name list */
static void cleanListOfProjects ()
{
   NAMELISTPTR p = listofprojs, nextp = NULL;
   for (; p; p = nextp)
   {
      nextp = p->next;
      FreeNamelist (p);
   }
   listofprojs = NULL;
}

/* clean up the name list */
static void cleanListOfInodes ()
{
   NUM2LISTPTR p = listofinodes, nextp = NULL;
   for (; p; p = nextp)
   {
      nextp = p->next;
      FreeNum2list (p);
   }
   listofinodes = NULL;
}
