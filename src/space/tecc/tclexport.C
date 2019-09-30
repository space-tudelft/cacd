/*
 * ISC License
 *
 * Copyright (C) 1988-2018 by
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

#include <iostream>
#include <sstream>

#include "src/space/tecc/define.h"
#include "src/space/tecc/extern.h"

//==============================================================================
//
//  Prototypes.
//
//==============================================================================

static void exportToTclStr (std::ostream& ostr);
static void outputLayerCondition (struct layCondRef* layer_cond_ref, std::ostream& ostr);

//==============================================================================
//
//  Convert mask number to mask name.
//
//==============================================================================

char * maskname (int i)
{
    if (i < 0) return ((char*)"@gnd"); /* return always @gnd (not @sub) */
    else if (i < procdata -> nomasks)
        return (procdata -> mask_name[i]);
    else if (i < procdata -> nomasks + subdata -> nomasks)
        return (subdata -> mask_name[i - procdata -> nomasks]);
    return (NULL);
}

//==============================================================================
//
//  Export to Tcl script file.
//
//==============================================================================

void exportToTcl (FILE *fp)
{
    std::ostringstream sstr;
    exportToTclStr (sstr);
    fwrite ((void*) sstr.str().c_str(), sstr.str().size(), 1, fp);
}

static void exportToTclStr (std::ostream& ostr)
{
    int index, i, j;

    // Output the masks.
    {
        index = 0;
        for(i = 0; i < procdata->nomasks; ++i)
        {
            ostr << "set technology_table(masks:" << index++ << ":name) " << procdata->mask_name[i] << std::endl;
        }

        for(i = 0; i < subdata->nomasks; ++i)
        {
            ostr << "set technology_table(masks:" << index++ << ":name) " << subdata->mask_name[i] << std::endl;
        }

        ostr << "set technology_table(masks:size) " << index << std::endl;

        ostr << "" << std::endl;
    }

    // Output the conductors.
    {
        index = 0;
        for(i = 0; i < res_cnt;)
        {
            ostr << "set technology_table(conductors:" << index << ":name) " << ress[i].name << std::endl;
            ostr << "set technology_table(conductors:" << index << ":mask) " << maskname(ress[i].mask) << std::endl;
            ostr << "set technology_table(conductors:" << index << ":sort) \"" << ress[i].sort << "\"" << std::endl;
            ostr << "set technology_table(conductors:" << index << ":value) " << ress[i].val << std::endl;
            ostr << "set technology_table(conductors:" << index << ":condition) \"";

            for(j = i; j < res_cnt && ress[j].id == ress[i].id; ++j)
            {
                if(j != i) ostr << " | ";
                ASSERT(ress[j].cond != 0);
                outputLayerCondition(ress[j].cond, ostr);
            }
            ostr << "\"" << std::endl;

            ostr << std::endl;

            index++;
            i = j;
        }

        ostr << "set technology_table(conductors:size) " << index << std::endl;
        ostr << std::endl;
    }

    // Output the contacts.
    {
        index = 0;
        for(i = 0; i < con_cnt;)
        {
            ostr << "set technology_table(contacts:" << index << ":name) " << cons[i].name << std::endl;
            ostr << "set technology_table(contacts:" << index << ":mask1) " << maskname(cons[i].mask1) << std::endl;
            ostr << "set technology_table(contacts:" << index << ":mask2) " << maskname(cons[i].mask2) << std::endl;
            ostr << "set technology_table(contacts:" << index << ":value) " << cons[i].val << std::endl;
            ostr << "set technology_table(contacts:" << index << ":condition) \"";

            for(j = i; j < con_cnt && cons[j].id == cons[i].id; ++j)
            {
                if(j != i) ostr << " | ";
                ASSERT(cons[j].cond != 0);
                outputLayerCondition(cons[j].cond, ostr);
            }
            ostr << "\"" << std::endl;

            ostr << std::endl;

            index++;
            i = j;
        }

        ostr << "set technology_table(contacts:size) " << index << std::endl;
        ostr << std::endl;
    }

    // Output the vdimensions.
    {
        index = 0;
        for(i = 0; i < vdm_cnt;)
        {
            ostr << "set technology_table(vdimensions:" << index << ":name) " << vdms[i].name << std::endl;
            ostr << "set technology_table(vdimensions:" << index << ":mask) " << maskname(vdms[i].mask) << std::endl;
            ostr << "set technology_table(vdimensions:" << index << ":condition) \"";

            for(j = i; j < vdm_cnt && vdms[j].id == vdms[i].id; ++j)
            {
                if(j != i) ostr << " | ";
                ASSERT(vdms[j].cond != 0);
                outputLayerCondition(vdms[j].cond, ostr);
            }
            ostr << "\"" << std::endl;

            ostr << "set technology_table(vdimensions:" << index << ":z) " << vdms[i].height << std::endl;
            ostr << "set technology_table(vdimensions:" << index << ":thickness) " << vdms[i].thickness << std::endl;
            ostr << "set technology_table(vdimensions:" << index << ":spacing) " << vdms[i].spacing << std::endl;

            ostr << std::endl;

            index++;
            i = j;
        }
        ostr << "set technology_table(vdimensions:size) " << index << std::endl;
        ostr << std::endl;
    }

    // Output the dielectrics.
    {
        for(i = 0; i < diel_cnt; ++i)
        {
            ostr << "set technology_table(dielectrics:" << i << ":name) " << diels[i].name << std::endl;
            ostr << "set technology_table(dielectrics:" << i << ":z) " << diels[i].bottom*1e-6 << std::endl;
            ostr << "set technology_table(dielectrics:" << i << ":permittivity) " << diels[i].permit << std::endl;
            ostr << std::endl;
        }

        ostr << "set technology_table(dielectrics:size) " << diel_cnt << std::endl;
    }
}

//==============================================================================
//
//  Output layer condition.
//
//==============================================================================

static void
outputLayerCondition (struct layCondRef* layer_cond_ref, std::ostream& ostr)
{
    bool printed = false;
    for(; layer_cond_ref != 0; layer_cond_ref = layer_cond_ref->next)
    {
        if(printed)
            ostr << " ";
        else
            printed = true;

        if(!layer_cond_ref->layC->present)
            ostr << "!";

        if(layer_cond_ref->layC->lay->occurrence == EDGE)
            ostr << "-";
        else if(layer_cond_ref->layC->lay->occurrence == OTHEREDGE)
            ostr << "=";

        const char* name = maskname(layer_cond_ref->layC->lay->mask);
        ASSERT(name != 0);

        ostr << name;
    }

    if (!printed) ostr << "1";
}
