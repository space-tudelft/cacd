/*
 * Revision 3.2.2 1999/4/20  18:00:00  Weidong
 * BSIM3v3.2.2 release
 */

/**********
Copyright 1999 Regents of the University of California.  All rights reserved.
Author: Min-Chie Jeng.
Author: 1997-1999 Weidong Liu.
File: b3check.c
**********/

#include "spice.h"
#include <stdio.h>
#include <math.h>
#include "util.h"
#include "cktdefs.h"
#include "bsim3def.h"
#include "trandefs.h"
#include "const.h"
#include "sperror.h"
#include "devdefs.h"

int BSIM3checkModel(BSIM3model *model, BSIM3instance *here, CKTcircuit *ckt)
{
    struct bsim3SizeDependParam *pParam;
    int Fatal_Flag = 0;
    FILE *fplog;
    char *msg;
    char *ver = "3.2.2";

#define PRINT(x) fprintf(fplog, msg, x); printf(msg, x)

    if ((fplog = fopen("b3v3check.log", "w")))
    {
	pParam = here->pParam;
	fprintf(fplog, "BSIM3v%s Parameter Checking.\n", ver);
        if (strcmp(model->BSIM3version, ver))
        {   msg = "Warning: This model is BSIM3v%s; you specified a wrong version number.\n";
	    PRINT(ver);
        }
	fprintf(fplog, "Model = %s\n", (char*)model->BSIM3modName);

	if (pParam->BSIM3nlx < -pParam->BSIM3leff)
	{   msg = "Fatal: Nlx = %g is less than -Leff.\n";
	    PRINT(pParam->BSIM3nlx);
	    Fatal_Flag = 1;
	}
	if (model->BSIM3tox <= 0.0)
	{   msg = "Fatal: Tox = %g is not positive.\n";
	    PRINT(model->BSIM3tox);
	    Fatal_Flag = 1;
	}
        if (model->BSIM3toxm <= 0.0)
        {   msg = "Fatal: Toxm = %g is not positive.\n";
	    PRINT(model->BSIM3toxm);
            Fatal_Flag = 1;
        }
	if (pParam->BSIM3npeak <= 0.0)
	{   msg = "Fatal: Nch = %g is not positive.\n";
	    PRINT(pParam->BSIM3npeak);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3nsub <= 0.0)
	{   msg = "Fatal: Nsub = %g is not positive.\n";
	    PRINT(pParam->BSIM3nsub);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3ngate < 0.0)
	{   msg = "Fatal: Ngate = %g is not positive.\n";
	    PRINT(pParam->BSIM3ngate);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3ngate > 1.e25)
	{   msg = "Fatal: Ngate = %g is too high.\n";
	    PRINT(pParam->BSIM3ngate);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3xj <= 0.0)
	{   msg = "Fatal: Xj = %g is not positive.\n";
	    PRINT(pParam->BSIM3xj);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3dvt1 < 0.0)
	{   msg = "Fatal: Dvt1 = %g is negative.\n";
	    PRINT(pParam->BSIM3dvt1);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3dvt1w < 0.0)
	{   msg = "Fatal: Dvt1w = %g is negative.\n";
	    PRINT(pParam->BSIM3dvt1w);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3w0 == -pParam->BSIM3weff)
	{   msg = "Fatal: (W0 + Weff) = 0 causing divided-by-zero.\n";
	    PRINT(msg);
	    Fatal_Flag = 1;
        }
	if (pParam->BSIM3dsub < 0.0)
	{   msg = "Fatal: Dsub = %g is negative.\n";
	    PRINT(pParam->BSIM3dsub);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3b1 == -pParam->BSIM3weff)
	{   msg = "Fatal: (B1 + Weff) = 0 causing divided-by-zero.\n";
	    PRINT(msg);
	    Fatal_Flag = 1;
        }
        if (pParam->BSIM3u0temp <= 0.0)
	{   msg = "Fatal: u0 at current temperature = %g is not positive.\n";
	    PRINT(pParam->BSIM3u0temp);
	    Fatal_Flag = 1;
        }
/* Check delta parameter */
        if (pParam->BSIM3delta < 0.0)
	{   msg = "Fatal: Delta = %g is less than zero.\n";
	    PRINT(pParam->BSIM3delta);
	    Fatal_Flag = 1;
        }
	if (pParam->BSIM3vsattemp <= 0.0)
	{   msg = "Fatal: Vsat at current temperature = %g is not positive.\n";
	    PRINT(pParam->BSIM3vsattemp);
	    Fatal_Flag = 1;
	}
/* Check Rout parameters */
	if (pParam->BSIM3pclm <= 0.0)
	{   msg = "Fatal: Pclm = %g is not positive.\n";
	    PRINT(pParam->BSIM3pclm);
	    Fatal_Flag = 1;
	}
	if (pParam->BSIM3drout < 0.0)
	{   msg = "Fatal: Drout = %g is negative.\n";
	    PRINT(pParam->BSIM3drout);
	    Fatal_Flag = 1;
	}
        if (pParam->BSIM3pscbe2 <= 0.0)
        {   msg = "Warning: Pscbe2 = %g is not positive.\n";
	    PRINT(pParam->BSIM3pscbe2);
        }

      if (model->BSIM3unitLengthSidewallJctCap > 0.0 ||
          model->BSIM3unitLengthGateSidewallJctCap > 0.0)
      {
	if (here->BSIM3drainPerimeter < pParam->BSIM3weff)
	{   msg = "Warning: Pd = %g is less than W.\n";
	    PRINT(here->BSIM3drainPerimeter);
	}
	if (here->BSIM3sourcePerimeter < pParam->BSIM3weff)
	{   msg = "Warning: Ps = %g is less than W.\n";
	    PRINT(here->BSIM3sourcePerimeter);
	}
      }

        if (pParam->BSIM3noff < 0.1)
        {   msg = "Warning: Noff = %g is too small.\n";
	    PRINT(pParam->BSIM3noff);
        }
        if (pParam->BSIM3noff > 4.0)
        {   msg = "Warning: Noff = %g is too large.\n";
	    PRINT(pParam->BSIM3noff);
        }
        if (pParam->BSIM3voffcv < -0.5)
        {   msg = "Warning: Voffcv = %g is too small.\n";
	    PRINT(pParam->BSIM3voffcv);
        }
        if (pParam->BSIM3voffcv > 0.5)
        {   msg = "Warning: Voffcv = %g is too large.\n";
	    PRINT(pParam->BSIM3voffcv);
        }
        if (model->BSIM3ijth < 0.0)
        {   msg = "Fatal: Ijth = %g cannot be negative.\n";
	    PRINT(model->BSIM3ijth);
            Fatal_Flag = 1;
        }

/* Check capacitance parameters */
        if (pParam->BSIM3clc < 0.0)
	{   msg = "Fatal: Clc = %g is negative.\n";
	    PRINT(pParam->BSIM3clc);
	    Fatal_Flag = 1;
        }
        if (pParam->BSIM3moin < 5.0)
        {   msg = "Warning: Moin = %g is too small.\n";
	    PRINT(pParam->BSIM3moin);
        }
        if (pParam->BSIM3moin > 25.0)
        {   msg = "Warning: Moin = %g is too large.\n";
	    PRINT(pParam->BSIM3moin);
        }
        if (pParam->BSIM3acde < 0.4)
        {   msg = "Warning: Acde = %g is too small.\n";
	    PRINT(pParam->BSIM3acde);
        }
        if (pParam->BSIM3acde > 1.6)
        {   msg = "Warning: Acde = %g is too large.\n";
	    PRINT(pParam->BSIM3acde);
        }

      if (model->BSIM3paramChk == 1)
      {
/* Check L and W parameters */
	if (pParam->BSIM3leff <= 5.0e-8)
	{   msg = "Warning: Leff = %g may be too small.\n";
	    PRINT(pParam->BSIM3leff);
	}
	if (pParam->BSIM3leffCV <= 5.0e-8)
	{   msg = "Warning: Leff for CV = %g may be too small.\n";
	    PRINT(pParam->BSIM3leffCV);
	}
        if (pParam->BSIM3weff <= 1.0e-7)
	{   msg = "Warning: Weff = %g may be too small.\n";
	    PRINT(pParam->BSIM3weff);
	}
	if (pParam->BSIM3weffCV <= 1.0e-7)
	{   msg = "Warning: Weff for CV = %g may be too small.\n";
	    PRINT(pParam->BSIM3weffCV);
	}

/* Check threshold voltage parameters */
	if (pParam->BSIM3nlx < 0.0)
	{   msg = "Warning: Nlx = %g is negative.\n";
	    PRINT(pParam->BSIM3nlx);
        }
	if (model->BSIM3tox < 1.0e-9)
	{   msg = "Warning: Tox = %g is less than 10A.\n";
	    PRINT(model->BSIM3tox);
        }
        if (pParam->BSIM3npeak <= 1.0e15)
	{   msg = "Warning: Nch = %g may be too small.\n";
	    PRINT(pParam->BSIM3npeak);
	}
	else if (pParam->BSIM3npeak >= 1.0e21)
	{   msg = "Warning: Nch = %g may be too large.\n";
	    PRINT(pParam->BSIM3npeak);
	}
	if (pParam->BSIM3nsub <= 1.0e14)
	{   msg = "Warning: Nsub = %g may be too small.\n";
	    PRINT(pParam->BSIM3nsub);
	}
	else if (pParam->BSIM3nsub >= 1.0e21)
	{   msg = "Warning: Nsub = %g may be too large.\n";
	    PRINT(pParam->BSIM3nsub);
	}
	if (pParam->BSIM3ngate > 0.0 && pParam->BSIM3ngate <= 1.e18)
	{   msg = "Warning: Ngate = %g is less than 1.E18cm^-3.\n";
	    PRINT(pParam->BSIM3ngate);
	}
        if (pParam->BSIM3dvt0 < 0.0)
	{   msg = "Warning: Dvt0 = %g is negative.\n";
	    PRINT(pParam->BSIM3dvt0);
	}
	if (fabs(1.0e-6 / (pParam->BSIM3w0 + pParam->BSIM3weff)) > 10.0)
	{   msg = "Warning: (W0 + Weff) may be too small.\n";
	    PRINT(msg);
        }

/* Check subthreshold parameters */
	if (pParam->BSIM3nfactor < 0.0)
	{   msg = "Warning: Nfactor = %g is negative.\n";
	    PRINT(pParam->BSIM3nfactor);
	}
	if (pParam->BSIM3cdsc < 0.0)
	{   msg = "Warning: Cdsc = %g is negative.\n";
	    PRINT(pParam->BSIM3cdsc);
	}
	if (pParam->BSIM3cdscd < 0.0)
	{   msg = "Warning: Cdscd = %g is negative.\n";
	    PRINT(pParam->BSIM3cdscd);
	}
/* Check DIBL parameters */
	if (pParam->BSIM3eta0 < 0.0)
	{   msg = "Warning: Eta0 = %g is negative.\n";
	    PRINT(pParam->BSIM3eta0);
	}
/* Check Abulk parameters */
	if (fabs(1.0e-6 / (pParam->BSIM3b1 + pParam->BSIM3weff)) > 10.0)
       	{   msg = "Warning: (B1 + Weff) may be too small.\n";
	    PRINT(msg);
        }
/* Check Saturation parameters */
     	if (pParam->BSIM3a2 < 0.01)
	{   msg = "Warning: A2 = %g is too small. Set to 0.01.\n";
	    PRINT(pParam->BSIM3a2);
	    pParam->BSIM3a2 = 0.01;
	}
	else if (pParam->BSIM3a2 > 1.0)
	{   msg = "Warning: A2 = %g is larger than 1. A2 is set to 1 and A1 is set to 0.\n";
	    PRINT(pParam->BSIM3a2);
	    pParam->BSIM3a2 = 1.0;
	    pParam->BSIM3a1 = 0.0;
	}

	if (pParam->BSIM3rdsw < 0.0)
	{   msg = "Warning: Rdsw = %g is negative. Set to zero.\n";
	    PRINT(pParam->BSIM3rdsw);
	    pParam->BSIM3rdsw = 0.0;
	    pParam->BSIM3rds0 = 0.0;
	}
	else if (pParam->BSIM3rds0 > 0.0 && pParam->BSIM3rds0 < 0.001)
	{   msg = "Warning: Rds at current temperature = %g is less than 0.001 ohm. Set to zero.\n";
	    PRINT(pParam->BSIM3rds0);
	    pParam->BSIM3rds0 = 0.0;
	}
	if (pParam->BSIM3vsattemp < 1.0e3)
	{   msg = "Warning: Vsat at current temperature = %g may be too small.\n";
	    PRINT(pParam->BSIM3vsattemp);
	}
	if (pParam->BSIM3pdibl1 < 0.0)
	{   msg = "Warning: Pdibl1 = %g is negative.\n";
	    PRINT(pParam->BSIM3pdibl1);
	}
	if (pParam->BSIM3pdibl2 < 0.0)
	{   msg = "Warning: Pdibl2 = %g is negative.\n";
	    PRINT(pParam->BSIM3pdibl2);
	}
/* Check overlap capacitance parameters */
        if (model->BSIM3cgdo < 0.0)
	{   msg = "Warning: cgdo = %g is negative. Set to zero.\n";
	    PRINT(model->BSIM3cgdo);
	    model->BSIM3cgdo = 0.0;
        }
        if (model->BSIM3cgso < 0.0)
	{   msg = "Warning: cgso = %g is negative. Set to zero.\n";
	    PRINT(model->BSIM3cgso);
	    model->BSIM3cgso = 0.0;
        }
        if (model->BSIM3cgbo < 0.0)
	{   msg = "Warning: cgbo = %g is negative. Set to zero.\n";
	    PRINT(model->BSIM3cgbo);
	    model->BSIM3cgbo = 0.0;
        }
     }
	fclose(fplog);
    }
    else {
	fprintf(stderr, "Warning: Can't open log file. Parameter checking skipped.\n");
    }

    return(Fatal_Flag);
}
