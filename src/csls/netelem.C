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

netelem::netelem (char *s, Stack *stack, char ntype) : lnk (ntype)
{
    name = strsav (s);
    xs = stack;
    eqv = NULL;
    nmem = NULL;
    type = (ntype == TermType) ? N_TERMINAL : N_NET;
#ifdef DMEM
    netelem_nbyte += sizeof(class netelem);
    if (netelem_nbyte > netelem_maxnbyte) netelem_maxnbyte = netelem_nbyte;
#endif
}

netelem::~netelem () {
    delete name;
#ifdef DMEM
    char_nbyte -= strlen(name) + 1;
#endif
    if(lnk::type == NetType)
    {
        if(xs)
        {
	    stackfree(xs, XELEM);
            delete xs;
        }
    }
#ifdef DMEM
    netelem_nbyte -= sizeof(class netelem);
#endif
}

void netelem::print()
{
    fprintf (stderr, "[ Netelem ] ptr = %p {\n", this);
    fprintf (stderr, "\tname =\t%s\n", name);
    fprintf (stderr, "\ttype =\t%d\n", type);
    fprintf (stderr, "\txs   =\t%p\n", xs);
    fprintf (stderr, "}\n");
}

void netelem::to_db()
{
	long lower[10], upper[10];
	int i;
	int size = xs ? xs -> length () : 0;

	strcpy (cterm.term_name, name);
	cterm.term_dim = size;

	if (size > 0) {
	    char **pxs = xs -> base ();

	    cterm.term_lower = lower;
	    cterm.term_upper = upper;

	    for (i = 0; i < size; ++i) {
		lower[i] = ((Xelem *) *pxs) -> left_bound;
		upper[i] = ((Xelem *) *pxs) -> right_bound;
		++pxs;
	    }
	}

	switch (type) {
	case T_global:   cterm.term_attribute = (char*)"global";   break;
	case T_input:    cterm.term_attribute = (char*)"input";    break;
	case T_output:   cterm.term_attribute = (char*)"output";   break;
	case T_inout:    cterm.term_attribute = (char*)"inout";    break;
	case T_tristate: cterm.term_attribute = (char*)"tristate"; break;
	default: cterm.term_attribute = 0;
	}

	dmPutDesignData(dsp_term, CIR_TERM);
}

net_ref::net_ref (class netelem *pnet, Stack *xs) : lnk (NetRefType)
{
    net = pnet;
    net_xs = xs;
    inst = NULL;
    inst_xs = NULL;
    ref_xs = NULL;
#ifdef DMEM
    net_ref_nbyte += sizeof(class net_ref);
    if (net_ref_nbyte > net_ref_maxnbyte) net_ref_maxnbyte = net_ref_nbyte;
#endif
}

net_ref::~net_ref () {
#ifdef DMEM
    net_ref_nbyte -= sizeof(class net_ref);
#endif
    if(net_xs)
    {
/*
	if(((Link *) net)->type == NetType)
	    stackfree(net_xs, XELEM);
        delete net_xs;
*/
    }

    if(ref_xs)
    {
        delete ref_xs;
    }
}
