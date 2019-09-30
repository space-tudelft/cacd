/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
 *	Simon de Graaf
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
 * Note: copied initial from nelsis R4 dmi
 *
 * Functions which provide conversion of paths from
 * local to remote and vice versa, using NFS-mount tables.
 */

#include "src/ocean/nelsea/prototypes.h"
#include "src/ocean/nelsea/def.h"
#include "src/ocean/nelsea/nelsis.h"
#ifdef CPLUSPLUSHACK
#include "src/ocean/libseadif/sysdep.h"
#endif

#define NM_SIZE 255

#include <mntent.h>

/* The include file <mntent.h> #define's the name of the mount table file
 * either as MNT_MNTTAB or MOUNTED, dependend on SYSV or BSD system.
 */
#if defined(MNT_MNTTAB)
#define MOUNTTAB MNT_MNTTAB
#else
#define MOUNTTAB MOUNTED
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

/* Local Declarations */
static char *getHostNamePart (char *hn, char *fsname);
static int matchPaths (char *path, char *mnt_dir);
static void noSignalError (char *rce, char *err);

/* Remote To Local Path conversion:
 * Given a hostname and absolute path (on the disk with name == hostname)
 * an absolute path from the local host (host where the tool is executed on)
 * is composed and (StrSaved) returned.
 */
char *RemToLocPath (char *rem_lib)
{
    struct stat buf;
    struct mntent *mntinfo;
    FILE *fp;
    char *fs_path; /* file-system path */
    char hostname[NM_SIZE+1];
    char hn[NM_SIZE+1];	/* temp. hostname */
    char loc_path[DM_MAXPATHLEN]; /* local_path */
    char *rem_path;
    int max_score, score; /* match scores */

    if (!(rem_path = getHostNamePart (hostname, rem_lib)) || *rem_path != '/') {
	noSignalError ("BADARG", "RemToLoc"); /* abs. path required */
	return (NULL);
    }

    /* Step 1.
     *    If specified hostname == local hostname:
     * 	  Return rem_path.
     */
    if (gethostname (hn, NM_SIZE) < 0) {
	noSignalError ("GETHOSTNAME", "RemToLoc");
	return (NULL);
    }

    if (strcmp (hostname, hn) == 0) {
	strcpy (loc_path, rem_path);
	goto exit_point;
    }

    /* Step 2.
     *    Get NFS mount info and lookup for automount directory
     */
    if (!(fp = setmntent (MOUNTTAB, "r"))) {
	noSignalError ("FOPENFAIL", "RemToLoc:mounttable");
	return (NULL);
    }

    score = max_score = 0;

    for (mntinfo = getmntent (fp); mntinfo; mntinfo = getmntent (fp))
    {
	/* consider only NFS mounts */
	//if (strcmp (mntinfo->mnt_type, MNTTYPE_NFS) != 0) continue;

	/* consider only fs mounts with ':' */
	if (!(fs_path = strchr (mntinfo->mnt_fsname, ':'))) continue;

	if (!getHostNamePart (hn, mntinfo->mnt_fsname)) {
	    noSignalError ("MOUNTTABERR", "RemToLoc:hostpart");
	    continue;
	}
	if (strcmp (hn, hostname) == 0) {
	    ++fs_path;
	    if (!*fs_path) {
		noSignalError ("MOUNTTABERR", "RemToLoc:pathpart");
	        continue;
	    }
	    score = matchPaths (rem_path, fs_path);
#ifdef DEBUG
	    fprintf (stderr, "RemToLoc: host=%s, fs_path=%s, sc=%d\n", hn, fs_path, score);
#endif
	    if (score > max_score) {
		max_score = score;
		strcpy (loc_path, mntinfo->mnt_dir);
#ifdef DEBUG
		fprintf (stderr, "RemToLoc: new mnt_dir=%s\n", loc_path);
#endif
	    }
	}
    }

    (void) endmntent (fp);

    if (max_score) { /* Hostname and rem_path mounted via NFS */
	if (max_score == 1) {
	    /*
	     * Example: dutedid:/ mounted on /dutedid.
	     * The matches are:
	     *    specified hostname == dutedid, mounted fs == '/'
	     * If specified rem_path is only "/" then skip it, else
	     * glue it to the mounted directory (/dutedid/.../..)
	     * Remark: score == 0 means that '/''ses are involved,
	     *         because only absolute paths can be mounted!
	     */
	    if (*rem_path == '/' && *(rem_path+1)) max_score = 0;
	}
	strcat (loc_path, rem_path + max_score);
    }
    else *loc_path = '\0';

exit_point:

    /* Step 7.
	* Check if it was not possible to create a local path.
	* Test the obtained path for existence.
	*/
    if (!*loc_path || stat (loc_path, &buf) < 0) {
	if (*loc_path)
	noSignalError ("RemToLoc", "stat < 0");
	noSignalError ("PATHNOEXIST", loc_path);
	return (NULL);
    }
    return (cs (loc_path));
}

/* Local To Remote Path:
 * Transforms the local path (abs. or rel.) to the corresponding hostname
 * and abs. path from the root of the disk with name 'hostname'.
 * If loc_path is a rel. path, it will be changed to an abs. path (and returned).
 * If the (prefix of the) local path has not been mounted locally,
 * than the specified loc_path is returned, hostname points to
 * the local host and dmerrno is set to DME_NOMOUNT.
 * Remark: returns are StrSaved.
 */
