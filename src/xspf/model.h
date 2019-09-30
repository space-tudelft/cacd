
/*
 * ISC License
 *
 * Copyright (C) 1997-2013 by
 *	Frederik Beeftink
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

struct model_par {
    char *name;
    char *value;
    char separator;
    short var_in_val;        /* variables in value */
    struct model_par *next;
};

struct lib_model {
    char *name;
    char *orig_name;
    char *type_name;
    char *typical[RANGE_NUMBER];
    char *lower[RANGE_NUMBER];
    char *upper[RANGE_NUMBER];
    struct model_par *par_list;
    struct cell_par  *pars;
    char *specified;     /* model specified in library, set to library type */
    short var_in_par;    /* variables in parameters */
    short model_type; /* 1=exact, 2=scalable, 3=subst, 4=default */
    struct lib_model *next;
};