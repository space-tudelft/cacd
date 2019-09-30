/*
 * ISC License
 *
 * Copyright (C) 2009-2018 by
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
 *	Cga2gds converts a GDS ASCII file
 *	into a GDS II binary file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include "src/libddm/dmincl.h"

#define TOOLVERSION "1.1 18-May-2009"

/* Stream Data Types */
#define DT_NODATA 0
#define DT_WORD   1
#define DT_SHORT  2
#define DT_LONG   3
#define DT_DOUBLE 5
#define DT_ASCII  6

char *argv0 = "cga2gds"; /* Program Name */
char *use_msg = "\nUsage: %s cga-file gds-file\n\n";

static int rec_nr;
static int x[8190+1];
static int y[8190+1];
static int yylen;

FILE *fpdata;
FILE *yyin;
char buf[1032];
char structure[280];
char *element;
char *record;

int yyparse (void);
void print_double (double);
void print_int2 (int);
void print_int4 (int);
void write_int2 (int rtype, int value);
void write_int4 (int rtype, int value);
void write_header (int rtype, int dtype, int nr);
void write_nodata (int rtype);
void write_string (int rtype, char *str, int len);
void write_bytes (char *str, int len);
void read_write_word (int rtype);

void add_read_info ()
{
    if (*structure) fprintf (stderr, "%s: structure %s:\n", argv0, structure);
    if (element) fprintf (stderr, "%s: element %s:\n", argv0, element);
    if (record) fprintf (stderr, "%s: record %s:\n", argv0, record);
}

void error_msg (char *msg)
{
    add_read_info ();
    fprintf (stderr, "%s: line %d: error: %s\n", argv0, rec_nr, msg);
    exit (1);
}

void warning (char *msg)
{
    fprintf (stderr, "%s: line %d: warning: %s\n", argv0, rec_nr, msg);
}

void expecting (char *item)
{
    add_read_info ();
    fprintf (stderr, "%s: line %d: expecting `%s', ", argv0, rec_nr, item);
    if (yylen > 60) { buf[60] = 0; fprintf (stderr, "found `%s...'.\n", buf); }
    else if (yylen > 0) fprintf (stderr, "found `%s'.\n", buf);
    else fprintf (stderr, "found `EOF'.\n");
    exit (1);
}

void test_expecting (char *item)
{
    if (strcmp (buf, item)) expecting (item);
}

void read_item (char *item)
{
    static int last_c;
    int dq = 0;

    if (last_c == '\n') ++rec_nr;

    yylen = 0;

    /* Skip white space.  */
    while ((last_c = fgetc (yyin)) != EOF && isspace (last_c)) if (last_c == '\n') ++rec_nr;
    if (last_c == '"') { ++dq; if ((last_c = fgetc (yyin)) == '"') goto ret; }
    if (last_c == EOF) expecting (item);

    buf[yylen++] = last_c;
    while ((last_c = fgetc (yyin)) != EOF) {
	if (dq) { if (last_c == '"') break; }
	else if (isspace (last_c)) break;
	if (yylen == 1024) error_msg ("buffer overflow (> 1024 chars)");
	buf[yylen++] = last_c;
    }
ret:
    buf[yylen] = 0;
}

int is_record (char *item)
{
    if (strcmp (buf, item)) return 0;
    record = item;
    return 1;
}

int is_element (char *item)
{
    if (strcmp (buf, item)) return 0;
    element = item;
    return 1;
}

int read_record (char *item, int test_item)
{
    record = NULL;
    read_item (item);
    if (is_record (item)) return 1;
    if (test_item) expecting (item);
    return 0;
}

double read_double ()
{
    char *s, *t;

    read_item ("double");
    s = buf;
    if (*s == '-' || *s == '+') ++s;
    t = s;
    while (isdigit (*s)) ++s;
    if (*t == '0' && s > t+1) s = t; /* error: leading zero */
    if (*s == '.') { ++s; ++t;
	while (isdigit (*s)) ++s;
    }
    if (s > t && (*s == 'E' || *s == 'e')) { ++s;
	if (*s == '-' || *s == '+') ++s;
	t = s; while (isdigit (*s)) ++s;
    }
    if (s == t || *s) expecting ("double");
    return strtod (buf, NULL);
}

