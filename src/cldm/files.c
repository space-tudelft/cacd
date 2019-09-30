/*
 * ISC License
 *
 * Copyright (C) 1983-2018 by
 *	J. Annevelink
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

#include "src/cldm/extern.h"

void open_files ()
{
    struct stat statBuf;

    fp_box  = dmOpenStream (mod_key, "box" , "w");
    fp_info = dmOpenStream (mod_key, "info", "w");
    fp_mc   = dmOpenStream (mod_key, "mc"  , "w");
    fp_nor  = dmOpenStream (mod_key, "nor" , "w");
    fp_term = dmOpenStream (mod_key, "term", "w");
    fp_anno = dmOpenStream (mod_key, "annotations", "w");
    ganno.type = GA_FORMAT;
    ganno.o.format.fmajor = ganno.o.format.fminor = 1;
    dmPutDesignData (fp_anno, GEO_ANNOTATE);
    if (dmStat (mod_key, "torname", &statBuf) != -1) {
	dmUnlink (mod_key, "torname");
    }
}

void close_files ()
{
    dmCloseStream (fp_box, COMPLETE);
    dmCloseStream (fp_info, COMPLETE);
    dmCloseStream (fp_mc, COMPLETE);
    dmCloseStream (fp_nor, COMPLETE);
    dmCloseStream (fp_term, COMPLETE);
    dmCloseStream (fp_anno, COMPLETE);
}
