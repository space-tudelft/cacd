/*
 * ISC License
 *
 * Copyright (C) 1993-2018 by
 *	Paul Stravers
 *	Patrick Groeneveld
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

#include <stdio.h>		  /* required by <dmincl.h> */
#ifdef __cplusplus
#ifndef __STDC__
#define __STDC__  1		  /* sometimes not defined */
#endif
extern "C" {
#endif
#include "src/libddm/dmincl.h"
#ifdef __cplusplus
}
#endif
#undef LAYOUT			  /* this clashes with <sealib.h> */
#undef CIRCUIT			  /* this clashes with <sealib.h> */
#include "src/ocean/esea/cedif.h"
#include <stdlib.h>		  /* exit() */
#include <string.h>
#include <malloc.h>

#define MAXPATHLENGTH 250	  /* max length of a unix file path name */

/* STATIC VARIABLES
 */
static DM_PROJECT *localProjectkey = NULL;
static char *circuit_str = "circuit";

/* STATIC PROTOTYPES
 */
static void findCirports (CIRCUITPTR circuit, DM_CELL *cellkey);
static void writeCirinsts (DM_STREAM *stream, CIRCUITPTR circuit);
static void writeCirports (DM_STREAM *stream, CIRCUITPTR circuit);
static void writeNetlist  (DM_STREAM *stream, CIRCUITPTR circuit);
static void buildNelsisNet (NETPTR net);
static char *baseName (char *);

void openNelsis ()
{
   dmInit ("cedif");
   localProjectkey = dmOpenProject (DEFAULT_PROJECT, DEFAULT_MODE);
   if (!localProjectkey) report (eFatal, "cannot open local project");
}

void closeNelsis ()
{
   dmCloseProject (localProjectkey, COMPLETE);
   dmQuit ();
}

void exitNelsis (int exitValue)
{
   exit (exitValue);
}

/* initialize the array projlist[] with the names of all imported libraries: */
static STRING *getProjectlist ()
{
   static int projlist_read = 0;
   static STRING projlist[MAXEXTERNLIBS+1];
   char scanline[MAXPATHLENGTH+1];
   char projpath[MAXPATHLENGTH+1];
   FILE *fp;
   int i = 0;

   if (projlist_read == TRUE) return projlist;

   /* open projlist to have a look at it... */
   if (!(fp = fopen ("projlist", "r")))
	report (eFatal, "cannot open projlist\n");
   else {
       while (fgets (scanline, 100, fp)) {
	  if (i >= MAXEXTERNLIBS) break;
	  if (scanline[0] == '#') continue; /* skip comments */
	  if (sscanf (scanline, "%s", projpath) != 1) continue;
	  projlist[i++] = cs (projpath);
       }
       fclose (fp);
   }

   if (i >= MAXEXTERNLIBS) {
      report (eWarning, "too many projects imported (> %d)", MAXEXTERNLIBS);
      i = MAXEXTERNLIBS;
   }
   projlist[i] = NULL;
   projlist_read = TRUE;
   return projlist;
}

LIBRARYPTR findNelsisLibrary (STRING libname, SEADIFPTR seadif_tree)
{
   LIBRARYPTR lib;

   for (lib = seadif_tree->library; lib; lib = lib->next)
      if (lib->name == libname) break;
   if (!lib)
   {  /* we did not find the lib, check that it really exists */
      STRING *liblist = getProjectlist();
      int i = 0;
      for (; liblist[i]; ++i)
	 if (strcmp (libname, baseName (liblist[i])) == 0) break;
      if (!liblist[i])
	 report (eFatal, "you'll have to import library \"%s\" first...", libname);
      /* it's OK, create a seadif struct that represents the lib: */
      NewLibrary (lib);
      lib->name = cs (libname);
      lib->next = seadif_tree->library;
      seadif_tree->library = lib;
   }
   return lib;
}

CIRCUITPTR findNelsisCircuit (STRING cirname, LIBRARYPTR lib)
{
   DM_CELL     *cellkey;
   DM_PROJECT  *projectkey;
   char        *remoteCellName;
   FUNCTIONPTR function;
   CIRCUITPTR  circuit;

   /* find the remote project key and the remote cell name: */
   projectkey = dmFindProjKey (IMPORTED, cirname, localProjectkey, &remoteCellName, circuit_str);
   if (!projectkey)
      report (eFatal, "cannot open cell %s in project %s (maybe not imported?)", cirname, lib->name);

   cellkey = dmCheckOut (projectkey, remoteCellName, ACTUAL, DONTCARE, circuit_str, READONLY);
   if (!cellkey)
      report (eFatal, "cannot check out cell %s in project %s", cirname, lib->name);

   /* now that we know the circuit exists, create a seadif struct for it: */
   NewFunction (function);	 /* creat a function and link it to LIB */
   function->name = cs (cirname);
   function->library = lib;
   function->next = lib->function;
   lib->function = function;

   NewCircuit (circuit);	 /* create a circuit and link it to FUNCTION */
   circuit->name = cs (cirname);
   circuit->function = function;
   function->circuit = circuit;

   /* we are interested in the interface of the circuit, that is cirports: */
   findCirports (circuit, cellkey);

   dmCheckIn (cellkey, COMPLETE);
   return circuit;
}