int expect_int = 1;
int read_int (int min, int max)
{
    char *s, *t;
    int n = 0;

    read_item ("integer");
    s = buf;
    if (*s == '-' || *s == '+') ++s;
    t = s;
    while (isdigit (*s)) {
	if (n >= 0 && n <= max) n = 10*n + (*s - '0');
	++s;
    }
    if (s == t || (*t == '0' && s > t+1)) {
	if (expect_int) expecting ("integer");
	expect_int = 1;
    }
    else {
	if (n >= 0 && n <= max) {
	    if (*buf == '-') n = -n;
	    if (n >= min) return n;
	}
	add_read_info ();
	fprintf (stderr, "%s: line %d: found `%s', expecting integer in range [%d,%d].\n", argv0, rec_nr, buf, min, max);
	exit (1);
    }
    return 0;
}

int read_string (int maxlen)
{
    read_item ("string");
    if (yylen > maxlen) {
	add_read_info ();
	fprintf (stderr, "%s: line %d: found too long string, expecting <= %d chars.\n", argv0, rec_nr, maxlen);
	exit (1);
    }
    return yylen;
}

void read_elflags_plex (char *item)
{
    if (read_record (item, 0)) return;
    if (is_record ("ELFLAGS")) {
	read_write_word (38);
	if (read_record (item, 0)) return;
    }
    if (is_record ("PLEX")) {
	write_int4 (47, read_int (0, 33554431));
	if (read_record (item, 0)) return;
    }
    expecting (item);
}

void read_strans (char *item, int read)
{
    if (read && read_record (item, 0)) return;
    if (is_record ("STRANS")) {
	read_write_word (26);
	if (read_record (item, 0)) return;
    }
    if (is_record ("MAG")) {
	double v = read_double ();
	if (v <= 0) warning ("MAG value <= 0");
	write_header (27, DT_DOUBLE, 1);
	print_double (v);
	if (read_record (item, 0)) return;
    }
    if (is_record ("ANGLE")) {
	double v = read_double ();
	if (v < 0) warning ("ANGLE value < 0");
	write_header (28, DT_DOUBLE, 1);
	print_double (v);
	if (read_record (item, 0)) return;
    }
    expecting (item);
}

int main (int argc, char *argv[])
{
    if (argc <= 1) {
	fprintf (stderr, "%s %s\n", argv0, TOOLVERSION);
	fprintf (stderr, use_msg, argv0);
	return 1;
    }

    if (argc != 3) {
	fprintf (stderr, "%s: too `%s' arguments given\n", argv0, argc < 3 ? "less" : "many");
	fprintf (stderr, use_msg, argv0);
	return 1;
    }

    dmInit (argv0); /* for license */

    /* Open CGA input file.
    */
    if (!(yyin = fopen (argv[1], "r"))) {
	fprintf (stderr, "%s: cannot open/read cga-file `%s'\n", argv0, argv[1]);
	goto err;
    }

    if (fopen (argv[2], "r")) {
	fprintf (stderr, "%s: gds-file `%s' already exists\n", argv0, argv[2]);
	goto err;
    }

    /* Open GDS output file.
    */
    if (!(fpdata = fopen (argv[2], "wb"))) {
	fprintf (stderr, "%s: cannot open/write gds-file `%s'\n", argv0, argv[2]);
	goto err;
    }

    /* Parse input.
    */
    if (yyparse ()) goto err;

    fclose (fpdata);
    fprintf (stderr, "%s: -- program finished --\n", argv0);
    return 0;
err:
    fprintf (stderr, "%s: -- program aborted --\n", argv0);
    return 1;
}

void read_date ()
{
    print_int2 (read_int (0, 2050)); /* year */
    print_int2 (read_int (0, 12)); /* month */
    print_int2 (read_int (0, 31)); /* day */
}

void read_time ()
{
    print_int2 (read_int (0, 23)); /* hour */
    print_int2 (read_int (0, 59)); /* minute */
    print_int2 (read_int (0, 59)); /* second */
}

void read_date_and_time ()
{
    read_date (); read_time (); /* last modified */
    read_date (); read_time (); /* last access */
}

