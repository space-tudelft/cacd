/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Wayne A. Christopher, U. C. Berkeley CAD Group
**********/

/*
 * Routines to do complex mathematical functions. These routines require
 * the -lm libraries. We sacrifice a lot of space to be able
 * to avoid having to do a seperate call for every vector element,
 * but it pays off in time savings.  These routines should never
 * allow FPE's to happen.
 *
 * Complex functions are called as follows:
 *  cx_something(data, type, length, &newlength, &newtype),
 *  and return a char * that is cast to complex or double.
 */

#include "spice.h"
#include "cpdefs.h"
#include "ftedefs.h"
#include "ftedata.h"
#include "ftecmath.h"

char * cx_tan(char *data, int type, int length, int *newlength, int *newtype)
{
    int i;

    *newlength = length;
    if (type == VF_COMPLEX) {
	double u, us, uc;
	double v, vs, vc;
	complex *cc = (complex *) data;
	complex * c = alloc_c(length);
	*newtype = VF_COMPLEX;
        for (i = 0; i < length; i++) {
	    u = degtorad(realpart(&cc[i]));
	    v = degtorad(imagpart(&cc[i]));
	    us = sin(u);
	    uc = cos(u);
	    vc = cosh(v);
	    vs = sinh(v);
	    rcheck(uc * vc, "tan");
	    rcheck(us * vs, "tan");
	    cdiv(us * vc, uc * vs, uc * vc, us * vs, realpart(&c[i]), imagpart(&c[i]));
        }
        return ((char *) c);
    } else {
	double u, uc;
	double *dd = (double *) data;
	double * d = alloc_d(length);
	*newtype = VF_REAL;
        for (i = 0; i < length; i++) {
	    u = degtorad(dd[i]);
	    uc = cos(u);
	    rcheck(uc != 0, "tan");
	    d[i] = sin(u) / uc;
        }
        return ((char *) d);
    }
}

char * cx_atan(char *data, int type, int length, int *newlength, int *newtype)
{
    double *d;
    int i;

    d = alloc_d(length);
    *newtype = VF_REAL;
    *newlength = length;
    if (type == VF_COMPLEX) {
	complex *cc = (complex *) data;
        for (i = 0; i < length; i++)
            d[i] = radtodeg(atan(realpart(&cc[i])));
    } else {
	double *dd = (double *) data;
        for (i = 0; i < length; i++)
            d[i] = radtodeg(atan(dd[i]));
    }
    return ((char *) d);
}

/* Normalize the data so that the magnitude of the greatest value is 1. */

char * cx_norm(char *data, int type, int length, int *newlength, int *newtype)
{
    double largest = 0.0;
    int i;

    *newlength = length;
    if (type == VF_COMPLEX) {
	complex *cc = (complex *) data;
	complex * c = alloc_c(length);
	*newtype = VF_COMPLEX;
        for (i = 0; i < length; i++)
            if (cmag(&cc[i]) > largest)
                largest = cmag(&cc[i]);
	if (largest > 0.0) {
	    for (i = 0; i < length; i++) {
		realpart(&c[i]) = realpart(&cc[i]) / largest;
		imagpart(&c[i]) = imagpart(&cc[i]) / largest;
	    }
	    return ((char *) c);
	}
    } else {
	double *dd = (double *) data;
	double * d = alloc_d(length);
	*newtype = VF_REAL;
        for (i = 0; i < length; i++)
            if (FTEcabs(dd[i]) > largest)
                largest = FTEcabs(dd[i]);
	if (largest > 0.0) {
	    for (i = 0; i < length; i++)
		d[i] = dd[i] / largest;
	    return ((char *) d);
	}
    }
    fprintf(cp_err, "Error: can't normalize a 0 vector\n");
    return (NULL);
}

char * cx_uminus(char *data, int type, int length, int *newlength, int *newtype)
{
    int i;

    *newlength = length;
    if (type == VF_COMPLEX) {
	complex *cc = (complex *) data;
	complex * c = alloc_c(length);
	*newtype = VF_COMPLEX;
        for (i = 0; i < length; i++) {
            realpart(&c[i]) = - realpart(&cc[i]);
            imagpart(&c[i]) = - imagpart(&cc[i]);
        }
        return ((char *) c);
    } else {
	double *dd = (double *) data;
	double * d = alloc_d(length);
	*newtype = VF_REAL;
        for (i = 0; i < length; i++)
            d[i] = - dd[i];
        return ((char *) d);
    }
}

char * cx_rnd(char *data, int type, int length, int *newlength, int *newtype)
{
    int i, j, k;

    *newlength = length;
    if (type == VF_COMPLEX) {
	complex *cc = (complex *) data;
	complex * c = alloc_c(length);
	*newtype = VF_COMPLEX;
        for (i = 0; i < length; i++) {
            j = floor(realpart(&cc[i]));
            k = floor(imagpart(&cc[i]));
            realpart(&c[i]) = j ? random() % j : 0;
            imagpart(&c[i]) = k ? random() % k : 0;
        }
        return ((char *) c);
    } else {
	double *dd = (double *) data;
	double * d = alloc_d(length);
	*newtype = VF_REAL;
        for (i = 0; i < length; i++) {
            j = floor(dd[i]);
            d[i] = j ? random() % j : 0;
        }
        return ((char *) d);
    }
}

/* Compute the mean of a vector. */

