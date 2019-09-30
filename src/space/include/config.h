/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Nick van der Meijs
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

/* Feature: Moments Preserving Reduction, implies MOMENTS
 */
#define SNE			/* Enable Selective Node Elimination */

/* Note: Substrate Resistance extraction also implies MOMENTS
*/
#define MOMENTS			/* Enable Moments Extraction */

/* defines for space3d/Xspace program (SdeG)
*/
#if defined(CONFIG_SPACE3D) || defined(CONFIG_XSPACE)
#define CAP3D			/* Enable 3D Capacitance Extraction */
#define DISPLAY			/* Enable Xspace Animation */
#endif

#ifdef CONFIG_SPACE2
#undef MOMENTS
#undef SNE
#endif

#define TOR_DS_BOUNDARIES	/* Maintain drain/source boundaries
				   when extracting transistors.
				   This way, separate drains/sources
				   are found. */

/* #define MONITOR  		   -%m option for producing mon.out file
				   to be processed with prof (1), if your
				   system supports the monitor (3) function */

#define SCALE           4       /* Resolution of geo-coordinates, should
				 * be equal to SCALE in makegln. */
#define MAX_ELEM_NAME  32	/* Max length of element name */
#define MAX_TORNAME    16	/* Max length of transistor name */

#define MES_SUPPLYSHORT		/* Give a warning when vdd is connected to vss/gnd */

/* #define DEBUG_NODES		   Debug the nodes that are in core: use it to
				   debug when node items are left in core ! */

/*  Next are for portability among KR C, Ansi C and C++.
*/
#if defined(__STDC__) || defined(__cplusplus)
#define XTFUNCPROTO
#else
#define const /* nothing */
#endif