char *LocToRemPath (char **hostname, char *loc_path)
{
    struct mntent *mntinfo;
    FILE *fp;
    char *fs_path;
    char rem_path[DM_MAXPATHLEN];
    char new_path[DM_MAXPATHLEN];
    char hn[NM_SIZE+1];
    char cwd[DM_MAXPATHLEN];
    int score, max_score, error = 0;

    /* Step 1.
     *     Convert loc_path to an abs. path
     */
    if (!getcwd (cwd, DM_MAXPATHLEN)) { fprintf (stderr, "ERROR 1\n"); ++error; }
    if (chdir (loc_path) != 0) { fprintf (stderr, "ERROR 2\n"); ++error; }

    if (!getcwd (new_path, DM_MAXPATHLEN)) {
	fprintf (stderr, "ERROR 3: I am not allowed to access '%s'\n", loc_path);
	++error;
    }
    if (chdir (cwd) != 0) { fprintf (stderr, "ERROR 4\n"); ++error; }

    if (error) {
	noSignalError ("LocToRem", "getcwd/chdir");
	noSignalError ("PATHCONV", loc_path);
	return (NULL);
    }

    loc_path = new_path;

    /* Step 2.
     *     Try to find a NFS-mounttable slot
     */
    if (!(fp = setmntent (MOUNTTAB, "r"))) {
	noSignalError ("FOPENFAIL", "LocToRem:mounttable");
	return (NULL);
    }

    score = max_score = 0;

    for (mntinfo = getmntent (fp); mntinfo; mntinfo = getmntent (fp))
    {
	/* consider only NFS mounts */
	//if (strcmp (mntinfo->mnt_type, MNTTYPE_NFS) != 0) continue;

	/* consider only fs mounts with ':' */
	if (!(fs_path = strchr (mntinfo->mnt_fsname, ':'))) continue;

	score = matchPaths (loc_path, mntinfo->mnt_dir);
	if (score > max_score) {
	    if (!getHostNamePart (hn, mntinfo->mnt_fsname)) {
		noSignalError ("MOUNTTABERR", "LocToRem:hostpart");
		continue;
	    }
	    ++fs_path;
	    if (!*fs_path) {
		noSignalError ("MOUNTTABERR", "LocToRem:pathpart");
		continue;
	    }
            strcpy (rem_path, fs_path);
	    max_score = score;
#ifdef DEBUG
	    fprintf (stderr, "LocToRem: sc=%d, host=%s, path=%s\n", score, hn, rem_path);
#endif
	}
#ifdef DEBUG
	fprintf (stderr, "LocToRem: sc=%d, fs_name=%s\n", score, mntinfo->mnt_fsname);
#endif
    }

    (void) endmntent (fp);

    if (max_score > 0) {
	/* Test whether loc_path and mounted directory match in full.
	* If there remains a part of the loc_part, and the mounted
	* file-system is only '/' or '//' then skip the first slash
	* of that remainder.
	* e.g. mount dutente:/ /mnt/dutente
	*     with rem_path == /
	*     with loc_path == /mnt/dutente/tmp/abc (max_score=12)
	*/
	if (*(loc_path + max_score)) {
	    if (strcmp (rem_path, "/") == 0) {
		++max_score; /* skip slash of remainder of loc_path */
	    }
	    /* if there remains part of loc_path and the mount dir
	     * is only '/' then add a slash
	     * e.g. mount dutedix:/export/root/dutepil /
	     *   with rem_path == /export/root/dutepil
	     *   with loc_path == /tmp/abc (max_score=1)
	     */
	    else if (max_score == 1) --max_score;
	}

	loc_path = strcat (rem_path, loc_path + max_score);
    }
    else {
	/* The only conclusion which can be drawn at this point:
	* path is local (the path [or a part of it] is not mounted or such),
	* so return local hostname and loc_path.
	* The local-path has been tested under step 1.
	*/
	if (gethostname (hn, NM_SIZE) < 0) {
	    noSignalError ("GETHOSTNAME", "LocToRem");
	    return (NULL);
	}
    }

   *hostname = cs (hn);
    return (cs (loc_path));
}

/* This function qualifies the match of mnt_dir
 * (designating the mounted directory or the fs path prefix) to path.
 */
static int matchPaths (char *path, char *mnt_dir)
{
    int score;

    /*
     * Constraints: at the end-point + 1 position of equality
     * must be in force:
     * - mnt_dir is at its end
     * - path is at its end or starts with a new (sub)dir {'/'}
     * if not satisfied: match score = 0.
     * There is however one exception: if mnt_dir == "/";
     * in this case the score is set on 1
     */

    /* Arg checks: */
    if (!path || !mnt_dir) return (0);

    /* Exception_1: */
    if (strcmp (mnt_dir, "/") == 0 && *path == '/') return (1);

    /* Normal cases: */
    for (score = 0; *path && *mnt_dir && *path == *mnt_dir; path++, mnt_dir++, score++);

    /* A match of a number of whole segments (==sub-directories) is required,
     * e.g. mnt_dir="/usr" does not match with path="/usrx"
     */
    if (!*mnt_dir && (!*path || *path == '/')) return (score);
    return (0);
}

/* This function returns a char pointer to the 'pathname' part of
 * a 'hostname:path' string (e.g. as in first column in mounttable).
 * The hostname part is stored in the 'hn' buffer.
 */
static char *getHostNamePart (char *hn, char *fsname)
{
    int i;

    if (fsname)
    for (i = 0; *fsname; ++fsname) {
	if (*fsname == ':') { /* required */
	    if (i == 0) break;
	    if (i > NM_SIZE) error (FATAL_ERROR, "Too long hostname!");
	    hn[i] = '\0';
	    return (fsname+1);
	}
	if (i <= NM_SIZE) hn[i++] = *fsname;
    }
    return (NULL); /* no hostname part */
}

/* patrick: temp to get rid of error messages */
static void noSignalError (char *rce, char *err)
{
  if (!err)
    fprintf (stderr, "ERROR in hostnamemaker: %s: %s\n", rce, err);
}