/* read the terminals of a cell and add them to the Seadif CIRCUIT: */
static void findCirports (CIRCUITPTR circuit, DM_CELL *cell)
{
   long lower[10], upper[10];
   char attribute_string[256];
   CIRPORTPTR cp;
   int result;
   DM_STREAM *stream = dmOpenStream (cell, "term", "r");

   if (!stream) report (eFatal, "cannot open the term stream of cell %s", circuit->name);

   dm_get_do_not_alloc = 1;
   cterm.term_attribute = attribute_string;
   cterm.term_lower = lower;
   cterm.term_upper = upper;

   while ((result = dmGetDesignData (stream, CIR_TERM)) > 0) {
      NewCirport (cp);
      cp->name = cs (cterm.term_name);
      cp->next = circuit->cirport;
#ifdef SDF_PORT_DIRECTIONS
      cp->direction = SDF_PORT_UNKNOWN;
#endif
      circuit->cirport = cp;
   }
   dm_get_do_not_alloc = 0;
   if (result) report (eFatal, "error while reading term stream of cell %s", circuit->name);
}

void writeNelsisCircuit (CIRCUITPTR circuit)
{
   DM_CELL   *cell;
   DM_STREAM *stream;
   char cellName[DM_MAXNAME+1];

   /* test: is the name too long? */
   if (strlen (circuit->name) > DM_MAXNAME) {
      report (eWarning, "cell name %s too long for nelsis, truncated", circuit->name);
      strncpy (cellName, circuit->name, DM_MAXNAME+1);
   }
   else
      strcpy (cellName, circuit->name);

   cell = dmCheckOut (localProjectkey, cellName, DERIVED, DONTCARE, circuit_str, UPDATE);
   if (!cell) report (eFatal, "cannot check out cell %s", cellName);

   /* write the model call stream */
   if (!(stream = dmOpenStream (cell, "mc", "w")))
      report (eFatal, "cannot write mc stream of cell %s", cellName);
   writeCirinsts (stream, circuit);
   dmCloseStream (stream, COMPLETE);

   /* write the cirport stream */
   if (!(stream = dmOpenStream (cell, "term", "w")))
      report (eFatal, "cannot write term stream of cell %s", cellName);
   writeCirports (stream, circuit);
   dmCloseStream (stream, COMPLETE);

   /* write the netlist */
   if (!(stream = dmOpenStream (cell, "net", "w")))
      report (eFatal, "cannot write net stream of cell %s", cellName);
   writeNetlist (stream, circuit);
   dmCloseStream (stream, COMPLETE);

   dmCheckIn (cell, COMPLETE);
}

static void writeCirinsts (DM_STREAM *stream, CIRCUITPTR circuit)
{
   CIRINSTPTR cinst = circuit->cirinst;

   for (; cinst; cinst = cinst->next)
   {
      if (strlen (cinst->name) > DM_MAXNAME) {
	 report (eWarning, "cell %s(%s): instance name %s too long, truncated",
		circuit->name, circuit->function->library->name, cinst->name);
	 strncpy (cmc.inst_name, cinst->name, DM_MAXNAME+1);
      }
      else
	 strcpy (cmc.inst_name, cinst->name);

      if (cinst->circuit->function->library->flag.l & EXTERNAL_LIBRARY)
	 cmc.imported = IMPORTED;
      else
	 cmc.imported = LOCAL;

      if (strlen (cinst->circuit->name) > DM_MAXNAME) {
	 report (eWarning, "imported cell name %s too long for nelsis, truncated",
		cinst->circuit->name);
	 strncpy (cmc.cell_name, cinst->circuit->name, DM_MAXNAME+1);
      }
      else
	 strcpy (cmc.cell_name, cinst->circuit->name);

      cmc.inst_attribute = "";
      cmc.inst_dim = 0;
      cmc.inst_lower = cmc.inst_upper = NULL;
      if (dmPutDesignData (stream, CIR_MC))
	 report (eFatal, "cannot put data on mc stream of cell %s", circuit->name);
   }
}

