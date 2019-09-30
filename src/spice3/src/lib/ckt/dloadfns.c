/**********
Copyright 1990 Regents of the University of California. All rights reserved.
Author: 1988 Jaijeet S Roychowdhury
**********/

#include "spice.h"
#include "distodef.h"

/*
 * all subFns are local to this file so they need not be renamed to
 * the awful 7 letter standard; however, for reasons of uniformity,
 * they are being renamed, losing all readability in the process.
 * the renaming convention is as follows:
 *   example: 3v3F1m2
 * 3v => 3 variable term xyz
 * 2F1m2 => Two F1 minus F2
 * therefore the old name would be : S3v3F1minusF2
 * for the imaginary sub functions, the v is replaced by an i
 */

/* 5 arguments */
double S2v2F1(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y)
{
    return(cxy * (r1h1x * r1h1y - i1h1x * i1h1y));
}

/* 5 arguments */
double S2i2F1(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y)
{
    return(cxy * (r1h1x * i1h1y + i1h1x * r1h1y));
}

/* 9 arguments */
double S2v3F1(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r2h11x, double i2h11x, double r2h11y, double i2h11y)
{
    return(cxy * (r1h1x * r2h11y - i1h1x * i2h11y + r1h1y * r2h11x - i1h1y * i2h11x));
}

/* 9 arguments */
double S2i3F1(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r2h11x, double i2h11x, double r2h11y, double i2h11y)
{
    return(cxy * (r1h1x * i2h11y + i1h1x * r2h11y + r1h1y * i2h11x + i1h1y * r2h11x));
}

/* 9 arguments */
double S2vF12(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h2x, double i1h2x, double r1h2y, double i1h2y)
{
    return(cxy * (r1h1x * r1h2y - i1h1x * i1h2y + r1h1y * r1h2x - i1h1y * i1h2x));
}

/* 9 arguments */
double S2iF12(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h2x, double i1h2x, double r1h2y, double i1h2y)
{
    return(cxy * (r1h1x * i1h2y + i1h1x * r1h2y + r1h1y * i1h2x + i1h1y * r1h2x));
}

/* 17 arguments */
double S2v2F12(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r2h11x, double i2h11x, double r2h11y, double i2h11y, double h2f1f2x, double ih2f1f2x, double h2f1f2y, double ih2f1f2y)
{
    return(cxy * (2 * (r1h1x * h2f1f2y - i1h1x * ih2f1f2y + r1h1y * h2f1f2x - i1h1y * ih2f1f2x) + r1h2x * r2h11y - i1h2x * i2h11y + r1h2y * r2h11x - i1h2y * i2h11x));
}

/* 17 arguments */
double S2i2F12(double cxy, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r2h11x, double i2h11x, double r2h11y, double i2h11y, double h2f1f2x, double ih2f1f2x, double h2f1f2y, double ih2f1f2y)
{
    return(cxy * (2 * (r1h1x * ih2f1f2y + i1h1x * h2f1f2y + r1h1y * ih2f1f2x + i1h1y * h2f1f2x) + r1h2x * i2h11y + i1h2x * r2h11y + r1h2y * i2h11x + i1h2y * r2h11x));
}

/* 7 arguments */
double S3v3F1(double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z)
{
    return(cxyz * ((r1h1x * r1h1y - i1h1x * i1h1y) * r1h1z - (i1h1x * r1h1y + r1h1x * i1h1y) * i1h1z));
}

/* 7 arguments */
double S3i3F1(double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z)
{
    return(cxyz * ((r1h1x * r1h1y - i1h1x * i1h1y) * i1h1z + (i1h1x * r1h1y + r1h1x * i1h1y) * r1h1z));
}

/* 13 arguments */
double S3v2F12(double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r1h2z, double i1h2z)
{
    return(cxyz * ((r1h1x * r1h1y - i1h1x * i1h1y) * r1h2z - (i1h1x * r1h1y + r1h1x * i1h1y) * i1h2z
		 + (r1h1x * r1h1z - i1h1x * i1h1z) * r1h2y - (i1h1x * r1h1z + r1h1x * i1h1z) * i1h2y
		 + (r1h1z * r1h1y - i1h1z * i1h1y) * r1h2x - (i1h1z * r1h1y + r1h1z * i1h1y) * i1h2x));
}

