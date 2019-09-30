static char *rcsid = "$Id: param.c,v 1.1 2018/04/30 12:17:42 simon Exp $";
/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	T. Vogel
 *	A.J. van Genderen
 *	S. de Graaf
 *	A.J. van der Hoeven
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
 * Contains a number of functions to access and
 * update the parameter list of a network.
 * A parameter list is implemented as a single linked list.
 */
#include "src/match/head.h"
#include "src/match/proto.h"

/*	VARIABLES:
 */
Import network * nom_netw;
Import network * act_netw;

/*
 * Set parameter of the specified device to specified value.
 * If 'par' is not present, it is added to the parameter list.
 * If value is equal NULL, no value is assigned.
 * If value is not equal NULL, it must be allocated in memory!
 */
Public void set_par (object *dev, string par, string value)
{
    bucket *ThiS;

    Assert (dev && par);

    for (ThiS = dev -> par_list; ThiS; ThiS = ThiS -> next) {
	if (strcmp (ThiS -> key, par) == 0) { /* par present */
	    if (ThiS -> data) deletemem (ThiS -> data);
	    ThiS -> data = value;
	    return;
	}
    }

    ThiS = mk_a_bucket ();
    ThiS -> key  = par;
    ThiS -> data = value;
    ThiS -> next = dev -> par_list;
    dev -> par_list = ThiS;
}

Public void set_parpair (object *dev, char *attr, char *par)
{
    int str_l, i, c;

    if ((str_l = strlen (par)) > 0) { /* search 'par' */
	for (; *attr; ++attr) {
	    if (*attr != ';' && *attr != '=' && *attr != ' ') {
		for (i = 0; i < str_l; ++i) {
		    if (attr[i] != par[i]) break;
		}
		attr += i;
		if (i == str_l) { /* found par? */
		    if (*attr == '=' || *attr == ' ' || *attr == ';' || !*attr) break;
		}
		while (*attr && *attr != ';') ++attr;
		if (!*attr) break;
	    }
	}
	if (*attr) { /* get value */
	    while (*attr == '=' || *attr == ' ') ++attr; /* skip */
	    if (*attr != ';' && *attr) {
		i = 0;
		while (attr[i] != ';' && attr[i] && attr[i] != ' ') ++i;
		c = attr[i];
		attr[i] = '\0';
		set_par (dev, par, newmem (attr));
		attr[i] = c;
	    }
	}
    }
}

/*
 * Get the (long) value of parameter 'par' from attribute 'attr'.
 */
Public int get_ival (char *attr, char *par, long *ip)
{
    int str_l, i = -1;

    if ((str_l = strlen (par)) > 0) { /* search 'par' */
	for (; *attr; ++attr) {
	    if (*attr != ';' && *attr != '=' && *attr != ' ') {
		for (i = 0; i < str_l; ++i) {
		    if (attr[i] != par[i]) break;
		}
		attr += i;
		if (i == str_l) { /* found par? */
		    if (*attr == '=' || *attr == ' ' || *attr == ';' || !*attr) break;
		}
		while (*attr && *attr != ';') ++attr;
		if (!*attr) break;
	    }
	}
	i = -1;
	if (*attr) { /* get value */
	    while (*attr == '=' || *attr == ' ') ++attr; /* skip */
	    if (*attr != ';' && *attr) {
		*ip = atol (attr);
		i = 0;
	    }
	}
    }
    return (i);
}

/*
 * Returns a pointer to the value of the
 * named parameter of the specified device.
 * NULL is returned if the parameter does not exist.
 */
Public string get_par (object *dev, string par)
{
    bucket *ThiS;

    Assert (dev && par);

    for (ThiS = dev -> par_list; ThiS; ThiS = ThiS -> next) {
	if (strcmp (ThiS -> key, par) == 0) return (ThiS -> data);
    }
    return (NULL);
}
