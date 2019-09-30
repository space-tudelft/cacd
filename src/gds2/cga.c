/*
 * ISC License
 *
 * Copyright (C) 1987-2018 by
 *	R. Paulussen
 *	S. de Graaf
 *	A.J. van Genderen
 *	N.P. van der Meijs
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
 * PROCESS DESCRIPTION:
 *	Cga converts a GDS formatted file into ASCII.
 *	The ASCII-output is sent to the standard output.
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>

#define PE fprintf (stderr,
#define PO fprintf (stdout,

#define BLOCKSIZE    2048
#define NR_REC_TYPES 57

/* record types: */
#define BGNLIB   1
#define UNITS    3
#define ENDLIB   4
#define BGNSTR   5
#define STRNAME  6
#define ENDSTR   7
#define BOUNDARY 8
#define PATH     9
#define SREF    10
#define AREF    11
#define TEXT    12
#define XY      16
#define ENDEL   17
#define SNAME   18
#define NODE    21
#define REFLIBS 31
#define FONTS   32
#define BOX     45

static
char   *rec_dscr[NR_REC_TYPES] =
{
    "HEADER", "BGNLIB", "LIBNAME", "UNITS", "ENDLIB",
    "BGNSTR", "STRNAME", "ENDSTR", "BOUNDARY", "PATH",
    "SREF", "AREF", "TEXT", "LAYER", "DATATYPE",
    "WIDTH", "XY", "ENDEL", "SNAME", "COLROW",
    "TEXTNODE", "NODE", "TEXTTYPE", "PRESENTATION", "SPACING",
    "STRING", "STRANS", "MAG", "ANGLE", "UINTEGER",
    "USTRING", "REFLIBS", "FONTS", "PATHTYPE", "GENERATIONS",
    "ATTRTABLE", "STYPTABLE", "STRTYPE", "ELFLAGS", "ELKEY",
    "LINKTYPE", "LINKKEYS", "NODETYPE", "PROPATTR", "PROPVALUE",
    "BOX", "BOXTYPE", "PLEX", "BGNEXTN", "ENDEXTN",
    "TAPENUM", "TAPECODE", "STRCLASS", "RESERVED", "FORMAT",
    "MASK", "ENDMASKS"
};

static
char   *rec_short[NR_REC_TYPES] =
{
    "HE", "BL", "LI", "UN", "EL", "BS", "NA", "ES", "BD", "PA",
    "SR", "AR", "TE", "LA", "DA", "WI", "XY", "EE", "SN", "CO",
    "TN", "NO", "TT", "PS", "SP", "SI", "ST", "MG", "AN", "UI",
    "US", "RL", "FO", "PT", "GE", "AT", "SB", "SY", "EF", "EK",
    "LT", "LK", "NT", "PR", "PV", "BO", "BT", "PX", "BX", "EX",
    "TA", "TC", "SC", "RE", "FM", "MA", "EM"
};

unsigned
char    buffer[BLOCKSIZE];	/* buffer of 1 tape block */
char    buf[80];

int     stdout_closed = 0;
int     record_len;
int     file_len;
int     data_type;
int     record_type = -1;
int     rec_nr = 1;
int     bflag;
char   *bname;
int     lflag;
int     nflag;
int     oflag = -1;
int     sflag;
int     tflag;
int     uflag;
int     wflag;
int     xflag;
int     usage;
int     uuf = 1;

FILE   *fp_gds;

char   *argv0 = "cga";		/* Program Name */
char   *use_msg =		/* Command Line */
"\nUsage: %s [-bNAME] [-l] [-n] [-oRT[,C]] [-pRN[,R2]] [-s] [-t] [-u[F]] [-w] [-x] GDS-file\n\n";

void read_units (void);
void read_integer (int type);
void read_string (void);
void read_bit (void);
void read_double (void);
void pr_exit (int err_no, int nr);