int yyparse ()
{
    int n, nr, firsttime;

    rec_nr = 1;

    read_record ("HEADER", 1);
    write_int2 (0, read_int (0, 700));

    read_record ("BGNLIB", 1);
    write_header (1, DT_SHORT, 12);
    read_date_and_time ();

    read_record ("LIBNAME", 1);
    write_string (2, buf, read_string (255));

    if (read_record ("UNITS", 0)) goto units;

    if (is_record ("REFLIBS")) {
	char *s, *t;
	n = read_string (512);
	t = buf; nr = 1;
	while (*t) if (*t++ == ';') ++nr;
	write_header (31, DT_ASCII, nr*44);
	s = buf;
	for (n = 0; n < nr; ++n) {
	    t = s;
	    while (*t && *t != ';') ++t;
	    if ((t - s) > 44) error_msg ("reference library name > 44");
	    if (*t) *t++ = 0;
	    write_bytes (s, 44);
	    s = t;
	}
	if (read_record ("UNITS", 0)) goto units;
    }
    if (is_record ("FONTS")) {
	char *s, *t;
	n = read_string (512);
	write_header (32, DT_ASCII, 4*44);
	s = buf;
	for (n = 0; n < 4; ++n) {
	    t = s;
	    while (*t && *t != ';') ++t;
	    if ((t - s) > 44) error_msg ("textfont file name > 44");
	    if (*t) *t++ = 0;
	    write_bytes (s, 44);
	    s = t;
	}
	if (read_record ("UNITS", 0)) goto units;
    }
    if (is_record ("ATTRTABLE")) {
	write_string (35, buf, read_string (44));
	if (read_record ("UNITS", 0)) goto units;
    }
    if (is_record ("GENERATIONS")) {
	write_int2 (34, read_int (2, 99));
	if (read_record ("UNITS", 0)) goto units;
    }
    if (is_record ("FORMAT")) {
	write_int2 (54, read_int (0, 1));
	if (read_record ("UNITS", 0)) goto units;
	if (is_record ("MASK")) {
	    do {
		write_string (55, buf, read_string (50));
	    } while (read_record ("MASK", 0));
	    test_expecting ("ENDMASKS");
	    write_nodata (56);
	}
	else expecting ("UNITS|MASK");
	if (read_record ("UNITS", 0)) goto units;
    }
    expecting ("UNITS");

units:
    write_header (3, DT_DOUBLE, 2);
    {
	double v;
	v = read_double ();
	if (v <= 0) warning ("UNITS value <= 0");
	print_double (v);
	v = read_double ();
	if (v <= 0) warning ("UNITS value <= 0");
	print_double (v);
    }

    while (read_record ("BGNSTR", 0)) {
	write_header (5, DT_SHORT, 12);
	read_date_and_time ();

	read_record ("STRNAME", 1);
	write_string (6, buf, read_string (255));
	strcpy (structure, buf);

	firsttime = 1;
	while (!read_record ("ENDSTR", 0)) {

	    if (firsttime) { firsttime = 0;
		if (is_record ("STRCLASS")) {
		    read_write_word (52);
		    continue;
		}
	    }

	    if (is_element ("BOUNDARY")) {
		write_nodata (8);

		read_elflags_plex ("LAYER");
		write_int2 (13, read_int (0, 255));

		read_record ("DATATYPE", 1);
		write_int2 (14, read_int (0, 255));

		read_record ("XY", 1);
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 8190) error_msg ("too many coordinate pairs (> 8190)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 4) error_msg ("too less coordinate pairs (< 4)");
		n = nr - 1;
		if (x[0] != x[n] || y[0] != y[n]) error_msg ("first/last point don't coincide");
		if (nr > 200) warning ("BOUNDARY has > 200 coordinate pairs");
		write_header (16, DT_LONG, nr * 2);
		for (n = 0; n < nr; ++n) { print_int4 (x[n]); print_int4 (y[n]); }
	    }
	    else if (is_element ("PATH")) {
		write_nodata (9);

		read_elflags_plex ("LAYER");
		write_int2 (13, read_int (0, 255));

		read_record ("DATATYPE", 1);
		write_int2 (14, read_int (0, 255));

		if (read_record ("XY", 0)) goto path_xy;

		if (is_record ("PATHTYPE")) {
		    write_int2 (33, read_int (0, 4));
		    if (read_record ("XY", 0)) goto path_xy;
		}
		if (is_record ("WIDTH")) {
		    write_int4 (15, read_int (-2147483647, 2147483647));
		    if (read_record ("XY", 0)) goto path_xy;
		}
		if (is_record ("BGNEXTN")) {
		    write_int4 (48, read_int (-32000, 32000));
		    if (read_record ("XY", 0)) goto path_xy;
		}
		if (is_record ("ENDEXTN")) {
		    write_int4 (49, read_int (-32000, 32000));
		    if (read_record ("XY", 0)) goto path_xy;
		}
		expecting ("XY");
path_xy:
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 8190) error_msg ("too many coordinate pairs (> 8190)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 2) error_msg ("too less coordinate pairs (< 2)");
		if (nr > 200) warning ("PATH has > 200 coordinate pairs");
		write_header (16, DT_LONG, nr * 2);
		for (n = 0; n < nr; ++n) { print_int4 (x[n]); print_int4 (y[n]); }
	    }
	    else if (is_element ("TEXT")) {
		write_nodata (12);

		read_elflags_plex ("LAYER");
		write_int2 (13, read_int (0, 255));

		read_record ("TEXTTYPE", 1);
		write_int2 (22, read_int (0, 255));

		if (read_record ("XY", 0)) goto text_xy;

		if (is_record ("PRESENTATION")) {
		    read_write_word (23);
		    if (read_record ("XY", 0)) goto text_xy;
		}
		if (is_record ("PATHTYPE")) {
		    write_int2 (33, read_int (0, 4));
		    if (read_record ("XY", 0)) goto text_xy;
		}
		if (is_record ("WIDTH")) {
		    write_int4 (15, read_int (-2147483647, 2147483647));
		    if (read_record ("XY", 0)) goto text_xy;
		}
		read_strans ("XY", 0);
text_xy:
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 1) error_msg ("too many coordinate pairs (> 1)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 1) error_msg ("too less coordinate pairs (< 1)");
		write_header (16, DT_LONG, 2);
		print_int4 (x[0]); print_int4 (y[0]);

		test_expecting (record = "STRING");
		write_string (25, buf, read_string (512));

		read_record ("ENDEL", 0);
	    }
	    else if (is_element ("SREF")) {
		write_nodata (10);

		read_elflags_plex ("SNAME");
		write_string (18, buf, read_string (255));

		read_strans ("XY", 1);
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 1) error_msg ("too many coordinate pairs (> 1)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 1) error_msg ("too less coordinate pairs (< 1)");
		write_header (16, DT_LONG, 2);
		print_int4 (x[0]); print_int4 (y[0]);
	    }
	    else if (is_element ("AREF")) {
		write_nodata (11);

		read_elflags_plex ("SNAME");
		write_string (18, buf, read_string (255));

		read_strans ("COLROW", 1);
		write_header (19, DT_SHORT, 2);
		print_int2 (read_int (0, 32767));
		print_int2 (read_int (0, 32767));

		read_record ("XY", 1);
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 3) error_msg ("too many coordinate pairs (> 3)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 3) error_msg ("too less coordinate pairs (< 3)");
		write_header (16, DT_LONG, 6);
		print_int4 (x[0]); print_int4 (y[0]);
		print_int4 (x[1]); print_int4 (y[1]);
		print_int4 (x[2]); print_int4 (y[2]);
	    }
	    else if (is_element ("NODE")) {
		write_nodata (21);

		read_elflags_plex ("LAYER");
		write_int2 (13, read_int (0, 255));

		read_record ("NODETYPE", 1);
		write_int2 (42, read_int (0, 255));

		read_record ("XY", 1);
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 50) error_msg ("too many coordinate pairs (> 50)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 1) error_msg ("too less coordinate pairs (< 1)");
		write_header (16, DT_LONG, nr * 2);
		for (n = 0; n < nr; ++n) { print_int4 (x[n]); print_int4 (y[n]); }
	    }
	    else if (is_element ("BOX")) {
		write_nodata (45);

		read_elflags_plex ("LAYER");
		write_int2 (13, read_int (0, 255));

		read_record ("BOXTYPE", 1);
		write_int2 (46, read_int (0, 255));

		read_record ("XY", 1);
		for (nr = 0;; ++nr) {
		    expect_int = 0;
		    x[nr] = read_int (-2147483647, 2147483647); /* x */
		    if (expect_int) break;
		    if (nr == 5) error_msg ("too many coordinate pairs (> 5)");
		    expect_int = 1;
		    y[nr] = read_int (-2147483647, 2147483647); /* y */
		}
		if (nr < 5) error_msg ("too less coordinate pairs (< 5)");
		if (x[0] != x[4] || y[0] != y[4]) error_msg ("first/last point don't coincide");
		write_header (16, DT_LONG, nr * 2);
		for (n = 0; n < nr; ++n) { print_int4 (x[n]); print_int4 (y[n]); }
	    }
	    else {
		expecting ("element|ENDSTR");
	    }

	    while (!is_record ("ENDEL")) {
		if (!is_record ("PROPATTR")) expecting ("ENDEL");
		write_int2 (43, read_int (1, 127));
		read_record ("PROPVALUE", 1);
		write_string (44, buf, read_string (255));
		if (read_record ("ENDEL", 0)) break;
	    }
	    write_nodata (17);
	    element = NULL;
	}

	write_nodata (7);
	*structure = 0;
    }

    test_expecting ("ENDLIB");
    write_nodata (4);

    return 0;
}
//===========================================================================