char * cx_mean(char *data, int type, int length, int *newlength, int *newtype)
{
    complex *c;
    double *d;
    complex *cc = (complex *) data;
    double *dd = (double *) data;
    int i;

    *newlength = 1;
    rcheck(length > 0, "mean");
    if (type == VF_REAL) {
        d = alloc_d(1);
        *newtype = VF_REAL;
        for (i = 0; i < length; i++)
            *d += dd[i];
        *d /= length;
        return ((char *) d);
    } else {
        c = alloc_c(1);
        *newtype = VF_COMPLEX;
        for (i = 0; i < length; i++) {
            realpart(c) += realpart(cc + i);
            imagpart(c) += imagpart(cc + i);
        }
        realpart(c) /= length;
        imagpart(c) /= length;
        return ((char *) c);
    }
}

char * cx_length(char *data, int type, int length, int *newlength, int *newtype)
{
    double *d;

    *newlength = 1;
    *newtype = VF_REAL;
    d = alloc_d(1);
    *d = length;
    return ((char *) d);
}

/* Return a vector from 0 to the magnitude of the argument. Length of the
 * argument is irrelevent.
 */

char * cx_vector(char *data, int type, int length, int *newlength, int *newtype)
{
    complex *cc = (complex *) data;
    double *dd = (double *) data;
    int i, len;
    double *d;

    if (type == VF_REAL)
        len = FTEcabs(*dd);
    else
        len = cmag(cc);
    if (len == 0) len = 1;
    d = alloc_d(len);
    *newlength = len;
    *newtype = VF_REAL;
    for (i = 0; i < len; i++) d[i] = i;
    return ((char *) d);
}

/* Create a vector of the given length composed of all ones. */

char * cx_unitvec(char *data, int type, int length, int *newlength, int *newtype)
{
    complex *cc = (complex *) data;
    double *dd = (double *) data;
    int i, len;
    double *d;

    if (type == VF_REAL)
        len = FTEcabs(*dd);
    else
        len = cmag(cc);
    if (len == 0) len = 1;
    d = alloc_d(len);
    *newlength = len;
    *newtype = VF_REAL;
    for (i = 0; i < len; i++) d[i] = 1;
    return ((char *) d);
}

/* Calling methods for these functions are:
 *  cx_something(data1, data2, datatype1, datatype2, length)
 * The length of the two data vectors is always the same, and is the length
 * of the result. The result type is complex if one of the args is complex.
 */

char * cx_plus(char *data1, char *data2, int datatype1, int datatype2, int length)
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i;

    if (datatype1 == VF_REAL && datatype2 == VF_REAL) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) d[i] = dd1[i] + dd2[i];
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            realpart(&c[i]) = realpart(&c1) + realpart(&c2);
            imagpart(&c[i]) = imagpart(&c1) + imagpart(&c2);
        }
        return ((char *) c);
    }
}

char * cx_minus(char *data1, char *data2, int datatype1, int datatype2, int length)
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i;

    if (datatype1 == VF_REAL && datatype2 == VF_REAL) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) d[i] = dd1[i] - dd2[i];
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            realpart(&c[i]) = realpart(&c1) - realpart(&c2);
            imagpart(&c[i]) = imagpart(&c1) - imagpart(&c2);
        }
        return ((char *) c);
    }
}

char * cx_times(char *data1, char *data2, int datatype1, int datatype2, int length)
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i;

    if (datatype1 == VF_REAL && datatype2 == VF_REAL) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) d[i] = dd1[i] * dd2[i];
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            realpart(&c[i]) = realpart(&c1) * realpart(&c2)
                - imagpart(&c1) * imagpart(&c2);
            imagpart(&c[i]) = imagpart(&c1) * realpart(&c2)
                + realpart(&c1) * imagpart(&c2);
        }
        return ((char *) c);
    }
}

char * cx_mod(char *data1, char *data2, int datatype1, int datatype2, int length)
{
    double *dd1 = (double *) data1;
    double *dd2 = (double *) data2;
    double *d;
    complex *cc1 = (complex *) data1;
    complex *cc2 = (complex *) data2;
    complex *c, c1, c2;
    int i, r1, r2, i1, i2, r3, i3;

    if (datatype1 == VF_REAL && datatype2 == VF_REAL) {
        d = alloc_d(length);
        for (i = 0; i < length; i++) {
            r1 = floor(FTEcabs(dd1[i]));
            rcheck(r1 > 0, "mod");
            r2 = floor(FTEcabs(dd2[i]));
            rcheck(r2 > 0, "mod");
            r3 = r1 % r2;
            d[i] = (double) r3;
        }
        return ((char *) d);
    } else {
        c = alloc_c(length);
        for (i = 0; i < length; i++) {
            if (datatype1 == VF_REAL) {
                realpart(&c1) = dd1[i];
                imagpart(&c1) = 0.0;
            } else {
                realpart(&c1) = realpart(&cc1[i]);
                imagpart(&c1) = imagpart(&cc1[i]);
            }
            if (datatype2 == VF_REAL) {
                realpart(&c2) = dd2[i];
                imagpart(&c2) = 0.0;
            } else {
                realpart(&c2) = realpart(&cc2[i]);
                imagpart(&c2) = imagpart(&cc2[i]);
            }
            r1 = floor(FTEcabs(realpart(&c1)));
            rcheck(r1 > 0, "mod");
            r2 = floor(FTEcabs(realpart(&c2)));
            rcheck(r2 > 0, "mod");
            i1 = floor(FTEcabs(imagpart(&c1)));
            rcheck(i1 > 0, "mod");
            i2 = floor(FTEcabs(imagpart(&c2)));
            rcheck(i2 > 0, "mod");
            r3 = r1 % r2;
            i3 = i1 % i2;
            realpart(&c[i]) = (double) r3;
            imagpart(&c[i]) = (double) i3;
        }
        return ((char *) c);
    }
}