int main (int argc, char *argv[])
{
    char *cp;
    int rec_len;
    int file_size;
    int i;
    int j = sizeof (int);
    int o_rec = 0, o_cnt = 0;
    int rec1 = 1;
    int rec2 = 0X7FFFFFFE;
    register int skipflag = 0;
    register int skipelement = 0;
    register int no_nl = 0;
    register unsigned char *bp;
    int BOUNDARY_cnt = 0, PATH_cnt = 0, TEXT_cnt = 0, NODE_cnt = 0, BOX_cnt = 0;

    for (i = 1; i < argc && *argv[i] == '-'; ++i) {
	switch (argv[i][1]) {
	case 'b':
	    bflag = 1;
	    bname = &argv[i][2];
	    if (!*bname) {
		++usage;
		PE "%s: -b: no structure name given\n", argv0);
	    }
	    if (lflag) lflag = 0;
	    break;
	case 'l':
	    if (!bflag) lflag = 1;
	    break;
	case 'n':
	    nflag = 1;
	    break;
	case 'o':
	    o_cnt = 0;
	    if ((cp = strchr (argv[i], ','))) {
		*cp++ = '\0';
		if ((o_cnt = atoi (cp)) < 0) o_cnt = 0;
	    }
	    o_rec = o_cnt;
	    oflag = atoi (&argv[i][2]);
	    if (oflag >= NR_REC_TYPES || oflag < 0) {
		++usage;
		PE "%s: -o: illegal record type: %d\n", argv0, oflag);
	    }
	    break;
	case 'p':
	    if ((cp = strchr (argv[i], ','))) *cp++ = '\0';
	    if ((rec1 = atoi (&argv[i][2])) < 1) rec1 = 1;
	    rec2 = rec1;
	    if (cp && (rec2 = atoi (cp)) < rec1) rec2 = rec1;
	    break;
	case 's':
	    sflag = 1;
	    break;
	case 't':
	    tflag = 1;
	    break;
	case 'w':
	    wflag = 1;
	    break;
	case 'x':
	    xflag = 1;
	    break;
	case 'u':
	    if (argv[i][2]) {
		if ((uuf = atoi (&argv[i][2])) <= 0) {
		    ++usage;
		    PE "%s: -u: illegal mul. factor: %d\n", argv0, uuf);
		}
	    }
	    else uflag = 1;
	    break;
	default:
	    ++usage;
	    PE "%s: %s: unknown option\n", argv0, argv[i]);
	    break;
	}
    }

    if (tflag) {
	PO "%s: RT:  SN:  LNAME:\n", argv0);
	PO "%s: ----------------\n", argv0);
	for (i = 0; i < NR_REC_TYPES; ++i)
	    PO "%s: %2d - %s - %s\n", argv0, i, rec_short[i], rec_dscr[i]);
	goto fini;
    }

    if (lflag && wflag) {
	++usage;
	PE "%s: options -l and -w cannot both be given\n", argv0);
    }

    if (i >= argc) {
	++usage;
	PE "%s: no GDS file given\n", argv0);
    }
    else if (i < argc - 1) {
	++usage;
	PE "%s: too many arguments given\n", argv0);
    }
    if (usage) {
	PE use_msg, argv0);
	exit (1);
    }

    if (j != 4) {
	PE "%s: error: sizeof int not equal to 4 bytes\n", argv0);
	exit (1);
    }

    if (!(fp_gds = fopen (argv[i], "rb"))) {
	PE "%s: cannot open GDS file '%s'\n", argv0, argv[i]);
	exit (1);
    }

    fseek (fp_gds, 0L, 2);
    file_size = file_len = ftell (fp_gds);
    if (file_len & 1) pr_exit (1, file_len);
    fseek (fp_gds, 0L, 0);

    if (uflag) read_units ();

    /*
    ** Process GDS input data:
    */
    while (file_len >= 4) {
	if (!fread ((char *) buffer, 4, 1, fp_gds)) pr_exit (2, 0);
	file_len -= 4;
	bp = buffer;
	record_len  = *bp++;
	record_len  = (record_len << 8) | *bp++;
	if (record_type == ENDLIB) break;
	record_type = *bp++;
	data_type   = *bp;

	if (record_type >= NR_REC_TYPES || record_type < 0)
	    pr_exit (8, record_type);

	if (rec_nr < rec1) ++skipflag;
	else if (oflag >= 0) {
	    if (record_type == oflag) o_rec = 0;
	    else if (++o_rec > o_cnt) ++skipflag;
	}

	if (record_len < 4) {
	    if (record_len == 0 && record_type == 0 && data_type == 0) {
		if (!skipflag && !lflag) {
		    if (nflag) PO "%d:", rec_nr);
		    PO "%s %d\n", sflag ? "PD" : "PADDING", file_len + 4);
		}
		break; /* padding found */
	    }
	    pr_exit (3, record_len);
	}
	if (record_len & 1)
	    PE "%s: warning: record %d, odd record length (= %d)\n",
		argv0, rec_nr, record_len);

	if (lflag || wflag)
	switch (record_type) {
	    case SNAME:
	    case STRNAME:
		if (!wflag) goto skip_print;
		if (record_type == SNAME) PO " ");
		else PO "\n");
		break;

	    case BGNSTR:
		BOUNDARY_cnt = 0;
		PATH_cnt = 0;
		TEXT_cnt = 0;
		NODE_cnt = 0;
		BOX_cnt = 0;
		PO "\n");
	    case SREF:
	    case AREF:
		no_nl = 1;
		break;

	    case ENDSTR:
		if (wflag) break;
		if (BOUNDARY_cnt) PO ": BOUNDARY = %d\n", BOUNDARY_cnt);
		if (PATH_cnt) PO ": PATH = %d\n", PATH_cnt);
		if (TEXT_cnt) PO ": TEXT = %d\n", TEXT_cnt);
		if (NODE_cnt) PO ": NODE = %d\n", NODE_cnt);
		if (BOX_cnt) PO ": BOX = %d\n", BOX_cnt);
		break;

	    case ENDEL:
		if (wflag) PO " ");
		else skipelement = 1;
		break;
	    case BOUNDARY:
		++BOUNDARY_cnt;
		if (wflag) no_nl = 1;
		else skipelement = 1;
		break;
	    case PATH:
		++PATH_cnt;
		if (wflag) no_nl = 1;
		else skipelement = 1;
		break;
	    case TEXT:
		++TEXT_cnt;
		if (wflag) no_nl = 1;
		else skipelement = 1;
		break;
	    case NODE:
		++NODE_cnt;
		if (wflag) no_nl = 1;
		else skipelement = 1;
		break;
	    case BOX:
		++BOX_cnt;
		if (wflag) no_nl = 1;
		else skipelement = 1;
		break;

	    default:
		if (no_nl) PO " ");
	}

	if (!skipflag && !skipelement && !bflag) {
	    if (nflag && !lflag) PO "%d:", rec_nr);
	    PO "%s", sflag ? rec_short[record_type] : rec_dscr[record_type]);
	    if (record_type == ENDSTR && bname) {
		PO "\n");
		break;
	    }
	}

