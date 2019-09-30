/*
 * ISC License
 *
 * Copyright (C) 1986-2018 by
 *	A.C. de Graaf
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

#include "src/sls/extern.h"

extern void evaldf (int fx, char *S);
extern void initdf (int fx, char *S);
extern void loaddf (int fx, char *S);
extern void next_forced_event (NODE *n, int eventpres);
extern int  eval_forcedinfos (NODE *n, simtime_t tmin, simtime_t *pmintswitch, short *pnextfres);
extern int  getval_forcedinfos (NODE *n, simtime_t t);

static void all_fsched (int fox, int res);
static int  ctol (char c);
static int  evalfand (int fix);
static int  evalfexor (int fix);
static int  evalfnand (int fix);
static int  evalfnor (int fix);
static int  evalfor (int fix);
static void fsched (int fox, int res);
static char ltoc (int l);

void initfunctionals (int round)
{
    int i;
    int cnt;
    int fsx;
    int fix;
    int frx;
    int fox;
    int newx;
    FUNCTION * f;

    if (round == 1)
	arr_init ((char **)(void *)&fsnulls, sizeof (int), 10, 3.0);

    for (i = 0; i < F_cnt; i++) {
        if (F[i].type < 1000) {

            f = &F[i];
            fsx = f -> fsx;

            arr_reset ((char *)fsnulls);

            if ((fox = f -> fox) >= 0) {
                cnt = FO[ fox++ ].x;
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

            if ((fix = f -> fix) >= 0) {
                cnt = FI[ fix++ ];
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

            if ((frx = f -> frx) >= 0) {
                cnt = FR[ frx++ ];
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

            newx = arr_new ((char **)(void *)&fsnulls);
            fsnulls[ newx ] = -1;

            currf = &F[i];
	    if (round == 1)

	        /* the load part of the user-defined function block
		   is executed */

		loaddf ((int)F[i].type, (char *)&FS[ F[i].fsx ]);

	    else

	        /* the initial part of the user-defined function block
		   is executed   */

		initdf ((int)F[i].type, (char *)&FS[ F[i].fsx ]);
        }
    }
}

