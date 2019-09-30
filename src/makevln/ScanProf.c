/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/makevln/incl.h"

#define UpdStopPos(c_sr) {\
if (c_sr -> ol_dur > MIN_INTEGER && (*p_stop_pos) > c_sr -> ol_dur)\
    *p_stop_pos = c_sr -> ol_dur;\
else if (*p_stop_pos > c_sr -> duration)\
    *p_stop_pos = c_sr -> duration; }

struct S_vln s_vln;
extern struct sr_field *h_sr;
extern FILE *vln_file;
extern long  sr_pos;

static int edge_type (struct sr_field *c_sr);
static void UpdSrField (struct sr_field *c_sr, int EdgeType);

/*
**  Generate the edges (i.e. line segments)
**  from the Stateruler Profile
*/
void ScanProf (long *p_stop_pos)
{
    struct sr_field *c_sr;
    struct sr_field *c_sr_next;
    int    EdgeType, EdgeTypeNxt, ConnType;
    int    upcon_flag;
    long   bottom, checktype, checktypenxt;

    upcon_flag = FALSE;
    c_sr = h_sr -> next;
    while (c_sr != h_sr) {
	EdgeType = edge_type (c_sr);
	if (EdgeType == 0) {
	    UpdStopPos (c_sr);
            c_sr_next = c_sr -> next;
	}
	else {
	    ConnType = NULL_CONN;
	    bottom = c_sr -> yb;
	    if (EdgeType & StartOl)
		checktype = 0;
	    else
		checktype = c_sr -> checktype;
       /* insertion of the upcon_flag which is set if two fields */
       /* are connected but have different edge or check_types.  */
       /* If not used wrong connection_types  may be generated   */
       /* because the previous field may have been deleted during*/
       /* the update of the field. J.Liedorp 29_8_85             */

	    if ((CONN_SGRP (c_sr -> prev, c_sr)) || (upcon_flag == TRUE)) {
		ConnType += DOWN_CONN;
                upcon_flag = FALSE;
            }
	    UpdStopPos (c_sr);
	    while (CONN_SGRP (c_sr, c_sr -> next)) {
		ConnType += UP_CONN;
       /* change in the program to prevent taking together edges */
       /* with different checktypes. J.Liedorp 29-8-85.          */
		EdgeTypeNxt = edge_type (c_sr -> next);
	        if (EdgeTypeNxt & StartOl)
		checktypenxt = 0;
	    else
		checktypenxt = c_sr -> next -> checktype;
		if ((EdgeType == EdgeTypeNxt) && (checktype == checktypenxt)) {
		    c_sr_next = c_sr -> next;
		    UpdSrField (c_sr, EdgeType);
		    c_sr = c_sr_next;
		    UpdStopPos (c_sr);
		}
		else if ((EdgeType & STOP) && (EdgeTypeNxt != 0)) {
                    upcon_flag = TRUE;
		    break;
                }
                else break;
		ConnType -= UP_CONN;
	    }

	    s_vln.x  = sr_pos;
	    s_vln.yb = bottom;
	    s_vln.yt = c_sr -> yt;
	    s_vln.occ = (char)(EdgeType | 0100);
	    s_vln.con = (char)(ConnType | 0100);
	    s_vln.grp = c_sr -> group;
	    s_vln.cht = checktype;
	    fwrite ((char *)&s_vln, sizeof (s_vln), 1, vln_file);

            c_sr_next = c_sr -> next;
	    UpdSrField (c_sr, EdgeType);
	}
	c_sr = c_sr_next;
    }
}

static int edge_type (struct sr_field *c_sr)
{
    int r_val;

    if (c_sr -> duration == sr_pos)
	r_val = STOP;
    else
	r_val = 0;

    if (c_sr -> ol_dur == sr_pos) {
	r_val |= StopOl;
	if (c_sr -> checktype)
	    r_val |= ChgCT;
    }

    if (c_sr -> flag.start)
	r_val |= START;

    if (c_sr -> flag.overlap) {
	r_val |= StartOl;
	if (c_sr -> flag.ct_zero == FALSE)
	    r_val |= ChgCT;
    }

    if (c_sr -> flag.incident)
	r_val |= ChgCT;

    return (r_val);
}

static void UpdSrField (struct sr_field *c_sr, int EdgeType)
{
    struct sr_field *t;

    if (EdgeType & STOP) {
	/* delete stateruler field */
	t = c_sr;
	c_sr -> next -> prev = c_sr -> prev;
	c_sr -> prev -> next = c_sr -> next;
	/* This makes no sense (AvG 280797): c_sr = c_sr -> prev; */
	FREE (t);
	return;
    }

    if (EdgeType & START)
	c_sr -> flag.start = FALSE;
    if (EdgeType & StartOl)
	c_sr -> flag.overlap = FALSE;
    if (EdgeType & StopOl) {
	c_sr -> ol_dur = MIN_INTEGER;
	c_sr -> flag.ol_area = FALSE;
    }
    if (EdgeType & ChgCT)
	c_sr -> flag.incident = FALSE;
    if (c_sr -> checktype)
	c_sr -> flag.ct_zero = FALSE;
    else
	c_sr -> flag.ct_zero = TRUE;
}
