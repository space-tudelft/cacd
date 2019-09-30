/*
 * ISC License
 *
 * Copyright (C) 1991-2018 by
 *	Ireneusz Karkowski
 *	Patrick Groeneveld
 *	Paul Stravers
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

#include "src/ocean/libseadif/sealib.h"
#include "src/ocean/madonna/partitioner/genpart.h"
#include "src/ocean/madonna/partitioner/cost.h"
#include "src/ocean/madonna/partitioner/part.h"

extern double extraplaza;
extern int highnumpart;

int netcostinfolength = 0;
int madonnaspc = 0;
NETCOSTINFO netcostinfo[MAXNETCOSTFUNCTIONS+1];

#define COSTTAB  "div.tab"
#define COSTTAB2 "seadif/div.tab"

PRIVATE int readDivTable (void);
#ifdef __cplusplus
extern "C"
#endif
int findrectangleandcostfunctions (CIRCUITPTR circuit, int *nx, int *ny,
				  CFUNC *costfunctions, int calldepth);

/* recursively partition circuit. */
void madonna_ (TOTALPPTR *total, CIRCUITPTR circuit, int calldepth)
{
    CIRINSTPTR cinst;
    TOTALPPTR  total2 = NULL;
    CFUNC costfunctions[3];
    int nx, ny;

    madonnaspc += 2;
    *total = NULL;

    /* need to know the bounding boxes of the cells */
    if (!readlayoutofchildren (SDFLAYBBX+SDFLAYPORT, circuit, 0))
    {
	err (5, "madonna_ : cannot read layout of children... bye!");
    }

    if (!findrectangleandcostfunctions (circuit, &nx, &ny, costfunctions, calldepth))
	return; /* not enough cells, not worth partitioning */

    genpart (total, circuit, nx, ny, costfunctions, 30);

 /* madonnastat (*total); */

    /* this gives us the 4 partitions as children of total->bestpart */
    for (cinst = (*total)->bestpart->cirinst; cinst; cinst = cinst->next)
    {
	madonna_ (&total2, cinst->circuit, calldepth+1);

	if (!total2 || !total2->bestpart) continue;

	/* substitute cinst->circuit for its partitioned equivalent */
	if (--cinst->circuit->linkcnt <= 0) sdfdeletecircuit (cinst->circuit, TRUE);

	cinst->circuit = total2->bestpart;
	++cinst->circuit->linkcnt;
    }
    madonnaspc -= 2;
}

int costquad (int netdistr[], int numparts)
{
    static int quadcost[16] =
    {
	0, 0, 0, 1,
	0, 1, 2, 2,
	0, 2, 1, 2,
	1, 2, 2, 3
    };
    int bit, index, j;

    if (numparts != 4) {
	fprintf (stderr, "costquad: cannot compute cost for %d partitions!\n", numparts);
	dumpcore ();
    }

    index = 0;
    for (j = 0, bit = 1; j < numparts; ++j, bit <<= 1)
	if (netdistr[j]) index += bit;

    if (index >= 16 || index < 0) err (5, "costquad: internal error 657");

    return (quadcost[index]);
}

void madonnastat (TOTALPPTR total)
{
    int i;

    for (i = madonnaspc; i > 0; --i) printf (" ");

    fprintf (stderr, "(%s(%s(%s))): start=%d,best=%d,nmoves=%d(%d),temperature=%1.2f\n",
	total->bestpart->name, total->bestpart->function->name,
	total->bestpart->function->library->name,
	total->strtnetcost, total->bestnetcost, total->nmoves,
	total->bestmove, total->temperature);
}