#define PUTBYTE(c) putc ((char)(c), fpdata)

/* Convert an int to two-byte integer number in GDS format.
*/
void print_int2 (int n)
{
    PUTBYTE (n >> 8);
    PUTBYTE (n);
}

/* Convert an int to four-byte integer number in GDS format.
*/
void print_int4 (int n)
{
    PUTBYTE (n >> 24);
    PUTBYTE (n >> 16);
    PUTBYTE (n >>  8);
    PUTBYTE (n);
}

/* Convert a double to eight-byte real number in GDS format.
*/
void print_double (double ln)
{
    double d, xx;
    unsigned int m_r, m_l, m_h;
    int e_h, expo;

    if (ln != 0) {
	xx = (ln < 0 ? -ln : ln);
	xx = frexp (xx, &expo);
	xx = ldexp (xx, 28 + expo % 4);
	m_l = (unsigned int) xx;
        d = ldexp (xx - m_l, 32);
        m_r = (unsigned int) d;
	if (m_l & 0XF0000000) {
	    e_h = 65 + expo / 4;
	    m_h = (m_l >> 8);
	    m_l = (m_l << 24) | (m_r >> 8);
	}
	else {
	    e_h = 64 + expo / 4;
	    m_h = (m_l >> 4);
	    m_l = (m_l << 28) | (m_r >> 4);
	}
	if (e_h > 127) { /* overflow */
	    warning ("double value overflow");
	    e_h = 127;
	    m_h = 0X00FFFFFF;
	    m_l = 0XFFFFFFFF;
	}
	else if (e_h < 0) { /* underflow */
	    goto zero;
	}
	if (ln < 0) e_h |= 0X80;
	m_h |= (e_h << 24);
    }
    else { /* zero value */
zero:
	m_l = m_h = 0;
    }
    print_int4 (m_h);
    print_int4 (m_l);
}

