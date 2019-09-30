/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.J. van Genderen
 *	S. de Graaf
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
    CACD-RELEASE-ALLOWED
*/

#define TRUE                  1
#define FALSE                 0

#define Node                 01
#define Node_t               02
#define Modelcall            03
#define Transistor           04
#define Functional           05
#define Intercap             06

#define Nenh                 01
#define Penh                 02
#define Depl                 03
#define Res                  04

#define F_OR               1001   /* the standard function numbers */
#define F_AND              1002
#define F_NOR              1003
#define F_NAND             1004
#define F_EXOR             1005
#define F_INVERT           1006
#define D_NENH             2001   /* the device numbers */
#define D_PENH             2002
#define D_DEPL             2003
#define D_RES              2004
#define D_CAP              2005

#define NODETYPE          1
#define CHARTYPE          2
#define INTEGERTYPE       3
#define FLOATTYPE         4
#define DOUBLETYPE        5
#define EMPTYTYPE         0
#define DIFFTYPE         -1
#define NAMENEG          -2
#define NOPATH           -3
#define NONODE           -4
#define REFIERR          -5
#define REFINEG          -6
#define REFIMIS          -7
#define NOTRELEVANT      -8

#define INPUT_T           1      /* trigger output   */
#define INPUT_R           2      /* read only output */
#define OUTPUT            3
#define INOUT             7      /* bidirectional    */
#define NOTYPE            0
#define CHAR              4
#define INTEGER           5
#define FLOAT             6
#define DOUBLE            8

#define MAXDIM                4   /* maximum dimension of node or
				     modelcall (=instance) name array */
#if DM_MAXNAME > 255
#define NAMESIZE             255+1
#else
#define NAMESIZE             DM_MAXNAME+1
#endif
				   /* maximum number of characters of node,
				      model or modelcall name + 1 */

#define MARK_OLD2_NAMESIZE    12321
#define MARK_NEW_NAMESIZE     23432

#define MAXDMPATH            80
