/*
 * ISC License
 *
 * Copyright (C) 1990-2018 by
 *	Patrick Groeneveld
 *	Paul Stravers
 *	Simon de Graaf
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

#include "src/ocean/libseadif/sealib.h"
#include <string.h>
#include <ctype.h> /* declares tolower() */
#include "src/ocean/nelsea/prototypes.h"

/* The names in these arrays define substrings that indicate a power or ground
 * net if they appear at the start of the net name. Net names are down-cased
 * before they are compared to the names in these arrays.
 */
static char *groundnames[] = { "vss", "gnd", NULL };
static char *powernames[]  = { "vdd", NULL };

static void   fixPowerNet (NETPTR *netlist, char *powername, char *namearray[]);
static NETPTR collectPowerNets (NETPTR *netlist, char *namearray[]);
static int    lookUpInNameArray (char *name, char *namearray[]);
static NETPTR reduceNets (NETPTR netlist, char *powername);

/* The powerFix() routine scans the netlist of CIRCUIT. It then guesses from the
 * names of the nets what nets are power and ground nets. Then it collects all
 * the suspected power nets into a single net called "vdd" and all the suspected
 * ground nets into a single net called "vss"...
 */
void powerFix (CIRCUITPTR circuit)
{
    fixPowerNet (&circuit->netlist, "vss", groundnames);
    fixPowerNet (&circuit->netlist, "vdd", powernames);
}

static void fixPowerNet (NETPTR *netlist, char *powername, char *namearray[])
{
NETPTR powerlist = NULL;

if (!*netlist) return; /* just testin' ... Je weet het niet! */
/* move suspected power nets from netlist to powerlist... */
powerlist = collectPowerNets (netlist, namearray);
if (powerlist)
   {
   /* reduce the list of (suspected) power nets to a single powernet... */
   NETPTR powernet = reduceNets (powerlist, powername);
   /* and link this new powernet in front of the netlist ... */
   powernet->next = *netlist;
   *netlist = powernet;
   }
}

/* Remove the suspected power nets from NETLIST and return them as a new list. */
static NETPTR collectPowerNets (NETPTR *netlist, char *namearray[])
{
NETPTR powerlist = NULL, prevnet = NULL, nextnet = NULL, net = NULL;

for (net = *netlist; net; net = nextnet)
   {
   nextnet = net->next;	/* have to save cause maybe we move net to powerlist */
   if (lookUpInNameArray (net->name, namearray))
      {
      /* unlink net from the netlist... */
      if (!prevnet)
	 *netlist = nextnet;
      else
	 prevnet->next = nextnet;
      /* ...and link net in front of the powerlist... */
      net->next = powerlist;
      powerlist = net;
      }
   else
      prevnet = net;
   }
return powerlist;
}

#define MAXSTRING 255

/* Return TRUE if NAME starts with any of the substrings in NAMEARRAY,
 * 0 otherwise.  Ignore case.
 */
static int lookUpInNameArray (char *name, char *namearray[])
{
char lowerCaseName[1+MAXSTRING];
int j, c;

for (j = 0; (c = *name) && j < MAXSTRING; ++name)
   lowerCaseName[j++] = tolower (c);
lowerCaseName[j] = '\0';

for (j = 0; namearray[j]; j += 1)
   if (strstr (lowerCaseName, namearray[j]) == lowerCaseName)
      /* name starts with the string in namearray: this is a powernet! */
      return TRUE;
return 0;
}

/* reduce a list of nets to a single net (short-circuit them) ... */
static NETPTR reduceNets (NETPTR netlist, char *powername)
{
NETPTR net, nextnet, newnet;

if (!netlist) return NULL;	  /* the trivial case! */
/* add all but the first net in the list to the first net ... */
newnet = netlist;
for (net = netlist->next; net; net = nextnet)
   {
   CIRPORTREFPTR cpr;
   for (cpr = net->terminals; cpr; cpr = cpr->next)
      {
      cpr->net = newnet;	  /* set backward reference to new net */
      if (!cpr->next) break;
      }
   /* At this point cpr is NULL if there are no terminals, otherwise cpr is set
    * to the last terminal in net. Move the terminals to newnet ...
    */
   if (cpr)
      {
      cpr->next = newnet->terminals;
      newnet->terminals = net->terminals;
      net->terminals = NULL;
      newnet->num_term += net->num_term;
      }
   /* get rid of this net ... */
   nextnet = net->next;
   net->next = NULL;
   FreeNet (net);
   }
/* rename newnet to powername ... */
if (newnet->name) fs (newnet->name);
newnet->name = cs (powername);
/* ...and return the reduced net... */
return newnet;
}

/* Next two function are public (used by ghotiDelete for instance) */
int isPowerNet (NETPTR net)
{
    if (!net) return 0;
    return lookUpInNameArray (net->name, powernames);
}

int isGroundNet (NETPTR net)
{
    if (!net) return 0;
    return lookUpInNameArray (net->name, groundnames);
}
