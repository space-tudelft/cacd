/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1988 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "ifsim.h"
#include "inpdefs.h"
#include "inpmacs.h"
#include "fteext.h"
#include "util.h"

int INP2dot(GENERIC *ckt, INPtables *tab, card *current, GENERIC *task, GENERIC *gnode)
{
    /* .<something> Many possibilities */

    char *line; /* the part of the current line left to parse */
    char *name; /* the resistor's name */
    char *nname1;   /* the first node's name */
    char *nname2;   /* the second node's name */
    char *point;
    GENERIC *node1; /* the first node's node pointer */
    GENERIC *node2; /* the second node's node pointer */
    int error;      /* error code temporary */
    IFvalue ptemp;  /* a value structure to package resistance into */
    IFvalue *parm;  /* a pointer to a value struct for function returns */
    IFparm *prm;    /* pointer to parameter to search through array */
    char *token;    /* a token from the line */
    int which;      /* which analysis we are performing */
    int found;
    int i;          /* generic loop variable */
    GENERIC *foo;   /* pointer to analysis */
    char *steptype; /* ac analysis, type of stepping function */
    double dtemp;   /* random double precision temporary */
    char *word;     /* something to stick a word of input into */

    line = current->line;
    INPgetTok(&line, &token, 1);

    if (eq(token, ".model")) {
	/* don't have to do anything, since models were all done in pass 1 */
    }
    else if (eq(token, ".width") || eq(token, ".print") || eq(token, ".plot")) {
	/* obsolete - ignore */
	LITERR("Warning: obsolete control card - ignored\n")
    }
    else if (eq(token, ".temp")) {
	/* .temp temp1 temp2 temp3 temp4 ..... */
	/* not yet implemented - warn & ignore */
	LITERR("Warning: .TEMP card obsolete - use .options TEMP and TNOM\n")
    }
    else if (eq(token, ".op")) {
	/* .op */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "OP")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("DC operating point analysis unsupported\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Operating Point", &foo, task))
    }
    else if (eq(token, ".nodeset")) {
	/* .nodeset */
	which = -1;
	for (prm = ft_sim->nodeParms; prm < ft_sim->nodeParms+ft_sim->numNodeParms; prm++) {
	    if (eq(prm->keyword, "nodeset")) { which = prm->id; break; }
	}
	if (which == -1) {
	    LITERR("nodeset unknown to simulator.\n")
	    goto ret;
	}
	for (;;) { /* loop until we run out of data */
	    INPgetTok(&line, &name, 1);
	    /* check to see if in the form V(xxx) and grab the xxx */
	    if (!*name) break; /* end of line */
	    if ((*name == 'V' || *name == 'v') && strlen(name) == 1) {
		txfree(name);
		/* looks like V - must be V(xx) - get xx now*/
		INPgetTok(&line, &name, 1); INPtermInsert(ckt, &name, tab, &node1);
		ptemp.rValue = INPevaluate(&line, &error, 1);
		IFC(setNodeParm, (ckt, node1, which, &ptemp, (IFvalue*)NULL))
		continue;
	    }
	    LITERR("Error: .nodeset syntax error.\n")
	    break;
	}
	txfree(name);
    }
    else if (eq(token, ".disto")) {
	/* .disto {DEC OCT LIN} NP FSTART FSTOP <F2OVERF1> */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "DISTO")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Small signal distortion analysis unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Distortion Analysis", &foo, task))
	INPgetTok(&line, &steptype, 1); /* get DEC, OCT, or LIN */
	ptemp.iValue = 1;
	GCA(INPapName, (ckt, which, foo, steptype, &ptemp))
	txfree(steptype);
	parm = INPgetValue(ckt, &line, IF_INTEGER, tab); /* number of points*/
	GCA(INPapName, (ckt, which, foo, "numsteps", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstart */
	GCA(INPapName, (ckt, which, foo, "start", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstop */
	GCA(INPapName, (ckt, which, foo, "stop", parm))
	if (*line) {
	    parm = INPgetValue(ckt, &line, IF_REAL, tab); /* f1phase */
	    GCA(INPapName, (ckt, which, foo, "f2overf1", parm))
	}
    }
    else if (eq(token, ".noise")) {
        /* .noise V(OUTPUT,REF) SRC {DEC OCT LIN} NP FSTART FSTOP <PTSPRSUM> */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "NOISE")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Noise analysis unsupported.\n");
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Noise Analysis", &foo, task))

	/* Make sure the ".noise" command is followed by		*/
	/* V(xxxx).  If it is, extract 'xxxx'.  If not, report an error. */

	INPgetTok(&line, &name, 1);
	i = ((*name == 'V' || *name == 'v') && strlen(name) == 1);
	txfree(name);
	if (!i) {
	    LITERR("bad syntax [.noise v(OUT) SRC {DEC OCT LIN} NP FSTART FSTOP <PTSPRSUM>]\n");
	    goto ret;
	}
	INPgetTok(&line, &nname1, 0); INPtermInsert(ckt, &nname1, tab, &node1);
	ptemp.nValue = (IFnode)node1;
	GCA(INPapName, (ckt, which, foo, "output", &ptemp))

	if (*line != /* match ( */ ')') {
	    INPgetTok(&line, &nname2, 1); INPtermInsert(ckt, &nname2, tab, &node2);
	    ptemp.nValue = (IFnode)node2;
	} else {
	    ptemp.nValue = (IFnode)gnode;
	}
	GCA(INPapName, (ckt, which, foo, "outputref", &ptemp))

	INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	ptemp.uValue = name;
	GCA(INPapName, (ckt, which, foo, "input", &ptemp))

	INPgetTok(&line, &steptype, 1);
	ptemp.iValue = 1;
	error = INPapName(ckt, which, foo, steptype, &ptemp);
	txfree(steptype);
	if (error) current->error = INPerrCat(current->error, INPerror(error));
	parm = INPgetValue(ckt, &line, IF_INTEGER, tab);
	error = INPapName(ckt, which, foo, "numsteps", parm);
	if (error) current->error = INPerrCat(current->error, INPerror(error));
	parm = INPgetValue(ckt, &line, IF_REAL, tab);
	error = INPapName(ckt, which, foo, "start", parm);
	if (error) current->error = INPerrCat(current->error, INPerror(error));
	parm = INPgetValue(ckt, &line, IF_REAL, tab);
	error = INPapName(ckt, which, foo, "stop", parm);
	if (error) current->error = INPerrCat(current->error, INPerror(error));

	/* now see if "ptspersum" has been specified by the user */

	for (found = 0, point = line; (!found) && (*point != '\0');
	    found = ((*point != ' ') && (*(point++) != '\t')));
	if (found) {
	    parm = INPgetValue(ckt, &line, IF_INTEGER, tab);
	    error = INPapName(ckt, which, foo, "ptspersum", parm);
	    if (error) current->error = INPerrCat(current->error, INPerror(error));
	} else {
	    ptemp.iValue = 0;
	    error = INPapName(ckt, which, foo, "ptspersum", &ptemp);
	    if (error) current->error = INPerrCat(current->error, INPerror(error));
	}
    }
    else if (eq(token, ".four") || eq(token, ".fourier")) {
	/* .four */
	/* not implemented - warn & ignore */
	LITERR("Use fourier command to obtain fourier analysis\n")
    }
    else if (eq(token, ".ic")) {
	/* .ic */
	which = -1;
	for (prm = ft_sim->nodeParms; prm < ft_sim->nodeParms+ft_sim->numNodeParms; prm++) {
	    if (eq(prm->keyword, "ic")) { which = prm->id; break; }
	}
	if (which == -1) {
	    LITERR("ic unknown to simulator.\n")
	    goto ret;
	}
	for (;;) { /* loop until we run out of data */
	    INPgetTok(&line, &name, 1);
	    /* check to see if in the form V(xxx) and grab the xxx */
	    if (!*name) break; /* end of line */
	    if ((*name == 'V' || *name == 'v') && strlen(name) == 1) {
		txfree(name);
		/* looks like V - must be V(xx) - get xx now*/
		INPgetTok(&line, &name, 1); INPtermInsert(ckt, &name, tab, &node1);
		ptemp.rValue = INPevaluate(&line, &error, 1);
		IFC(setNodeParm, (ckt, node1, which, &ptemp, (IFvalue*)NULL))
		continue;
	    }
	    LITERR("Error: .ic syntax error.\n")
	    break;
	}
	txfree(name);
    }
    else if (eq(token, ".ac")) {
	/* .ac {DEC OCT LIN} NP FSTART FSTOP */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "AC")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("AC small signal analysis unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "AC Analysis", &foo, task))
	INPgetTok(&line, &steptype, 1); /* get DEC, OCT, or LIN */
	ptemp.iValue = 1;
	GCA(INPapName, (ckt, which, foo, steptype, &ptemp))
	txfree(steptype);
	parm = INPgetValue(ckt, &line, IF_INTEGER, tab); /* number of points*/
	GCA(INPapName, (ckt, which, foo, "numsteps", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstart */
	GCA(INPapName, (ckt, which, foo, "start", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstop */
	GCA(INPapName, (ckt, which, foo, "stop", parm))
    }
    else if (eq(token, ".pz")) {
	/* .pz nodeI nodeG nodeJ nodeK {V I} {POL ZER PZ} */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "PZ")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Pole-zero analysis unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Pole-Zero Analysis", &foo, task))
	parm = INPgetValue(ckt, &line, IF_NODE, tab);
	GCA(INPapName, (ckt, which, foo, "nodei", parm))
	parm = INPgetValue(ckt, &line, IF_NODE, tab);
	GCA(INPapName, (ckt, which, foo, "nodeg", parm))
	parm = INPgetValue(ckt, &line, IF_NODE, tab);
	GCA(INPapName, (ckt, which, foo, "nodej", parm))
	parm = INPgetValue(ckt, &line, IF_NODE, tab);
	GCA(INPapName, (ckt, which, foo, "nodek", parm))
	INPgetTok(&line, &steptype, 1); /* get V or I */
	ptemp.iValue = 1;
	GCA(INPapName, (ckt, which, foo, steptype, &ptemp))
	txfree(steptype);
	INPgetTok(&line, &steptype, 1); /* get POL, ZER, or PZ */
	ptemp.iValue = 1;
	GCA(INPapName, (ckt, which, foo, steptype, &ptemp))
	txfree(steptype);
    }
    else if (eq(token, ".dc")) {
	/* .dc SRC1NAME Vstart1 Vstop1 Vinc1 [SRC2NAME Vstart2 */
	/*        Vstop2 Vinc2 */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "DC")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("DC transfer curve analysis unsupported\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "DC transfer characteristic", &foo, task))
	INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	ptemp.uValue = name;
	GCA(INPapName, (ckt, which, foo, "name1", &ptemp))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vstart1 */
	GCA(INPapName, (ckt, which, foo, "start1", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vstop1 */
	GCA(INPapName, (ckt, which, foo, "stop1", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vinc1 */
	GCA(INPapName, (ckt, which, foo, "step1", parm))
	if (*line) {
	    INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	    ptemp.uValue = name;
	    GCA(INPapName, (ckt, which, foo, "name2", &ptemp))
	    parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vstart1 */
	    GCA(INPapName, (ckt, which, foo, "start2", parm))
	    parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vstop1 */
	    GCA(INPapName, (ckt, which, foo, "stop2", parm))
	    parm = INPgetValue(ckt, &line, IF_REAL, tab); /* vinc1 */
	    GCA(INPapName, (ckt, which, foo, "step2", parm))
	}
    }
    else if (eq(token, ".tf")) {
	/* .tf v( node1, node2 ) src */
	/* .tf vsrc2             src */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "TF")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Transfer Function analysis unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Transfer Function", &foo, task))
	INPgetTok(&line, &name, 0);
	/* name is now either V or I or a serious error */
	if (*name == 'v' && strlen(name) == 1) {
	    txfree(name);
	    if (*line != '(' /* match) */) {
		/* error, bad input format */
	    }
	    INPgetTok(&line, &nname1, 0); INPtermInsert(ckt, &nname1, tab, &node1);
	    ptemp.nValue = (IFnode)node1;
	    GCA(INPapName, (ckt, which, foo, "outpos", &ptemp))
	    if (*line != /* match ( */ ')') {
		INPgetTok(&line, &nname2, 1); INPtermInsert(ckt, &nname2, tab, &node2);
		ptemp.nValue = (IFnode)node2;
		GCA(INPapName, (ckt, which, foo, "outneg", &ptemp))
		ptemp.sValue = MALLOC(5 + strlen(nname1) + strlen(nname2));
		sprintf(ptemp.sValue, "V(%s,%s)", nname1, nname2);
		GCA(INPapName, (ckt, which, foo, "outname", &ptemp))
	    } else {
		ptemp.nValue = (IFnode)gnode;
		GCA(INPapName, (ckt, which, foo, "outneg", &ptemp))
		ptemp.sValue = MALLOC(4 + strlen(nname1));
		sprintf(ptemp.sValue, "V(%s)", nname1);
		GCA(INPapName, (ckt, which, foo, "outname", &ptemp))
	    }
	} else if (*name == 'i' && strlen(name) == 1) {
	    txfree(name);
	    INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	    ptemp.uValue = name;
	    GCA(INPapName, (ckt, which, foo, "outsrc", &ptemp))
	} else {
	    txfree(name);
	    LITERR("Syntax error: voltage or current expected.\n")
	    goto ret;
	}
	INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	ptemp.uValue = name;
	GCA(INPapName, (ckt, which, foo, "insrc", &ptemp))
    }
    else if (eq(token, ".tran")) {
	/* .tran Tstep Tstop <Tstart <Tmax> > <UIC> */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "TRAN")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Transient analysis unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Transient Analysis", &foo, task))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* Tstep */
	GCA(INPapName, (ckt, which, foo, "tstep", parm))
	parm = INPgetValue(ckt, &line, IF_REAL, tab); /* Tstop */
	GCA(INPapName, (ckt, which, foo, "tstop", parm))
	if (*line) {
	    dtemp = INPevaluate(&line, &error, 1);  /* tstart? */
	    if (error == 0) {
		ptemp.rValue = dtemp;
		GCA(INPapName, (ckt, which, foo, "tstart", &ptemp))
		dtemp = INPevaluate(&line, &error, 1);  /* tmax? */
		if (error == 0) {
		    ptemp.rValue = dtemp;
		    GCA(INPapName, (ckt, which, foo, "tmax", &ptemp))
		}
	    }
	}
	if (*line) {
	    INPgetTok(&line, &word, 1); /* uic? */
	    if (eq(word, "uic")) {
		ptemp.iValue = 1;
		GCA(INPapName, (ckt, which, foo, "uic", &ptemp))
	    } else {
		LITERR("Error: unknown parameter on .tran - ignored\n")
	    }
	    txfree(word);
	}
    }
    else if (eq(token, ".subckt") || eq(token, ".ends")) {
	/* not yet implemented - warn & ignore */
	LITERR("Warning: Subcircuits not yet implemented - ignored\n")
    }
    else if (eq(token, ".end")) {
	/* .end - end of input */
	/* not allowed to pay attention to additional input - return */
	txfree(token);
	return(1);
    }
    else if (eq(token, ".options") || eq(token, ".option") || eq(token, ".opt")) {
	/* .option - specify program options - rather complicated */
	/* use a subroutine to handle all of them to keep this */
	/* subroutine managable */
	INPdoOpts(ckt, ft_curckt->ci_curOpt, current, tab);
    }
    else if (eq(token, ".sens")) {
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "SENS")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Sensitivity unsupported.\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Sensitivity Analysis", &foo, task))

	/* Format is:
	 *	.sens <output>
	 *	+ [ac [dec|lin|oct] <pts> <low freq> <high freq> | dc ]
	 */

	/* Get the output voltage or current */
	INPgetTok(&line, &name, 0);
	/* name is now either V or I or a serious error */
	if (*name == 'v' && strlen(name) == 1) {
	    txfree(name);
	    if (*line != '(' /* match) */) {
		LITERR("Syntax error: '(' expected after 'v'\n");
		goto ret;
	    }
	    INPgetTok(&line, &nname1, 0); INPtermInsert(ckt, &nname1, tab, &node1);
	    ptemp.nValue = (IFnode)node1;
	    GCA(INPapName, (ckt, which, foo, "outpos", &ptemp))

	    if (*line != /* match ( */ ')') {
		INPgetTok(&line, &nname2, 1); INPtermInsert(ckt, &nname2, tab, &node2);
		ptemp.nValue = (IFnode)node2;
		GCA(INPapName, (ckt, which, foo, "outneg", &ptemp))
		ptemp.sValue = MALLOC(5 + strlen(nname1) + strlen(nname2));
		sprintf(ptemp.sValue, "V(%s,%s)", nname1, nname2);
		GCA(INPapName, (ckt, which, foo, "outname", &ptemp))
	    } else {
		ptemp.nValue = (IFnode)gnode;
		GCA(INPapName, (ckt, which, foo, "outneg", &ptemp))
		ptemp.sValue = MALLOC(4 + strlen(nname1));
		sprintf(ptemp.sValue, "V(%s)", nname1);
		GCA(INPapName, (ckt, which, foo, "outname", &ptemp))
	    }
	} else if (*name == 'i' && strlen(name) == 1) {
	    txfree(name);
	    INPgetTok(&line, &name, 1); INPinsert(&name, tab);
	    ptemp.uValue = name;
	    GCA(INPapName, (ckt, which, foo, "outsrc", &ptemp))
	} else {
	    txfree(name);
	    LITERR("Syntax error: voltage or current expected.\n")
	    goto ret;
	}

	INPgetTok(&line, &name, 1);
	if (eq(name, "pct")) {
		txfree(name);
		ptemp.iValue = 1;
		GCA(INPapName, (ckt, which, foo, "pct", &ptemp))
		INPgetTok(&line, &name, 1);
	}
	if (eq(name, "ac")) {
		INPgetTok(&line, &steptype, 1); /* get DEC, OCT, or LIN */
		ptemp.iValue = 1;
		GCA(INPapName, (ckt, which, foo, steptype, &ptemp))
		txfree(steptype);
		parm = INPgetValue(ckt, &line, IF_INTEGER, tab); /* number of points*/
		GCA(INPapName, (ckt, which, foo, "numsteps", parm))
		parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstart */
		GCA(INPapName, (ckt, which, foo, "start", parm))
		parm = INPgetValue(ckt, &line, IF_REAL, tab); /* fstop */
		GCA(INPapName, (ckt, which, foo, "stop", parm))
	} else if (*name && !eq(name, "dc")) {
		/* Bad flag */
		LITERR("Syntax error: 'ac' or 'dc' expected.\n")
	}
	txfree(name);
    }
#ifdef HAS_SENSE2
    else if (eq(token, ".sens2")) {
	/* .sens {AC} {DC} {TRAN} [dev=nnn parm=nnn]* */
	which = -1;
	for (i = 0; i < ft_sim->numAnalyses; i++) {
	    if (eq(ft_sim->analyses[i]->name, "SENS2")) { which = i; break; }
	}
	if (which == -1) {
	    LITERR("Sensitivity-2 analysis unsupported\n")
	    goto ret;
	}
	IFC(newAnalysis, (ckt, which, "Sensitivity-2 Analysis", &foo, task))
	while (*line) { /* read the entire line */
	    INPgetTok(&line, &word, 1);
	    for (i = 0; i < ft_sim->analyses[which]->numParms; i++) {
		/* find the parameter */
		if (eq(word, ft_sim->analyses[which]->analysisParms[i].keyword)) {
		    /* found it, analysis which, parameter i */
		    if (ft_sim->analyses[which]->analysisParms[i].dataType & IF_FLAG) {
			/* one of the keywords! */
			ptemp.iValue = 1;
			error = (*(ft_sim->setAnalysisParm))(ckt,
				foo, ft_sim->analyses[which]->analysisParms[i].id, &ptemp, (IFvalue*)NULL);
			if (error) current->error = INPerrCat(current->error, INPerror(error));
		    } else {
			parm = INPgetValue(ckt, &line, ft_sim->analyses[which]->analysisParms[i].dataType, tab);
			error = (*(ft_sim->setAnalysisParm))(ckt,
				foo, ft_sim->analyses[which]->analysisParms[i].id, parm, (IFvalue*)NULL);
			if (error) current->error = INPerrCat(current->error, INPerror(error));
		    }
		    break;
		}
	    }
	    if (i >= ft_sim->analyses[which]->numParms) {
		/* didn't find it! */
		LITERR("Error: unknown parameter on .sens-ignored\n")
	    }
	    txfree(word);
	}
    }
#endif
    else if (eq(token, ".probe")) {
	/* Maybe generate a "probe" format file in the future. */
    }
    else {
	LITERR("unimplemented control card - error\n")
    }
ret:
    txfree(token);
    return(0);
}