void evalfunctional (FUNCTION *f)
{
    char name[DM_MAXNAME + 1];
    int fix;
    int frx;
    int fox;
    int fsx;
    int cnt;
    int newx;
    int fsnullsx;
    int lsn;
    NODE * n;

    FORCEDSIGNAL *fs; /* the field 'forcedinfo'  of a forced node is */
		      /* of this type as well                        */

    if (debugsim) {
        switch (f -> type)
        {
            case F_OR :
                sprintf (name, "OR");
                break;
            case F_NOR :
                sprintf (name, "NOR");
                break;
            case F_AND :
                sprintf (name, "AND");
                break;
            case F_NAND :
                sprintf (name, "NAND");
                break;
            case F_EXOR :
                sprintf (name, "EXOR");
                break;
            case F_INVERT :
                sprintf (name, "INVERT");
                break;
            default :
                sprintf (name, "%s", FD[f->type].name);
                break;
        }
	fprintf (debug, "\n---------- evalfunctional %s ----------\n\n", name);
    }

    switch ( f -> type )
    {
	case F_OR :
	    all_fsched ( f -> fox, evalfor (f -> fix));
	    break;
	case F_AND :
	    all_fsched ( f -> fox, evalfand (f -> fix));
	    break;
	case F_NOR :
	    all_fsched ( f -> fox, evalfnor (f -> fix));
	    break;
	case F_NAND :
	    all_fsched ( f -> fox, evalfnand (f -> fix));
	    break;
	case F_EXOR :
	    all_fsched ( f -> fox, evalfexor (f -> fix));
	    break;
	case F_INVERT :
	    all_fsched ( f -> fox, evalfnand (f -> fix));
				   /* use nand evaluation */
	    break;

	default :

            fsx = f -> fsx;

            arr_reset ((char *)fsnulls);

	/* the output terminals of a user-defined function block as     */
	/* they are represented in the FO array, are initialized before */
	/* the beginning of the evaluation in the following way:        */
	/*    - when the terminal is of type OUTPUT the value of the    */
	/*      this terminal as it can be found in the 'forcedinfo'    */
	/*      list of the node is put back to the FS array;           */
	/*    - when the terminal is of type INOUT  the value of the    */
	/*      node is put in the FS  array;                           */
	/* if the value of the terminal is changed in the function      */
	/* this new value is put in the FS array; when this is not      */
	/* the case, the value as it was put in FS before evaluation    */
	/* of the function block will remain in FS                      */

            if ((fox = f -> fox) >= 0) {
                cnt = FO[ fox++ ].x;
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        n = &N[ FO[ fox ].x ];
			if ( FO[ fox ].type == INOUT ) {
			    lsn = LSTATE (n);
			    FS[ fsx ] = ltoc (lsn);
			}
			else if ( FO[ fox ].type == OUTPUT ) {
			    fs = n->forcedinfo;
			    while ( fs != NULL && fs->fox != fox )
				fs = fs->next;
			    if ( fs == NULL ) ERROR_EXIT(1);
			    FS[ fsx ] = ltoc (fs->stabfstate);
			}
			else
			     ERROR_EXIT(1);

			fox++;
			cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

	/* the  input terminals of a user-defined function block as     */
	/* they are represented in the FI array, are initialized before */
	/* the beginning of the evaluation by putting the value of the  */
	/* node the terminal is connected to in the FS array            */

            if ((fix = f -> fix) >= 0) {
                cnt = FI[ fix++ ];
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        n = &N[ FI[ fix++ ] ];
                        lsn = LSTATE (n);
                        FS[ fsx ] = ltoc (lsn);
                        if (debugsim) {
                            fprintf (debug, "input_t %s : %c\n", hiername (n - N), FS[fsx]);
                        }
                        cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

	/* the inread terminals of a user-defined function block as     */
	/* they are represented in the FR array, are initialized before */
	/* the beginning of the evaluation by putting the value of the  */
	/* node the terminal is connected to in the FS array            */

            if ((frx = f -> frx) >= 0) {
                cnt = FR[ frx++ ];
                while (cnt > 0) {
                    if ( FS[ fsx ] != '\0' ) {
                        n = &N[ FR[ frx++ ] ];
                        lsn = LSTATE (n);
                        FS[ fsx ] = ltoc (lsn);
                        if (debugsim) {
                            fprintf (debug, "input_r %s : %c\n", hiername (n - N), FS[fsx]);
                        }
                        cnt--;
                    }
                    else {
                        newx = arr_new ((char **)(void *)&fsnulls);
                        fsnulls[ newx ] = fsx;
                    }
                    fsx++;
                }
            }

            newx = arr_new ((char **)(void *)&fsnulls);
            fsnulls[ newx ] = -1;

	/* the behavior part of the user-defined function block is executed   */

            currf = f;
            evaldf ((int)(f -> type), (char *)&FS[ f -> fsx ]);

            fsx = f -> fsx;

            fsnullsx = 0;

            if ((fox = f -> fox) >= 0) {
                cnt = FO[ fox++ ].x;
                while (cnt > 0) {
                    if ( fsnulls [ fsnullsx ] == fsx ) {
                        FS[ fsx ] = '\0';
                        fsnullsx++;
                    }
                    else {
                        fsched ( fox++, ctol (FS[ fsx ]) );
                        FS[ fsx ] = 'X';
                        cnt--;
                    }
                    fsx++;
                }
            }

            if ((fix = f -> fix) >= 0) {
                cnt = FI[ fix++ ];
                while (cnt > 0) {
                    if ( fsnulls [ fsnullsx ] == fsx ) {
                        FS[ fsx ] = '\0';
                        fsnullsx++;
                    }
                    else {
                        FS[ fsx ] = 'X';
                        cnt--;
                    }
                    fsx++;
                }
            }

            if ((frx = f -> frx) >= 0) {
                cnt = FR[ frx++ ];
                while (cnt > 0) {
                    if ( fsnulls [ fsnullsx ] == fsx ) {
                        FS[ fsx ] = '\0';
                        fsnullsx++;
                    }
                    else {
                        FS[ fsx ] = 'X';
                        cnt--;
                    }
                    fsx++;
                }
            }
    }

    f -> evalflag = FALSE;
}

static void all_fsched (int fox, int res)
{
    int cnt = FO[fox++].x;

    while (cnt-- > 0) {
        fsched (fox++, res);
    }
}

static void fsched (int fox, int res)
{
    int nextres;
    int stabres;
    double delay;
    simtime_t minevent, maxevent;
    NODE * n;

    int found;
    int eventpres;
    short nextfres;
    simtime_t mintswitch;
    FORCEDSIGNAL *nfoxforcedinfo, *fs;

    if (stepdelay) {
	minevent = maxevent = tcurr;
    }
    else {
        switch (res)
        {
	    case H_state :
                delay = (double) FO[fox].trise / outtimeaccur;
	        break;
	    case L_state :
                delay = (double) FO[fox].tfall / outtimeaccur;
	        break;
	    case X_state :
	    case F_state :
	        if (FO[fox].trise < FO[fox].tfall)
                    delay = (double) FO[fox].trise / outtimeaccur;
	        else
                    delay = (double) FO[fox].tfall / outtimeaccur;
	        break;
	    default:
		delay = 0;
		ERROR_EXIT(1);
        }
	minevent = tcurr + (simtime_t)(delay * mindevtime);
	maxevent = tcurr + (simtime_t)(delay * maxdevtime);
    }

    stabres = res;

    /* for node n and the output/inout terminal of the function block    */
    /* represented by fox, the correct forcedinfo in the forcedinfo-list */
    /* is found                                                          */

    n = &N[ FO[fox].x ];
    nfoxforcedinfo = n->forcedinfo;
    while ( nfoxforcedinfo != NULL && nfoxforcedinfo ->fox != fox )
    	nfoxforcedinfo = nfoxforcedinfo->next;
    if ( nfoxforcedinfo == NULL ) ERROR_EXIT(1);

    /*     the found forcedinfo is given the correct initfstate          */

    if ( tcurr < nfoxforcedinfo->tswitch )
	nfoxforcedinfo -> initfstate = nfoxforcedinfo -> initfstate ;
    else if ( tcurr < nfoxforcedinfo->tswitch_stab )
	nfoxforcedinfo -> initfstate = nfoxforcedinfo -> nextfstate ;
    else
	nfoxforcedinfo -> initfstate = nfoxforcedinfo -> stabfstate ;

    /* the following 'if statements' change the value of nextres and/or    */
    /* minevent if special cases occur                                     */

    if (minevent != maxevent && stabres != nfoxforcedinfo->initfstate) {
        if (nfoxforcedinfo->initfstate == X_state) {
            nextres = stabres;
            minevent = maxevent;
            if (nfoxforcedinfo -> nextfstate == nextres
                && nfoxforcedinfo -> tswitch < minevent
                && nfoxforcedinfo -> tswitch > tcurr)
                minevent = nfoxforcedinfo -> tswitch;
        }
        else
            nextres = X_state;
    }
    else
        nextres = stabres;

    if (nfoxforcedinfo -> nextfstate != stabres
        && nextres != nfoxforcedinfo->initfstate
        && nfoxforcedinfo -> tswitch < minevent
        && nfoxforcedinfo -> tswitch > tcurr) {
        nextres = X_state;
        minevent = nfoxforcedinfo -> tswitch;
    }

    if (debugsim) {
	fprintf (debug,
           "output %s : (current) %c (next) %c (stable) %c\n",
	    hiername (n - N), ltoc ((int)(nfoxforcedinfo->initfstate)),
            ltoc (nextres), ltoc (stabres));
    }

    mintswitch = MAXSIMTIME;
    eventpres = FALSE;

    if (n -> inp
	&& n -> forcedinfo -> tswitch >= 0
	&& n -> forcedinfo -> tswitch < MAXSIMTIME) eventpres = TRUE;
    else {
	found = eval_forcedinfos( n, tcurr-1, &mintswitch, &nextfres );
	while ( found && ( ( nextfres == LSTATE(n)  && n->type == Forced )  ||
			   ( nextfres == F_state && n->type == Normal ) ) ) {
	    found = eval_forcedinfos( n, mintswitch, &mintswitch, &nextfres );
	}
	if (found) eventpres = TRUE;
    }

    /* now is known whether an event for node n is on the forced-eventlist */

    nfoxforcedinfo -> nextfstate = nextres;
    nfoxforcedinfo -> stabfstate = stabres;
    if (nextres != nfoxforcedinfo -> initfstate)
	nfoxforcedinfo -> tswitch = minevent;
    else
	nfoxforcedinfo -> tswitch = tcurr;
    if (stabres != nfoxforcedinfo -> initfstate)
	nfoxforcedinfo -> tswitch_stab = maxevent;
    else
	nfoxforcedinfo -> tswitch_stab = tcurr;

    if (debugsim) {
	fprintf (debug, "**new** eventpres = %d  fox = %d\n",eventpres,fox);
	fs = n->forcedinfo;
	while (fs) {
	    fprintf (debug, "**new** forcedinfo : fox = %d :\n\tinitfstate = %c\n\tnextfstate = %c\n\t",
		fs->fox, ltoc ((int)(fs->initfstate)), ltoc ((int)(fs->nextfstate)));
	    fprintf (debug, "stabfstate = %c\n\ttswitch = %lld\n\ttswitch_stab = %lld\n",
		ltoc ((int)(fs->stabfstate)), fs->tswitch, fs->tswitch_stab);
	    fs = fs->next;
	}
    }

    next_forced_event( n, eventpres );
}


/* next_force_event() : the forcedinfo-list of node n is evaluated and  */
/*                    : the first event on or after t=tcurr is put on   */
/*                    : the forced-event-list                           */

void next_forced_event (NODE *n, int eventpres)
{
    int found;
    simtime_t mintswitch;
    short nextfres;

    found = eval_forcedinfos( n, tcurr-1, &mintswitch, &nextfres );

    /* if an event results from the forcedinfo-list, BUT the resulting */
    /* logical value is the same as the value of the node at this      */
    /* moment, OR the resulting logical value is free and the type     */
    /* of the node is already normal  : the event has NO effect  and   */
    /* is not to be scheduled                                          */

    while ( found && ( ( nextfres == LSTATE(n)  && n->type == Forced )  ||
		       ( nextfres == F_state && n->type == Normal ) ) ) {
	found = eval_forcedinfos( n, mintswitch, &mintswitch, &nextfres );
    }

    if (n -> inp
	&& n -> forcedinfo -> tswitch >= 0
	&& n -> forcedinfo -> tswitch < MAXSIMTIME
	&& n -> forcedinfo -> tswitch < mintswitch) {

        /* This is an event where state nor type does change.
	   Only the value of the input signal will change here */

	mintswitch = n -> forcedinfo -> tswitch;
	nextfres = n -> forcedinfo -> nextfstate;
	found = TRUE;
    }

    if ( found ) {
	if ( eventpres )
	    resched_event( n, Forced, mintswitch );
	else
	    sched_event( n, Forced, mintswitch );
    }
    else {
	if (eventpres)
	    retr_event( n, Forced );
    }
}


/* eval_forcedinfos() : the forcedinfo-list of node n is evaluated and   */
/*                    : the first tswitch or tswitch_stab of a           */
/*                    : forcedinfo after t=tmin that is  found           */
/*                    : defines pmintswitch;                             */
/*                    : pnextfres is the logical value that results      */
/*                    : when the logical values of all the forcedinfo's  */
/*                    : on t=pmintswitch are combined                    */
/*                    : if NO new event results from the forcedinfo-list */
/*                    : the routine returns 0,  else 1                   */

int eval_forcedinfos (NODE *n, simtime_t tmin, simtime_t *pmintswitch, short *pnextfres)
{
    FORCEDSIGNAL *fs;

    *pmintswitch = MAXSIMTIME;
    fs = n->forcedinfo;
    while (fs) {
	if ( fs->tswitch > tmin && fs->tswitch < *pmintswitch ) {
		*pmintswitch = fs->tswitch;
	}
	else if ( fs->tswitch_stab > tmin && fs->tswitch_stab < *pmintswitch ) {
		*pmintswitch = fs->tswitch_stab;
	}
	fs = fs->next;
    }
    if (*pmintswitch == MAXSIMTIME) {
	*pnextfres = X_state;
	return(0);
    }

    *pnextfres = getval_forcedinfos (n, *pmintswitch);

    return (*pmintswitch < MAXSIMTIME);
}

int getval_forcedinfos (NODE *n, simtime_t t)
{
    FORCEDSIGNAL *fs;
    int val = F_state;

    if (n -> inp) {
	fs = n->forcedinfo;
	if ( t < fs->tswitch ) {
	    val = fs->initfstate;
	}
	else if ( t < fs->tswitch_stab ) {
	    val = fs->nextfstate;
	}
	else {
	    val = fs->stabfstate;
	}
	if (val == F_state)
	    fs = fs -> next;
	else
	    /* A non-free_state input value dominates the other values. */
	    return (val);
    }
    else
	fs = n->forcedinfo;

    while (fs) {
	if ( t < fs->tswitch ) {
	    if ( fs->initfstate != F_state &&
	    	 fs->initfstate != val ) {
		    if ( val != F_state ) {
			val = X_state;
		    }
		    else {
			val = fs->initfstate;
		    }
	    }
	}
	else if ( t < fs->tswitch_stab ) {
	    if ( fs->nextfstate != F_state &&
	    	 fs->nextfstate != val ) {
		    if ( val != F_state ) {
			val = X_state;
		    }
		    else {
			val = fs->nextfstate;
		    }
	    }
	}
	else  {
	    if ( fs->stabfstate != F_state &&
	    	 fs->stabfstate != val ) {
		    if ( val != F_state ) {
			val = X_state;
		    }
		    else {
			val = fs->stabfstate;
		    }
	    }
	}
	fs = fs->next;
    }
    return (val);
}

static int evalfor (int fix)
{
    NODE *n;
    int cnt;
    int res = L_state;

    for (cnt = FI[fix++]; cnt > 0; cnt--) {
	n = &N[FI[fix]];
	if (debugsim) {
	    fprintf (debug, "input %s : %d\n", hiername (FI[fix]), LSTATE(n));
	}
	switch (LSTATE(n)) {
	    case H_state :
		res = H_state;
		break;
	    case X_state :
		if (res != H_state) res = X_state;
		break;
	    case L_state :
		break;
	}
	fix++;
    }
    return (res);
}

static int evalfand (int fix)
{
    NODE *n;
    int cnt;
    int res = H_state;

    for (cnt = FI[fix++]; cnt > 0; cnt--) {
	n = &N[FI[fix]];
	if (debugsim) {
	    fprintf (debug, "input %s : %d\n", hiername (FI[fix]), LSTATE(n));
	}
	switch (LSTATE(n)) {
	    case H_state :
		break;
	    case X_state :
		if (res != L_state) res = X_state;
		break;
	    case L_state :
		res = L_state;
		break;
	}
	fix++;
    }
    return (res);
}

static int evalfnor (int fix)
{
    NODE *n;
    int cnt;
    int res = H_state;

    for (cnt = FI[fix++]; cnt > 0; cnt--) {
	n = &N[FI[fix]];
	if (debugsim) {
	    fprintf (debug, "input %s : %d\n", hiername (FI[fix]), LSTATE(n));
	}
	switch (LSTATE(n)) {
	    case H_state :
		res = L_state;
		break;
	    case X_state :
		if (res != L_state) res = X_state;
		break;
	    case L_state :
		break;
	}
	fix++;
    }
    return (res);
}

static int evalfnand (int fix)
{
    NODE *n;
    int cnt;
    int res = L_state;

    for (cnt = FI[fix++]; cnt > 0; cnt--) {
	n = &N[FI[fix]];
	if (debugsim) {
	    fprintf (debug, "input %s : %d\n", hiername (FI[fix]), LSTATE(n));
	}
	switch (LSTATE(n)) {
	    case H_state :
		break;
	    case X_state :
		if (res != H_state) res = X_state;
		break;
	    case L_state :
		res = H_state;
		break;
	}
	fix++;
    }
    return (res);
}

static int evalfexor (int fix)
{
    NODE *n;
    int cnt;
    int H_cnt;
    int X_cnt;
    int res;

    H_cnt = 0;
    X_cnt = 0;

    for (cnt = FI[fix++]; cnt > 0; cnt--) {
	n = &N[FI[fix]];
	if (debugsim) {
	    fprintf (debug, "input %s : %d\n", hiername (FI[fix]), LSTATE(n));
	}
	switch (LSTATE(n)) {
	    case H_state :
                H_cnt++;
		break;
	    case X_state :
                X_cnt++;
		break;
	    case L_state :
		break;
	}
	fix++;
    }
    if (X_cnt > 0)
        res = X_state;
    else {
        if (((H_cnt / 2) * 2) != H_cnt)
            res = H_state;            /* odd */
        else
            res = L_state;            /* even */
    }
    return (res);
}

static char ltoc (int l)
{
    char val;

    switch (l) {
        case L_state :
            val = 'O';
            break;
        case H_state :
            val = 'I';
            break;
        case F_state :
            val = 'F';
            break;
        case X_state :
        default :
            val = 'X';
            break;
    }
    return (val);
}

static int ctol (char c)
{
    int val;

    switch (c) {
        case 'O':
            val = L_state;
            break;
        case 'I':
            val = H_state;
            break;
        case 'F':
            val = F_state;
            break;
        case 'X':
        default :
            val = X_state;
            break;
    }

    return (val);
}
