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

#ifdef SLS
#include "src/sls/extern.h"
#else
#include "src/sls_exp/extern.h"
#endif /* SLS */

static int vf_checkb (int *xvector, short *ind, int ends);

int vfunc (FUNCTION *f, char *varname, int *xvector, NODE_REF_LIST *ref)
{
    int i;
    int cnt;
    int fix;
    int frx;
    int fox;
    int fsx;
    int val;
    int size;
    int extra;
    FUNCVAR * fvar;

    fix = f -> fix + 1;
    frx = f -> frx + 1;
    fox = f -> fox + 1;
    fsx = f -> fsx + FD[ f -> type ].offsx;
    i = FD[ f -> type ].fvx;
    cnt = 0;
    while (cnt < FD[ f -> type ].fvx_cnt) {
	size = 0;
        fvar = &FV[i++];
        cnt++;
        switch (fvar -> type) {
            case INPUT_T :
                if (strcmp (fvar -> name, varname) == 0) {
                    if ((val = vf_checkb (xvector, fvar -> ind, 0)) < 0)
                        return (val);
                    fix += val;
                    ref -> nx = FI[ fix ];
                    ref -> xptr = &FI[ fix ];
                    return (NODETYPE);
                }
                else {
                    if (fvar -> ind[0] == 0)
                        fix += 1;
                    else if (fvar -> ind[1] == 0)
                        fix += fvar -> ind[0];
                    else
                        fix += fvar -> ind[0] * fvar -> ind[1];
                }
                break;
            case INPUT_R :
                if (strcmp (fvar -> name, varname) == 0) {
                    if ((val = vf_checkb (xvector, fvar -> ind, 0)) < 0)
                        return (val);
                    frx += val;
                    ref -> nx = FR[ frx ];
                    ref -> xptr = &FR[ frx ];
                    return (NODETYPE);
                }
                else {
                    if (fvar -> ind[0] == 0)
                        frx += 1;
                    else if (fvar -> ind[1] == 0)
                        frx += fvar -> ind[0];
                    else
                        frx += fvar -> ind[0] * fvar -> ind[1];
                }
                break;
            case INOUT :
            case OUTPUT :
                if (strcmp (fvar -> name, varname) == 0) {
                    if ((val = vf_checkb (xvector, fvar -> ind, 0)) < 0)
                        return (val);
                    fox += val;
                    ref -> nx = FO[ fox ].x;
                    ref -> xptr = &FO[ fox ].x;
                    return (NODETYPE);
                }
                else {
                    if (fvar -> ind[0] == 0)
                        fox += 1;
                    else if (fvar -> ind[1] == 0)
                        fox += fvar -> ind[0];
                    else
                        fox += fvar -> ind[0] * fvar -> ind[1];
                }
                break;
            case CHAR :
                size = 1;
            case INTEGER :
                if (size == 0) size = sizeof (int);
            case FLOAT :
                if (size == 0) size = sizeof (float);
            case DOUBLE :
                if (size == 0) size = sizeof (double);
                if (size != 1) {
                    extra = 0;         /* it's not a CHAR */
                    fsx += size;
                    if (fsx % size != 0)
                        fsx += size - fsx % size;
                }
                else
                    extra = 1;
                if (strcmp (fvar -> name, varname) == 0) {
                    if ((val = vf_checkb (xvector, fvar -> ind, extra)) < 0)
                        return (val);
                    fsx += size * val;
                    ref -> nx = FS[ fsx ];
                    ref -> xptr = (int *)&FS[ fsx ];
                    switch (fvar -> type) {
                        case CHAR :
                            return (CHARTYPE);
                        case INTEGER:
                            return (INTEGERTYPE);
                        case FLOAT :
                            return (FLOATTYPE);
                        case DOUBLE :
                            return (DOUBLETYPE);
                    }
                }
                else {
                    if (fvar -> ind[0] == 0)
                        fsx += size;
                    else if (fvar -> ind[1] == 0)
                        fsx += size * (fvar -> ind[0] + extra);
                    else
                        fsx += size * fvar -> ind[0] * (fvar -> ind[1] + extra);
                }
                break;
        }
    }

    return (NAMENEG);
}

static int vf_checkb (int *xvector, short *ind, int ends)
{
    int ret;

    switch (xvector[0]) {
        case 0 :
            if (ind[0] > 0)
                ret = REFIMIS;
            else
                ret = 0;
            break;
        case 1 :
            if (ind[0] <= 0)
                ret = REFINEG;
            else if (ind[1] > 0)
                ret = REFIERR;
            else {
                if (xvector[1] < 0 || xvector[1] >= ind[0])
                    ret = REFIERR;
                else
                    ret = xvector[1];
            }
            break;
        case 2 :
            if (ind[0] <= 0)
                ret = REFINEG;
            else if (ind[1] <= 0)
                ret = REFIERR;
            else {
                if (xvector[1] < 0 || xvector[1] >= ind[0]
                    || xvector[2] < 0 || xvector[2] >= ind[1])
                    ret = REFIERR;
                else
                    ret = xvector[1] * (ind[1] + ends) + xvector[2];
            }
            break;
        default :
            if (ind[0] < 0)
                ret = REFINEG;
            else
                ret = REFIERR;
    }

    return (ret);
}
