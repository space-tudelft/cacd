/**********
Copyright 1990 Regents of the University of California.  All rights reserved.
Author: 1985 Thomas L. Quarles
**********/

#include "spice.h"
#include <stdio.h>
#include "misc.h"
#include "iferrmsg.h"
#include "util.h"
#include "inpdefs.h"

char * INPdomodel(GENERIC *ckt, card *image, INPtables *tab)
{
    char *modname;
    int type;
    int lev;
    char *typename;
    char *err = NULL;
    char *line;

    line = image->line;
    INPgetTok(&line, &modname, 1); FREE(modname); /* throw away '.model' */
    INPgetTok(&line, &modname, 1); INPinsert(&modname, tab);
    INPgetTok(&line, &typename, 1);

    if (eq(typename, "npn") || eq(typename, "pnp")) {
        type = INPtypelook("BJT");
        if (type < 0) err = INPmkTemp("Device type BJT not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "d")) {
        type = INPtypelook("Diode");
        if (type < 0) err = INPmkTemp("Device type Diode not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "njf") || eq(typename, "pjf")){
        type = INPtypelook("JFET");
        if (type < 0) err = INPmkTemp("Device type JFET not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "nmf") || eq(typename, "pmf")) {
        type = INPtypelook("MES");
        if (type < 0) err = INPmkTemp("Device type MES not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "urc")) {
        type = INPtypelook("URC");
        if (type < 0) err = INPmkTemp("Device type URC not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "nmos") || eq(typename, "pmos")) {
        err = INPfindLev(line, &lev);
        switch(lev) {
            case 0:
            case 1:
                type = INPtypelook("Mos1");
                if (type < 0) err = INPmkTemp("Device type MOS1 not available in this binary\n");
                break;
            case 2:
                type = INPtypelook("Mos2");
                if (type < 0) err = INPmkTemp("Device type MOS2 not available in this binary\n");
                break;
            case 3:
                type = INPtypelook("Mos3");
                if (type < 0) err = INPmkTemp("Device type MOS3 not available in this binary\n");
                break;
            case 4:
                type = INPtypelook("BSIM1");
                if (type < 0) err = INPmkTemp("Device type BSIM1 not available in this binary\n");
                break;
            case 5:
                type = INPtypelook("BSIM2");
                if (type < 0) err = INPmkTemp("Device type BSIM2 not available in this binary\n");
                break;
            case 6:
                type = INPtypelook("Mos6");
                if (type < 0) err = INPmkTemp("Device type MOS6 not available in this binary\n");
                break;
            case 7:
            case 8:
                type = INPtypelook("BSIM3");
                if (type < 0) err = INPmkTemp("Device type BSIM3 not available in this binary\n");
                break;
            default: /* placeholder; use level 9 for the next model */
		type = -1; err = INPmkTemp("Only MOS device levels 1-8 are supported in this binary\n");
        }
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "r")) {
        type = INPtypelook("Resistor");
        if (type < 0) err = INPmkTemp("Device type Resistor not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "c")) {
        type = INPtypelook("Capacitor");
        if (type < 0) err = INPmkTemp("Device type Capacitor not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "sw")) {
        type = INPtypelook("Switch");
        if (type < 0) err = INPmkTemp("Device type Switch not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "csw")) {
        type = INPtypelook("CSwitch");
        if (type < 0) err = INPmkTemp("Device type CSwitch not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else if (eq(typename, "ltra")) {
        type = INPtypelook("LTRA");
        if (type < 0) err = INPmkTemp("Device type LTRA not available in this binary\n");
        INPmakeMod(modname, type, image);
    } else {
        type = -1;
        err = MALLOC(35 + strlen(typename));
        sprintf(err, "unknown model type %s - ignored\n", typename);
    }
    FREE(typename);
    return(err);
}