static void writeCirports (DM_STREAM *stream, CIRCUITPTR circuit)
{
   CIRPORTPTR cp = circuit->cirport;

   for (; cp; cp = cp->next)
   {
      if (strlen (cp->name) > DM_MAXNAME) {
	 report (eWarning, "cell %s(%s): port name %s too long, truncated",
		circuit->name, circuit->function->library->name, cp->name);
	 strncpy (cterm.term_name, cp->name, DM_MAXNAME+1);
      }
      else
	 strcpy (cterm.term_name, cp->name);

      cterm.term_attribute = NULL;
      cterm.term_dim = 0;
      cterm.term_lower = cterm.term_upper = NULL;
      if (dmPutDesignData (stream, CIR_TERM))
	 report (eFatal, "cannot put data on term stream of cell %s", circuit->name);
   }
}

static void writeNetlist (DM_STREAM *stream, CIRCUITPTR circuit)
{
   NETPTR net = circuit->netlist;

   for (; net; net = net->next)
   {
      CIRPORTREFPTR cpr;

      /* count terminals, find terminals with net name, which needs special treatment
       */
      int neqv = 0;
      for (cpr = net->terminals; cpr; cpr = cpr->next)
	 if (cpr->cirinst || cpr->cirport->name != net->name) neqv++;

      if (strlen (net->name) > DM_MAXNAME) {
	 report (eWarning, "cell %s(%s): net name %s too long, truncated",
		circuit->name, circuit->function->library->name, net->name);
	 strncpy (cnet.net_name, net->name, DM_MAXNAME+1);
      }
      else
	 strcpy (cnet.net_name, net->name);

      cnet.net_attribute = NULL;
      cnet.net_dim = 0;
      cnet.net_lower = cnet.net_upper = NULL;
      cnet.inst_name[0] = '\0';
      cnet.inst_dim = 0;
      cnet.inst_lower = cnet.inst_upper = NULL;
      cnet.ref_dim = 0;
      cnet.ref_lower = cnet.ref_upper = NULL;
      cnet.net_neqv = neqv;
      cnet.net_eqv = (struct cir_net *) calloc ((unsigned)neqv, (unsigned)sizeof(struct cir_net));
      if (!cnet.net_eqv) report (eFatal, "cannot allocate enough memory");

      buildNelsisNet (net);
      if (dmPutDesignData (stream, CIR_NET))
	 report (eFatal, "cannot put data on net stream of cell %s", circuit->name);

      free ((void *)cnet.net_eqv);
   }
}

static void buildNelsisNet (NETPTR net)
{
   int neqv = 0;
   CIRPORTREFPTR cpr;
   for (cpr = net->terminals; cpr; cpr = cpr->next)
   {
      if (!cpr->cirinst && cpr->cirport->name == net->name) continue;

      /* set terminal name */
      strncpy (cnet.net_eqv[neqv].net_name, cpr->cirport->name, DM_MAXNAME);
      if (strlen (cpr->cirport->name) > DM_MAXNAME)
	 cnet.net_eqv[neqv].net_name[DM_MAXNAME] = '\0';

      cnet.net_eqv[neqv].net_attribute = NULL;
      cnet.net_eqv[neqv].net_dim = 0;
      cnet.net_eqv[neqv].net_lower = cnet.net_eqv[neqv].net_upper = NULL;

      if (!cpr->cirinst) /* terminal on the father */
	 cnet.net_eqv[neqv].inst_name[0] = '\0';
      else
      {
	 /* terminal on a child */
	 if (strlen (cpr->cirinst->name) > DM_MAXNAME)
	    /* no warning, we've already seen this thing... */
	    strncpy (cnet.net_eqv[neqv].inst_name, cpr->cirinst->name, DM_MAXNAME+1);
	 else
	    strcpy (cnet.net_eqv[neqv].inst_name, cpr->cirinst->name);
      }
      cnet.net_eqv[neqv].inst_dim = 0;
      cnet.net_eqv[neqv].inst_lower = cnet.net_eqv[neqv].inst_upper = NULL;
      cnet.net_eqv[neqv].ref_dim = 0;
      cnet.net_eqv[neqv].ref_lower = cnet.net_eqv[neqv].ref_upper = NULL;
      cnet.net_eqv[neqv].net_neqv = 0;
      cnet.net_eqv[neqv].net_eqv = NULL;
      neqv++;
   }
}

/* return the filename part of a unix path */
char *baseName (char *s)
{
   char *p = strrchr (s, '/');
   if (p) return p+1;
   return s;
}