skip_print:
	if ((record_len -= 4) == 0) {
	    if (data_type == 0) goto next;
	    pr_exit (7, data_type);
	}

	rec_len = record_len;
	do {
	    if ((record_len = rec_len) > BLOCKSIZE) record_len = BLOCKSIZE;
	    if (!fread ((char *) buffer, record_len, 1, fp_gds)) pr_exit (5, 0);
	    file_len -= record_len;

	    switch (data_type) {
	    case 0:
		pr_exit (7, data_type);
		break;
	    case 1:
		if (record_len % 2) pr_exit (7, data_type);
		if (!skipflag && !skipelement && !bflag) read_bit ();
		break;
	    case 2:
		if (record_type == BGNLIB || record_type == BGNSTR) {
		    if (record_len != 24) pr_exit (7, data_type);
		    if (lflag && record_type == BGNSTR) break;
		}
		else {
		    if (record_len % 2) pr_exit (7, data_type);
		}
		if (!skipflag && !skipelement) read_integer (2);
		break;
	    case 3:
		if (record_type == XY) {
		    if (record_len % 8) pr_exit (7, data_type);
		}
		else {
		    if (record_len % 4) pr_exit (7, data_type);
		}
		if (!skipflag && !skipelement && !bflag) read_integer (4);
		break;
	    case 4:
		pr_exit (7, data_type);
		break;
	    case 5:
		if (record_len % 8) pr_exit (7, data_type);
		if (!skipflag && !skipelement && !bflag) read_double ();
		break;
	    case 6:
		if (!skipflag && !skipelement) read_string ();
		break;
	    default:
		pr_exit (6, data_type);
	    }
	} while ((rec_len -= record_len) > 0);