/* 13 arguments */
double S3i2F12(double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r1h2z, double i1h2z)
{
    return(cxyz * ((r1h1x * r1h1y - i1h1x * i1h1y) * i1h2z + (i1h1x * r1h1y + r1h1x * i1h1y) * r1h2z
		 + (r1h1x * r1h1z - i1h1x * i1h1z) * i1h2y + (i1h1x * r1h1z + r1h1x * i1h1z) * r1h2y
		 + (r1h1z * r1h1y - i1h1z * i1h1y) * i1h2x + (i1h1z * r1h1y + r1h1z * i1h1y) * r1h2x));
}

/* the load functions */
/* also renamed... */

/* 12 variables */
double DFn2F1(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z)
{
    return(S2v2F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x)
	 + S2v2F1(cyy, r1h1y, i1h1y, r1h1y, i1h1y)
	 + S2v2F1(czz, r1h1z, i1h1z, r1h1z, i1h1z)
	 + S2v2F1(cxy, r1h1x, i1h1x, r1h1y, i1h1y)
	 + S2v2F1(cyz, r1h1y, i1h1y, r1h1z, i1h1z)
	 + S2v2F1(cxz, r1h1x, i1h1x, r1h1z, i1h1z));
}

/* 12 variables */
double DFi2F1(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z)
{
    return(S2i2F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x)
	 + S2i2F1(cyy, r1h1y, i1h1y, r1h1y, i1h1y)
	 + S2i2F1(czz, r1h1z, i1h1z, r1h1z, i1h1z)
	 + S2i2F1(cxy, r1h1x, i1h1x, r1h1y, i1h1y)
	 + S2i2F1(cyz, r1h1y, i1h1y, r1h1z, i1h1z)
	 + S2i2F1(cxz, r1h1x, i1h1x, r1h1z, i1h1z));
}

