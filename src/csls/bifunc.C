/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
 *	A.J. van Genderen
 *	S. de Graaf
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

#include "src/csls/sys_incl.h"
#include "src/csls/class.h"
#include "src/csls/mkdbdefs.h"
#include "src/csls/mkdbincl.h"

class built_in_func {
public:
	char *name;
	Network *ntw;
};

typedef class built_in_func Bifunc;

Bifunc  *bifunc;

int bif_nbr;

void init_bifs ()
{
	Stack *pxs;
	Queue *bif_termq;
	Netelem *fterm;
	Xelem *pxelem;

	bif_termq = new Queue (QueueType);
	pxs = new Stack (1);
	pxelem = new Xelem (0, 0);
	pxs -> push ((char *) pxelem);
	fterm = new Netelem ((char*)"i", pxs, TermType);
	bif_termq -> put ((Link *) fterm);

	fterm = new Netelem ((char*)"o", 0, TermType);
	bif_termq -> put ((Link *) fterm);

	bifunc = new Bifunc [6];
        bif_nbr = 6;

	bifunc[0].name= (char*)"and";
	bifunc[1].name= (char*)"exor";
	bifunc[2].name= (char*)"nand";
	bifunc[3].name= (char*)"nor";
	bifunc[4].name= (char*)"or";
	bifunc[5].name= (char*)"invert";

	bifunc[0].ntw = new Network ((char*)"and");
	bifunc[1].ntw = new Network ((char*)"exor");
	bifunc[2].ntw = new Network ((char*)"nand");
	bifunc[3].ntw = new Network ((char*)"nor");
	bifunc[4].ntw = new Network ((char*)"or");
	bifunc[5].ntw = new Network ((char*)"invert");

	bifunc[0].ntw -> termq = bif_termq;
	bifunc[1].ntw -> termq = bif_termq;
	bifunc[2].ntw -> termq = bif_termq;
	bifunc[3].ntw -> termq = bif_termq;
	bifunc[4].ntw -> termq = bif_termq;
	bifunc[5].ntw -> termq = bif_termq;
}

Network *is_bifunc (char *name)
{
     int i;
     Bifunc * pbif = bifunc;

     for (i = 0; i < bif_nbr; i++)
     {
	 if (strcmp (name, pbif -> name) == 0) return (pbif -> ntw);
	 pbif++;
     }

     return (0);
}