next:
	if (no_nl) {
	    if (record_type == ENDEL || record_type == STRNAME)
		no_nl = skipelement = 0;
	}
	if (!skipflag && !skipelement && !no_nl && !bflag) PO "\n");
	else skipflag = 0;
	if (record_type == ENDEL) skipelement = 0;
	if (++rec_nr > rec2) break;
    }
    fclose (stdout);
    stdout_closed = 1;

    if (rec_nr > rec2 || bname) goto fini;

    if (record_type != ENDLIB) {
	PE "%s: warning: last record is not ENDLIB\n", argv0);
    }
    if (file_len == 0) {
	/*
	if (file_size % BLOCKSIZE) PE "%s: warning: no padding found\n", argv0);
	*/
	goto fini;
    }
    if (file_len >= BLOCKSIZE - 4) {
	PE "%s: warning: too many padding found\n", argv0);
	goto fini; /* note: read buffer too small */
    }
    if (!fread ((char *) buffer, file_len, 1, fp_gds)) {
	PE "%s: read error while reading padding\n", argv0);
	goto fini;
    }
    bp = buffer;
    while (file_len--) {
	if (*bp++) {
	    PE "%s: warning: padding not null\n", argv0);
	    break;
	}
    }
fini:
    PE "%s: -- program finished --\n", argv0);
    exit (0);
    return (0);
}

void read_units ()
{
    double  ln;
    register unsigned char *bp;
    register int e_h;
    unsigned int m_l, m_h;
    int fl = file_len;

    while (fl >= 4) {
	if (!fread ((char *) buffer, 4, 1, fp_gds)) pr_exit (2, 0);
	fl -= 4;
	bp = buffer;
	record_len  = *bp++;
	record_len  = (record_len << 8) | *bp++;
	record_type = *bp++;
	data_type   = *bp;

	if (record_len < 4) {
	    if (record_len == 0 && record_type == 0 && data_type == 0) {
		break; /* padding found */
	    }
	    pr_exit (3, record_len);
	}
	if ((record_len -= 4) == 0) goto next;

	if (record_len > BLOCKSIZE) pr_exit (0, record_len + 4);
	if (!fread ((char *) buffer, record_len, 1, fp_gds)) pr_exit (5, 0);
	fl -= record_len;

	if (data_type == 5) {
	    if (record_len % 8) pr_exit (7, data_type);
	    if (record_type == UNITS) {
		uflag = 0;
		bp = buffer;
		e_h = *bp++;
		m_h = *bp++;
		m_h = (m_h << 8) | *bp++;
		m_h = (m_h << 8) | *bp++;
		m_l = *bp++;
		m_l = (m_l << 8) | *bp++;
		m_l = (m_l << 8) | *bp++;
		m_l = (m_l << 8) | *bp++;
		if (m_h) {
		    ln = m_l;
		    ln = ldexp (ln + 4294967296.0 * m_h, -56);
		    ln = ldexp (ln, 4 * ((e_h & 0X7F) - 64));
		    if (e_h & 0X80) ln = -ln;
		}
		else ln = 0;
		uuf = (int)(1.0 / ln + 0.01);
		ln = 1.0 - ln * uuf;
		if (ln < -0.001 || ln > 0.001) {
		    uuf = 1;
		    PE "%s: option -u: sorry, cannot been used!\n", argv0);
		}
		break;
	    }
	}
next:
	if (++rec_nr > 10) break;
    }
    if (uflag) {
	PE "%s: option -u: cannot find UNITS record\n", argv0);
    }
    fseek (fp_gds, 0L, 0); /* rewind */
    rec_nr = 1;
}

void read_integer (int type) /* Read 2 or 4 byte integer numbers */
{
    register int i, nr, value;
    register char *s, *c;
    register unsigned char *bp = buffer;

    nr = (type == 4) ? record_len >> 2 : record_len >> 1;

    s = buf; *s++ = ' ';
    for (i = 0; i < nr; ++i) {
	if (xflag && record_type == XY) *buf = (i % 2) ? ' ' : '\n';
	value = *bp++;
	value = (value << 8) | *bp++;
	if (type == 4) {
	    value = (value << 8) | *bp++;
	    value = (value << 8) | *bp++;
	    if (uuf != 1) {
		if (!(value % uuf)) {
		    sprintf (s, "%d", value / uuf);
		}
		else {
		    sprintf (s, "%.3f", (double)value / uuf);
		    while (*++s != '.');
		    c = s + 3;
		    if (*c == '0') if (*--c == '0') --c;
		    *++c = 0;
		    s = buf + 1;
		}
	    }
	    else sprintf (s, "%d", value);
	}
	else sprintf (s, "%d", value);
	if (bflag) {
	    if (record_type == BGNSTR) {
		while (*++s);
		if (i < 11) *s++ = ' ';
	    }
	}
	else
	    PO buf);
    }
}

