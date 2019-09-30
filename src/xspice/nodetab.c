
/*
 * ISC License
 *
 * Copyright (C) 1999-2011 by
 *	Arjan van Genderen
 *	Simon de Graaf
 *	Edwin Matthijssen (APinfo)
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

#include "src/xspice/incl.h"

static long numberOfNodes = 0;
struct node_info *nodetab = NULL;

#ifdef __cplusplus
  extern "C" {
#endif
static struct ap_info *getAPinfo    (long N_nx, char *dev_name);
static struct ap_info *createAPinfo (long N_nx, char *dev_name);
#ifdef __cplusplus
  }
#endif

/*
** Creates the node table, allocates the needed amount of memory and
** initializes everything to zero.
*/
void createNodeTab (long non)
{
    long l;

    PALLOC (nodetab, non, struct node_info);
    numberOfNodes = non;
    for (l = 0; l < non; l++) {
	nodetab[l].isTerm = 0;
	nodetab[l].aplist = NULL;
    }
}

/*
** Delete the total node table structure, free all allocated memory.
*/
void deleteNodeTab ()
{
    long l;
    struct ap_info_ref *apiref, *help;

    for (l = 0; l < numberOfNodes; l++) {
	apiref = nodetab[l].aplist;

	while (apiref) {
	    Free (apiref -> ap);
	    help = apiref -> next;
	    Free (apiref);
	    apiref = help;
	}
    }
    Free (nodetab);
}

/*
** Add the area and perimeter for a certain transistor (specified by 'dev_name')
** to the node table of the node with index number N_nx.
** If the device does not yet exist on this node, create a new structure.
*/
void addAP (long N_nx, char *dev_name, double area, double perim)
{
    struct ap_info *ap;

    ap = getAPinfo (N_nx, dev_name);
    if (!ap) ap = createAPinfo (N_nx, dev_name);
    ap -> area  += area;
    ap -> perim += perim;
}

/*
** Get the area and perimeter information and
** the total number of transistors and the total width
** from the transistors with name 'dev_name' on node with index number N_nx.
*/
int getAP (long N_nx, char *dev_name, double *area, double *perim, long *cnt, double *width)
{
    struct ap_info *ap;

    ap = getAPinfo (N_nx, dev_name);
    if (ap) {
	*area = ap -> area;
	*perim = ap -> perim;
	*cnt = ap -> cnt;
	*width = ap -> width;
	return 1;
    }
    return 0;
}

/*
** Increase the total number of transistors and the total width of the
** transistors for a certain transistor (specified by 'dev_name') on the
** node with index number N_nx.
** If the device does not yet exist on this node, create a new structure.
*/
void addCntWidth (long N_nx, char *dev_name, double width)
{
    struct ap_info *ap;

    ap = getAPinfo (N_nx, dev_name);
    if (!ap) ap = createAPinfo (N_nx, dev_name);
    ap -> cnt++;
    ap -> width += width;
}

/*
** Try to find a structure on the node with index number N_nx from the
** transistor specified with 'dev_name'.
*/
static struct ap_info *getAPinfo (long N_nx, char *dev_name)
{
    struct ap_info_ref *apiref;

    apiref = nodetab[N_nx].aplist;
    while (apiref) {
    	if (!strcmp (apiref -> ap -> dev_name, dev_name)) return apiref -> ap;
	apiref = apiref -> next;
    }
    return NULL;
}

/*
** Create a structure on the node with index number N_nx for the
** transistor specified with 'dev_name'.
*/
static struct ap_info *createAPinfo (long N_nx, char *dev_name)
{
    struct ap_info_ref *apiref;

    apiref = nodetab[N_nx].aplist;
    if (apiref) {
	while (apiref -> next) apiref = apiref -> next;
        PALLOC (apiref -> next, 1, struct ap_info_ref);
	apiref = apiref -> next;
    } else {
	PALLOC (nodetab[N_nx].aplist, 1, struct ap_info_ref);
        apiref = nodetab[N_nx].aplist;
    }

    apiref -> next = NULL;
    PALLOC (apiref -> ap, 1, struct ap_info);
    apiref -> ap -> dev_name = newStringSpace (dev_name);
    apiref -> ap -> area = 0;
    apiref -> ap -> perim = 0;
    apiref -> ap -> cnt = 0;
    apiref -> ap -> width = 0;
    return apiref -> ap;
}
