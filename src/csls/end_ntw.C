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

extern Dictionary *ntw_dict;
extern Dictionary *sym_dict;
extern Dictionary *inst_dict;

extern Network *curr_ntw;

extern DM_CELL *dmkey;
extern DM_STREAM *dsp_term;
extern DM_STREAM *dsp_mc;
extern DM_STREAM *dsp_net;

int end_ntw (Network *ntw, int ext, int orig_mode)
{
    NetworkInstance *pinst;
    Netelem *pnet;
    int mode;

    if (sls_errno)
	mode = QUIT;
    else
	mode = orig_mode;

    if (!ext && !sls_errno && ntw -> netq && mode != QUIT)
	net_to_db (ntw -> netq);
    else if (ntw -> netq) {
	while(!ntw -> netq -> empty()) {
	    pnet = (Netelem *) ntw -> netq -> get();
	    if (pnet -> nmem)
		delete pnet -> nmem;
	    delete pnet;
	}
	delete ntw -> netq;
    }
    delete sym_dict;
    sym_dict = NULL;

    if (ntw->mcq) {
        while(!ntw->mcq -> empty()) {
	    pinst = (NetworkInstance *) ntw -> mcq -> get ();
	    delete pinst;
        }
	delete ntw -> mcq;
    }
    delete inst_dict;
    inst_dict = NULL;

    curr_ntw = NULL;

    if (!ext) {
	if (dsp_term) dmCloseStream(dsp_term, mode);
	if (dsp_mc) dmCloseStream(dsp_mc, mode);
	if (dsp_net) dmCloseStream(dsp_net, mode);
	dsp_term = NULL;
	dsp_mc = NULL;
	dsp_net = NULL;

	dmCheckIn (dmkey, mode);

	if (sls_errno) {
	    sls_errno = BADNETW;
	    return(1);
	}
    }

    return(0);
}