/* 28 args - 16 + 6 + 6 */
double DFn3F1(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double cxxx, double cyyy, double czzz, double cxxy, double cxxz, double cxyy, double cyyz, double cxzz, double cyzz, double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r2h11x, double i2h11x, double r2h11y, double i2h11y, double r2h11z, double i2h11z)
{
double tmp;
    tmp = S2v3F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r2h11x, i2h11x, r2h11x, i2h11x)
	+ S2v3F1(cyy, r1h1y, i1h1y, r1h1y, i1h1y, r2h11y, i2h11y, r2h11y, i2h11y)
	+ S2v3F1(czz, r1h1z, i1h1z, r1h1z, i1h1z, r2h11z, i2h11z, r2h11z, i2h11z)
	+ S2v3F1(cxy, r1h1x, i1h1x, r1h1y, i1h1y, r2h11x, i2h11x, r2h11y, i2h11y)
	+ S2v3F1(cyz, r1h1y, i1h1y, r1h1z, i1h1z, r2h11y, i2h11y, r2h11z, i2h11z)
	+ S2v3F1(cxz, r1h1x, i1h1x, r1h1z, i1h1z, r2h11x, i2h11x, r2h11z, i2h11z)
	+ S3v3F1(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x)
	+ S3v3F1(cyyy, r1h1y, i1h1y, r1h1y, i1h1y, r1h1y, i1h1y)
	+ S3v3F1(czzz, r1h1z, i1h1z, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3v3F1(cxxy, r1h1x, i1h1x, r1h1x, i1h1x, r1h1y, i1h1y)
	+ S3v3F1(cxxz, r1h1x, i1h1x, r1h1x, i1h1x, r1h1z, i1h1z)
	+ S3v3F1(cxyy, r1h1x, i1h1x, r1h1y, i1h1y, r1h1y, i1h1y)
	+ S3v3F1(cyyz, r1h1y, i1h1y, r1h1y, i1h1y, r1h1z, i1h1z)
	+ S3v3F1(cxzz, r1h1x, i1h1x, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3v3F1(cyzz, r1h1y, i1h1y, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3v3F1(cxyz, r1h1x, i1h1x, r1h1y, i1h1y, r1h1z, i1h1z);
    return(tmp);
}

/* 28 args - 10 + 6 + 6 */
double DFi3F1(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double cxxx, double cyyy, double czzz, double cxxy, double cxxz, double cxyy, double cyyz, double cxzz, double cyzz, double cxyz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r2h11x, double i2h11x, double r2h11y, double i2h11y, double r2h11z, double i2h11z)
{
double tmp;
    tmp = S2i3F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r2h11x, i2h11x, r2h11x, i2h11x)
	+ S2i3F1(cyy, r1h1y, i1h1y, r1h1y, i1h1y, r2h11y, i2h11y, r2h11y, i2h11y)
	+ S2i3F1(czz, r1h1z, i1h1z, r1h1z, i1h1z, r2h11z, i2h11z, r2h11z, i2h11z)
	+ S2i3F1(cxy, r1h1x, i1h1x, r1h1y, i1h1y, r2h11x, i2h11x, r2h11y, i2h11y)
	+ S2i3F1(cyz, r1h1y, i1h1y, r1h1z, i1h1z, r2h11y, i2h11y, r2h11z, i2h11z)
	+ S2i3F1(cxz, r1h1x, i1h1x, r1h1z, i1h1z, r2h11x, i2h11x, r2h11z, i2h11z)
	+ S3i3F1(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x)
	+ S3i3F1(cyyy, r1h1y, i1h1y, r1h1y, i1h1y, r1h1y, i1h1y)
	+ S3i3F1(czzz, r1h1z, i1h1z, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3i3F1(cxxy, r1h1x, i1h1x, r1h1x, i1h1x, r1h1y, i1h1y)
	+ S3i3F1(cxxz, r1h1x, i1h1x, r1h1x, i1h1x, r1h1z, i1h1z)
	+ S3i3F1(cxyy, r1h1x, i1h1x, r1h1y, i1h1y, r1h1y, i1h1y)
	+ S3i3F1(cyyz, r1h1y, i1h1y, r1h1y, i1h1y, r1h1z, i1h1z)
	+ S3i3F1(cxzz, r1h1x, i1h1x, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3i3F1(cyzz, r1h1y, i1h1y, r1h1z, i1h1z, r1h1z, i1h1z)
	+ S3i3F1(cxyz, r1h1x, i1h1x, r1h1y, i1h1y, r1h1z, i1h1z);
    return(tmp);
}

/* 18 args - 6 + 6 + 6 */
double DFnF12(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r1h2z, double i1h2z)
{
double tmp;
    tmp = S2vF12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x)
	+ S2vF12(cyy, r1h1y, i1h1y, r1h1y, i1h1y, r1h2y, i1h2y, r1h2y, i1h2y)
	+ S2vF12(czz, r1h1z, i1h1z, r1h1z, i1h1z, r1h2z, i1h2z, r1h2z, i1h2z)
	+ S2vF12(cxy, r1h1x, i1h1x, r1h1y, i1h1y, r1h2x, i1h2x, r1h2y, i1h2y)
	+ S2vF12(cyz, r1h1y, i1h1y, r1h1z, i1h1z, r1h2y, i1h2y, r1h2z, i1h2z)
	+ S2vF12(cxz, r1h1x, i1h1x, r1h1z, i1h1z, r1h2x, i1h2x, r1h2z, i1h2z);
    return(0.5 * tmp);
}

/* 18 args - 6 + 6 + 6 */
double DFiF12(double cxx, double cyy, double czz, double cxy, double cyz, double cxz, double r1h1x, double i1h1x, double r1h1y, double i1h1y, double r1h1z, double i1h1z, double r1h2x, double i1h2x, double r1h2y, double i1h2y, double r1h2z, double i1h2z)
{
double tmp;
    tmp = S2iF12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x)
	+ S2iF12(cyy, r1h1y, i1h1y, r1h1y, i1h1y, r1h2y, i1h2y, r1h2y, i1h2y)
	+ S2iF12(czz, r1h1z, i1h1z, r1h1z, i1h1z, r1h2z, i1h2z, r1h2z, i1h2z)
	+ S2iF12(cxy, r1h1x, i1h1x, r1h1y, i1h1y, r1h2x, i1h2x, r1h2y, i1h2y)
	+ S2iF12(cyz, r1h1y, i1h1y, r1h1z, i1h1z, r1h2y, i1h2y, r1h2z, i1h2z)
	+ S2iF12(cxz, r1h1x, i1h1x, r1h1z, i1h1z, r1h2x, i1h2x, r1h2z, i1h2z);
    return(tmp * 0.5); /* divided by 2 to scale down */
}

/* 40 vars - 16 + 6 + 6 + 6 + 6 */
double DFn2F12(DpassStr *p)
/*
 * a structure because a standard C compiler can handle only 32 variables.
 */
{
double tmp;
    tmp = S2v2F12(p->cxx, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r2h11x, p->i2h11x, p->r2h11x, p->i2h11x, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2x, p->ih2f1f2x)
	+ S2v2F12(p->cyy, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r2h11y, p->i2h11y, p->r2h11y, p->i2h11y, p->h2f1f2y, p->ih2f1f2y, p->h2f1f2y, p->ih2f1f2y)
	+ S2v2F12(p->czz, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z, p->r2h11z, p->i2h11z, p->r2h11z, p->i2h11z, p->h2f1f2z, p->ih2f1f2z, p->h2f1f2z, p->ih2f1f2z)
	+ S2v2F12(p->cxy, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r2h11x, p->i2h11x, p->r2h11y, p->i2h11y, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2y, p->ih2f1f2y)
	+ S2v2F12(p->cyz, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z, p->r2h11y, p->i2h11y, p->r2h11z, p->i2h11z, p->h2f1f2y, p->ih2f1f2y, p->h2f1f2z, p->ih2f1f2z)
	+ S2v2F12(p->cxz, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z, p->r2h11x, p->i2h11x, p->r2h11z, p->i2h11z, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2z, p->ih2f1f2z)
	+ S3v2F12(p->cxxx, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x)
	+ S3v2F12(p->cyyy, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y)
	+ S3v2F12(p->czzz, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3v2F12(p->cxxy, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y)
	+ S3v2F12(p->cxxz, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z)
	+ S3v2F12(p->cxyy, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y)
	+ S3v2F12(p->cyyz, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z)
	+ S3v2F12(p->cxzz, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3v2F12(p->cyzz, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3v2F12(p->cxyz, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z);
    return(tmp / 3.); /* divided by 3 to get kernel(otherwise we get 3*kernel) */
}

/* 40 vars - 16 + 6 + 6 + 6 + 6 */
double DFi2F12(DpassStr *p)
{
double tmp;
    tmp = S2i2F12(p->cxx, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r2h11x, p->i2h11x, p->r2h11x, p->i2h11x, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2x, p->ih2f1f2x)
	+ S2i2F12(p->cyy, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r2h11y, p->i2h11y, p->r2h11y, p->i2h11y, p->h2f1f2y, p->ih2f1f2y, p->h2f1f2y, p->ih2f1f2y)
	+ S2i2F12(p->czz, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z, p->r2h11z, p->i2h11z, p->r2h11z, p->i2h11z, p->h2f1f2z, p->ih2f1f2z, p->h2f1f2z, p->ih2f1f2z)
	+ S2i2F12(p->cxy, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r2h11x, p->i2h11x, p->r2h11y, p->i2h11y, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2y, p->ih2f1f2y)
	+ S2i2F12(p->cyz, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z, p->r2h11y, p->i2h11y, p->r2h11z, p->i2h11z, p->h2f1f2y, p->ih2f1f2y, p->h2f1f2z, p->ih2f1f2z)
	+ S2i2F12(p->cxz, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z, p->r2h11x, p->i2h11x, p->r2h11z, p->i2h11z, p->h2f1f2x, p->ih2f1f2x, p->h2f1f2z, p->ih2f1f2z)
	+ S3i2F12(p->cxxx, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x)
	+ S3i2F12(p->cyyy, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y)
	+ S3i2F12(p->czzz, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3i2F12(p->cxxy, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y)
	+ S3i2F12(p->cxxz, p->r1h1x, p->i1h1x, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z)
	+ S3i2F12(p->cxyy, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y)
	+ S3i2F12(p->cyyz, p->r1h1y, p->i1h1y, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z)
	+ S3i2F12(p->cxzz, p->r1h1x, p->i1h1x, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3i2F12(p->cyzz, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h1z, p->i1h1z, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z, p->r1h2z, p->i1h2z)
	+ S3i2F12(p->cxyz, p->r1h1x, p->i1h1x, p->r1h1y, p->i1h1y, p->r1h1z, p->i1h1z, p->r1h2x, p->i1h2x, p->r1h2y, p->i1h2y, p->r1h2z, p->i1h2z);
    return(tmp / 3.); /* divided by 3 to get kernel(otherwise we get 3*kernel) */
}

/* 12 variables */
double D1n2F1(double cxx, double r1h1x, double i1h1x)
{
    return(S2v2F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x));
}

double D1n3F1(double cxx, double cxxx, double r1h1x, double i1h1x, double r2h11x, double i2h11x)
{
    return(S2v3F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r2h11x, i2h11x, r2h11x, i2h11x)
	 + S3v3F1(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x));
}

/* 18 args - 6 + 6 + 6 */
double D1nF12(double cxx, double r1h1x, double i1h1x, double r1h2x, double i1h2x)
{
    return(0.5 * S2vF12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x));
}

/* 40 vars - 16 + 6 + 6 + 6 + 6 */
double D1n2F12(double cxx, double cxxx,
	double r1h1x, double i1h1x, double r1h2x, double i1h2x,
	double r2h11x, double i2h11x, double h2f1f2x, double ih2f1f2x)
{
double tmp;
    tmp = S2v2F12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x,
		r2h11x, i2h11x, r2h11x, i2h11x, h2f1f2x, ih2f1f2x, h2f1f2x, ih2f1f2x)
	+ S3v2F12(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x,
		r1h2x, i1h2x, r1h2x, i1h2x, r1h2x, i1h2x);
    return(tmp / 3.); /* divided by 3 to get kernel(otherwise we get 3*kernel) */
}

/* 12 variables */
double D1i2F1(double cxx, double r1h1x, double i1h1x)
{
    return(S2i2F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x));
}

double D1i3F1(double cxx, double cxxx, double r1h1x, double i1h1x, double r2h11x, double i2h11x)
{
    return(S2i3F1(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r2h11x, i2h11x, r2h11x, i2h11x)
	 + S3i3F1(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x));
}

/* 18 args - 6 + 6 + 6 */
double D1iF12(double cxx, double r1h1x, double i1h1x, double r1h2x, double i1h2x)
{
    return(0.5 * S2iF12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x));
}

/* 40 vars - 16 + 6 + 6 + 6 + 6 */
double D1i2F12(double cxx, double cxxx, double r1h1x, double i1h1x, double r1h2x, double i1h2x, double r2h11x, double i2h11x, double h2f1f2x, double ih2f1f2x)
{
double tmp;
    tmp = S2i2F12(cxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h2x, i1h2x, r1h2x, i1h2x,
			r2h11x, i2h11x, r2h11x, i2h11x, h2f1f2x, ih2f1f2x, h2f1f2x, ih2f1f2x)
	+ S3i2F12(cxxx, r1h1x, i1h1x, r1h1x, i1h1x, r1h1x, i1h1x,
			r1h2x, i1h2x, r1h2x, i1h2x, r1h2x, i1h2x);
    return(tmp / 3.); /* divided by 3 to get kernel(otherwise we get 3*kernel) */
}
