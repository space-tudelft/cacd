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

#define BLOCKSIZE       2048      /* GDS tape blocksize  */
#define D_BLOCKSIZE    (BLOCKSIZE<<1)
#define VERSION            5  /* versionnumber of GDS-format  */
#define BASE_LEN           4  /* Basic-length of all records  */
#define HEADER        0X0002
#define BGNLIB        0X0102
#define BGNSTR        0X0502
#define LIBNAME       0X0206
#define UNITS         0X0305
#define ENDLIB        0X0400
#define STRNAME       0X0606
#define ENDSTR        0X0700
#define BOUNDARY      0X0800
#define PATH          0X0900
#define SREF          0X0A00
#define AREF          0X0B00
#define LAYER         0X0D02
#define DATATYPE      0X0E02
#define WIDTH         0X0F03
#define XY            0X1003
#define ENDEL         0X1100
#define SNAME         0X1206
#define COLROW        0X1302
#define STRANS        0X1A01
#define STRING        0X1906
#define PRESENTATION  0X1701
#define TEXTTYPE      0X1602
#define MAG           0X1B05
#define ANGLE         0X1C05
#define PATHTYPE      0X2102
#define TAPENUM       0X3202
#define TAPECODE      0X3302
#define STRCLASS      0X3401

#define BOX 	      0x2D00
#define NODE 	      0x1500
#define TEXT 	      0x0C00
#define PROPATTR      0x2B02
#define PROPVALUE     0x2C06

#define NR_BLOCKS_L     1750   /* no. of blocks by  800 bpi/600 ft */
#define NR_BLOCKS_H     3250   /* no. of blocks by 1600 bpi/600 ft */

#define J                 16   /* information only printed if not -s */
#define R                  8
#define B                  4   /* if added, print also record nr.  */
#define A                  3   /* pr_exit modes: A=abort,          */
#define W                  2   /* W=warning, E=exit, I=information */
#define I                  1   /* B=abort unless -g is used        */
#define E                  0

#define MAX_STRLEN       512   /* maximum string length            */
#define MAX_COOR        8190   /* max. no. of polygon coord. pairs */
#define MAX_DOUBLE         2
#define MAX_GDS_LAYNR    255   /* max. layer number */

#define BUFLEN           512

#define DOUBLE             8
#define LONG	           4
#define SHORT	           2

#define DEF_ERR           -1
#define FALSE              0
#define TRUE               1

#define MAX32     2147483647
#define MIN32    -2147483648

#define PE    fprintf(stderr,

#define Abs(x)            (((x) < 0) ? -(x) : (x))

/* cgi.c */
int _quadragon (double x1, double y1, double x2, double y2, double x3, double y3, double x4, double y4);

/* swire.c */
int ese_swire (void);
int do_swire (void);