void read_string () /* Read ASCII string */
{
    int i, c, dq;
    char *s;
    buffer[record_len] = '\0';
    if (bflag) {
	if (record_type != STRNAME
            || strcmp (bname, (char *)buffer)) return;
	if (nflag) PO "%d:", rec_nr - 1);
	PO "%s%s\n", sflag ? rec_short[BGNSTR] : rec_dscr[BGNSTR], buf);
	if (nflag) PO "%d:", rec_nr);
	PO "%s", sflag ? rec_short[STRNAME] : rec_dscr[STRNAME]);
	bflag = 0;
    }
    if (record_type == FONTS || record_type == REFLIBS) {
	if (!*buffer && record_len <= 44) goto quote;
	dq = 0;
	for (i = 0; i < record_len; ++i) {
	    if (buffer[i] == ' ') { ++dq; break; }
	}
	if (dq) PO " \"");
	else PO " ");
	for (i = 0; i < record_len; i += 44) {
	    s = (char*)buffer + i + 44;
	    c = *s;
	    *s = 0;
	    if (i)
		PO ";%s", buffer + i);
	    else
		PO "%s", buffer + i);
	    *s = c;
	}
	if (dq) PO "\"");
    }
    else if (!*buffer || strchr ((char*)buffer, ' ')) {
quote:
	PO " \"%s\"", buffer);
    }
    else {
	PO " %s", buffer);
    }
}

void read_bit () /* Read bit-array words */
{
    register int i, nr, mask, word;
    register char *cp;
    register unsigned char *bp = buffer;

    nr = record_len >> 1;
    for (i = 0; i < nr; ++i) {
	cp = buf;
	*cp++ = ' ';
	word = *bp++;
	word = (word << 8) | *bp++;
	for (mask = 0X8000; mask; mask >>= 1) { /* for every bit */
	    *cp++ = (mask & word) ? '1' : '0';
	}
	*cp = '\0';
	PO "%s", buf);
    }
}

void read_double () /* Read 8 byte real numbers */
{
    double  ln;
    register unsigned char *bp = buffer;
    register char *cp;
    register int i, nr, e_h;
    unsigned int m_l, m_h;

    nr = record_len >> 3;

    for (i = 0; i < nr; ++i) {
	e_h = *bp++;
	m_h = *bp++;
	m_h = (m_h << 8) | *bp++;
	m_h = (m_h << 8) | *bp++;
	m_l = *bp++;
	m_l = (m_l << 8) | *bp++;
	m_l = (m_l << 8) | *bp++;
	m_l = (m_l << 8) | *bp++;
	if (m_h) {
	    ln = m_l;
	    ln = ldexp (ln + 4294967296.0 * m_h, -56);
	    ln = ldexp (ln, 4 * ((e_h & 0X7F) - 64));
	    if (e_h & 0X80) ln = -ln;
	}
	else ln = 0;
	if (record_type == UNITS) ln = ln * uuf;

	sprintf (buf, "%-7.4G", ln);
	cp = buf;
	while (*++cp)
	    if (*cp == ' ') {
		*cp = '\0';
		break;
	    }

	PO " %s", buf);
    }
}

void pr_exit (int err_no, int nr)
{
    if (!stdout_closed) {
	PO "\n");
	fclose (stdout);
    }
    PE "%s: %d: error: ", argv0, rec_nr);
    switch (err_no) {
	case 0: PE "too long record length (= %d)", nr); break;
	case 1: PE "odd file length (= %d)", nr); break;
	case 2: PE "cannot read record header"); break;
	case 3: PE "too small record length (= %d)", nr); break;
	case 4: PE "odd record length (= %d)", nr); break;
	case 5: PE "cannot read record"); break;
	case 6: PE "unknown data type (= %d)", nr); break;
	case 7: PE "data type %d, incorrect # of bytes (= %d)",
		    nr, record_len); break;
	case 8: PE "unknown record type (= %d)", nr); break;
    }
    PE "\n%s: -- program aborted --\n", argv0);
    exit (1);
}