int cost3x3 (int netdistr[], int numparts)
{
    if (numparts != 9) {
	fprintf (stderr, "\ncost3x3: numparts must be 9, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 9, 3));
}

int cost4x4 (int netdistr[], int numparts)
{
    if (numparts != 16) {
	fprintf (stderr, "\ncost4x4: numparts must be 16, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 16, 4));
}

int cost5x4 (int netdistr[], int numparts)
{
    if (numparts != 20) {
	fprintf (stderr, "\ncost5x4: numparts must be 20, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 20, 5));
}

int cost6x4 (int netdistr[], int numparts)
{
    if (numparts != 24) {
	fprintf (stderr, "\ncost6x4: numparts must be 24, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 24, 6));
}

int cost8x4 (int netdistr[], int numparts)
{
    if (numparts != 32) {
	fprintf (stderr, "\ncost8x4: numparts must be 32, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 32, 8));
}

int cost16x4 (int netdistr[], int numparts)
{
    if (numparts != 64) {
	fprintf (stderr, "\ncost16x4: numparts must be 64, not %d\n", numparts);
	dumpcore ();
    }
    return (gencost (netdistr, 64, 16));
}

/* Compute cost of a net-distribution.  Based on the size of the
 * enclosing rectangle and the number of sub-areas visited within that rectangle.
 */
#define BIG    10000
#define SMALL -10000

int gencost (int netdistr[], register int total, register int hor)
{
    register int mh; /* mh == (i+1) % hor */
    register int v;
    int i, minh = BIG, minv = BIG, maxh = SMALL, maxv = SMALL, nsubarea = 0, cost;

    for (mh = 1, v = 1, i = 0; i < total; ++i)
    {
	if (netdistr[i] > 0) {
	    ++nsubarea;
	    if (v  < minv) minv = v;
	    if (v  > maxv) maxv = v;
	    if (mh < minh) minh = mh;
	    if (mh > maxh) maxh = mh;
	}
	else if (netdistr[i] < 0) {
	    fprintf (stderr, "gencost: netdistr[%d] contains negative number: %d\n", i, netdistr[i]);
	    dumpcore ();
	}

	if (++mh > hor) { ++v; mh = 1; }
    }
    cost = (maxv - minv + 1) * (maxh - minh + 1) + nsubarea;
    return (cost > 0 ? cost : 0);
}

/* Initialize the table netcostinfo[] (see genpart.h) containing all
 * vital information about the available net cost functions and area
 * partitionings. The extern variable ``highnumpart'' determines the
 * upper limit on the number of subareas. On exit, this function sets
 * the global netcostinfolength to the length of netcostinfo[].
 */
int initnetcostinfo ()
{
    int j = 0;

    netcostinfo[j].ncells = 7;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 4; netcostinfo[j].row = 2;netcostinfo[j].clm = 2;
    if (highnumpart <= 4) goto weg;

    ++j;
    netcostinfo[j].ncells = 20;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 9; netcostinfo[j].row = 3; netcostinfo[j].clm = 3;
    if (highnumpart <= 9) goto weg;

    ++j;
    netcostinfo[j].ncells = 30;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 16; netcostinfo[j].row = 4; netcostinfo[j].clm = 4;
    if (highnumpart <= 16) goto weg;

    ++j;
    netcostinfo[j].ncells = 40;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 20; netcostinfo[j].row = 4; netcostinfo[j].clm = 5;
    if (highnumpart <= 20) goto weg;

    ++j;
    netcostinfo[j].ncells = 48;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 24; netcostinfo[j].row = 4; netcostinfo[j].clm = 6;
    if (highnumpart <= 24) goto weg;

    ++j;
    netcostinfo[j].ncells = 64;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 32; netcostinfo[j].row = 4; netcostinfo[j].clm = 8;
    if (highnumpart <= 32) goto weg;

    ++j;
    netcostinfo[j].ncells = 100;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 64; netcostinfo[j].row = 4; netcostinfo[j].clm = 16;
    if (highnumpart <= 64) goto weg;

    ++j;
    netcostinfo[j].ncells = 158;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 128; netcostinfo[j].row = 8; netcostinfo[j].clm = 16;
    if (highnumpart <= 128) goto weg;

    ++j;
    netcostinfo[j].ncells = 400;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 256; netcostinfo[j].row = 8; netcostinfo[j].clm = 32;
    if (highnumpart <= 256) goto weg;

    ++j;
    netcostinfo[j].ncells = 640;
    netcostinfo[j].costfunction[COSTSTATE] = (CFUNC)netstatecost;
    netcostinfo[j].costfunction[COSTVEC] = (CFUNC)netstatecost2;
    netcostinfo[j].numparts = 512; netcostinfo[j].row = 8; netcostinfo[j].clm = 64;
    if (highnumpart <= 512) goto weg;

weg:
    ++j;
    netcostinfo[j].ncells = 0; /* end of netcostinfo */
    netcostinfolength = j;
    return TRUE;
}

/*
 * Reads cost configuration from file div.tab if it exists. On error returns 1.
 */
int readDivTable ()
{
    FILE *in;
    int ncells; /* minimum #cells needed for this #numparts */
    int numparts, row, clm; /* numparts=row*clm, size and topology */
    int r, i = 0;

    if (!(in = fopen (COSTTAB, "rt")))
    if (!(in = fopen (COSTTAB2, "rt"))) {
	fprintf (stderr, "\n#### Couldn't find %s or %s - using internal defaults.\n", COSTTAB, COSTTAB2);
	return 1;
    }

    while (!feof (in))
    {
	if ((r = fscanf (in, "%d %d %d %d", &numparts, &clm, &row, &ncells)) != 4
		|| i >= MAXNETCOSTFUNCTIONS-1 || clm < 0 || row < 0 || ncells < 0)
	{
	    if (r == 0) break;
	    else {
		fclose (in);
		fprintf (stderr, "Error in \"%s\" file.\n", COSTTAB);
		return 1;
	    }
	}

	if (numparts <= highnumpart)
	{
	    netcostinfo[i].ncells = ncells;
	    netcostinfo[i].costfunction[COSTSTATE] = (CFUNC)netstatecost;
	    netcostinfo[i].costfunction[COSTVEC] = (CFUNC)netstatecost2;
	    netcostinfo[i].numparts = numparts;
	    netcostinfo[i].row = row;
	    netcostinfo[i].clm = clm;
	    i++;
	}
	else
	    break;
    }
    netcostinfo[i].ncells = 0; /* end of netcostinfo */
    netcostinfolength = i;

    fclose (in);
    return 0;
}
