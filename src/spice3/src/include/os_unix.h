/**********
Copyright 1990 Regents of the University of California. All rights reserved.
**********/

/*
 * Generic UNIX
 */

#define HAS_STAT		/* stat() returns info on files (inode) */
#define HAS_ISATTY		/* isatty()				*/
#define HAS_ACCESS		/* access()				*/
#define HAS_LONGJUMP		/* setjmp(), longjmp()		*/
#define HAS_WAIT		/* wait() wait for processes		*/

#define SYSTEM_PLOT5LPR	"lpr -P%s -g %s"
#define SYSTEM_PSLPR	"lpr -P%s %s"
#define SYSTEM_MAIL	"Mail -s \"%s (%s) Bug Report\" %s"
				/* simulator, version, address */
#define TEMPDIR "/tmp"