/* Write a record header to the outputfile.
** The header contains the record byte count and the record type.
*/
void write_header (int rtype, int dtype, int number)
{
    switch (dtype) { /* makes bytes of the number */
    case DT_ASCII:  break;
    case DT_SHORT:  number *= 2; break;
    case DT_LONG:   number *= 4; break;
    case DT_DOUBLE: number *= 8; break;
    default: error_msg ("write_header: incorrect data type");
    }
    number += 4; /* add 4 bytes, the basic record header length */
    if (number > 65534) error_msg ("write_header: incorrect byte count");
    PUTBYTE (number >> 8);
    PUTBYTE (number);
    PUTBYTE (rtype);
    PUTBYTE (dtype);
}

void write_nodata (int rtype)
{
    PUTBYTE (0);
    PUTBYTE (4); /* 4 bytes */
    PUTBYTE (rtype);
    PUTBYTE (DT_NODATA);
}

/* Write ONE Two-Byte_Integer record to the outputfile.
*/
void write_int2 (int rtype, int value)
{
    PUTBYTE (0);
    PUTBYTE (6); /* 4 + 2 bytes */
    PUTBYTE (rtype);
    PUTBYTE (DT_SHORT); /* short */
    print_int2 (value);
}

/* Write ONE Four-Byte_Integer record to the outputfile.
*/
void write_int4 (int rtype, int value)
{
    PUTBYTE (0);
    PUTBYTE (8); /* 4 + 4 bytes */
    PUTBYTE (rtype);
    PUTBYTE (DT_LONG);
    print_int4 (value);
}

/* Write ASCII_String record to the outputfile.
*/
void write_string (int rtype, char *s, int len)
{
    int odd = len % 2;
    if (odd) ++len;
    write_header (rtype, DT_ASCII, len);
    while (*s) { PUTBYTE (*s); ++s; }
    if (odd) PUTBYTE (0);
}

void write_bytes (char *s, int len)
{
    int i = 0;
    while (i++ < len) { PUTBYTE (*s); if (*s) ++s; }
}

/* Write Bit Array record to the outputfile.
*/
void read_write_word (int rtype)
{
    int i, n;

    read_item ("word");
    if (yylen != 16) expecting ("word");

    /* convert word string into two-byte number */
    n = 0;
    for (i = 0; i < 16; ++i) {
	n <<= 1;
	if (buf[i] == '1') ++n;
	else if (buf[i] != '0') expecting ("word");
    }

    PUTBYTE (0);
    PUTBYTE (6); /* 4 + 2 bytes */
    PUTBYTE (rtype);
    PUTBYTE (DT_WORD); /* bit array word */
    print_int2 (n);
}
